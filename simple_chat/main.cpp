#include <iostream>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <set>
#include "../wakusocket/server.hpp"

using boost::asio::ip::tcp;

class chat_participant : public boost::enable_shared_from_this<chat_participant> {
public:
  virtual ~chat_participant() {}
  virtual void deliver(const std::string& msg) = 0;
};

class chat_room {
public:
  void join(boost::shared_ptr<chat_participant> participant) {
    participants.insert(participant);
  }

  void leave(boost::shared_ptr<chat_participant> participant) {
    participants.erase(participant);
  }

  void deliver(const std::string& msg) {
    std::for_each(participants.begin(), participants.end(),
        boost::bind(&chat_participant::deliver, _1, boost::ref(msg)));
  }

private:
  std::set<boost::shared_ptr<chat_participant>> participants;
};

class chat_client : public wakusocket::client, public chat_participant{
public:
  chat_client(wakusocket::session &session_, chat_room &room_) : wakusocket::client(session_), room(room_) {}
  void on_message(const std::string &message) {
    std::cout << "RECEIVE: " << message << std::endl;
    room.deliver(message);
  }
  void on_open() {
    room.join(shared_from_this());
  }
  void on_close() {
    room.leave(shared_from_this());
  }
  void deliver(const std::string& msg) {
    send(msg);
  }

private:
  chat_room &room;
};

int main() try {
  boost::asio::io_service io_service;
  chat_room room;
  wakusocket::server s(io_service,
      tcp::endpoint(tcp::v4(), 12345),
      [&room](wakusocket::session &a){return boost::make_shared<chat_client>(a, room);}
  );
  std::cout << "=====chat_server run=====" << std::endl;
  io_service.run();
  return 0;
}
catch (std::exception& e) {
  std::cerr << "Exception: " << e.what() << "\n";
}

