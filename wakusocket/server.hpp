#ifndef WAKUSOCKET_SERVER
#define WAKUSOCKET_SERVER

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/uuid/sha1.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>

namespace asio = boost::asio;
using asio::ip::tcp;

namespace wakusocket {

class session;
class client {
public:
  client(session &session_) : parent(session_) {}
  virtual ~client() {}
  virtual void on_message(const std::string &message) {}
  virtual void on_open() {}
  virtual void on_close() {}
  void send(const std::string &message);
  void close();

private:
  session &parent;
};

typedef std::function<boost::shared_ptr<client>(session&)> client_create_func;
class session : public boost::enable_shared_from_this<session> {
public:
  session(asio::io_service& io_service, client_create_func &factory);
  void send(const std::string &message);
  void close();

private:
  friend class server;

  tcp::socket& get_socket();
  void catch_error(const boost::system::error_code& error);
  void start();
  void handle_write(const boost::system::error_code& error);
  void oneline_read(const boost::system::error_code& error);

private:
  tcp::socket socket;
  asio::streambuf receive_buffer;
  boost::shared_ptr<client> my_client;
};

class server {
public:
  server(asio::io_service& io_service_, const tcp::endpoint& endpoint, client_create_func factory_)
    : io_service(io_service_), acceptor(io_service_, endpoint), factory(factory_) {
    start_accept();
  }

  void start_accept() {
    boost::shared_ptr<session> new_session = boost::make_shared<session>(io_service, factory);
    acceptor.async_accept(new_session->get_socket(),
        boost::bind(&server::handle_accept, this, new_session, asio::placeholders::error));
  }

  void handle_accept(boost::shared_ptr<session> new_session, const boost::system::error_code& error) {
    if (!error) new_session->start();
    start_accept();
  }

private:
  asio::io_service& io_service;
  tcp::acceptor acceptor;
  client_create_func factory;
};

}

#endif
