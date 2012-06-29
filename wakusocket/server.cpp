#include "server.hpp"

namespace wakusocket {

session::session(asio::io_service& io_service, client_create_func &factory)
  : socket(io_service), my_client(factory(*this)) {
}

void session::send(const std::string &message) {
  asio::async_write(socket,
      asio::buffer(message.c_str(), message.size()),
      boost::bind(&session::handle_write, shared_from_this(),
        asio::placeholders::error));
}

void session::close() {
  socket.close();
}

tcp::socket& session::get_socket() { return socket; }

void session::start() {
  asio::async_read_until(socket, receive_buffer, "\n",
      boost::bind(&session::oneline_read, shared_from_this(), asio::placeholders::error));
}

void session::catch_error(const boost::system::error_code& error) {
  my_client->on_close();
}

void session::oneline_read(const boost::system::error_code& error) {
  if (error) {
    catch_error(error);
    return;
  }

  std::string mes;
  std::istream is(&receive_buffer);
  std::getline(is, mes);
  my_client->on_message(mes);
  asio::async_read_until(socket, receive_buffer, "\n",
      boost::bind(&session::oneline_read, shared_from_this(), asio::placeholders::error));
}

void session::handle_write(const boost::system::error_code& error) {
  if (!error) return;
  catch_error(error);
}

void client::send(const std::string &message) { parent.send(message); }
void client::close() { parent.close(); }

}

