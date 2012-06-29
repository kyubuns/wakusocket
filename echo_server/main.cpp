#include <iostream>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include "../wakusocket/server.hpp"

using boost::asio::ip::tcp;

class echo_client : public wakusocket::client {
public:
  echo_client(wakusocket::session &session_) : wakusocket::client(session_) {}
  void on_message(const std::string &message) {
    std::cout << "RECEIVE: " << message << std::endl;
    send(message);
  }
  void on_open() {
    std::cout << "on_open" << std::endl;
  }
  void on_close() {
    std::cout << "on_close" << std::endl;
  }
};

int main() try {
  boost::asio::io_service io_service;
  wakusocket::server s(io_service,
      tcp::endpoint(tcp::v4(), 12345),
      [](wakusocket::session &a){return boost::make_shared<echo_client>(a);}
  );
  std::cout << "=====echo_server run=====" << std::endl;
  io_service.run();
  return 0;
}
catch (std::exception& e) {
  std::cerr << "Exception: " << e.what() << "\n";
}

