#ifndef LISTENER_HPP_
#define LISTENER_HPP_
#include <protobuf/message.h>
#include <protobuf/descriptor.h>
#include <protobuf/service.h>
#include "client/protobuf_client_connection.hpp"
#include "boost/function.hpp"
// A bi direction long connection channel.
class Listener : public boost::noncopyable {
 private:
    typedef boost::function1<
      void,
      const ProtobufLineFormat*> MessageReceiveHandler;
    typedef hash_map<string, MessageReceiveHandler> HandlerTable;
 public:
  Listener(IOServicePtr io_service,
           const string &server,
           const string &port)
    : server_(server), port_(port),
      client_(io_service) {
      client_.set_listener(boost::bind(
          &Listener::Receive, this,
          _1));
  }
  bool IsConnected() {
    return client_.IsConnected();
  }
  bool Connect() {
    return client_.Connect(server_, port_);
  }
  template <typename MessageType>
  void Listen(const boost::function1<void, shared_ptr<MessageType> > &callback) {
    shared_ptr<MessageType> msg(new MessageType);
    const string &msg_name = msg->GetDescriptor()->full_name();
    if (handler_table_.find(msg_name) != handler_table_.end()) {
      LOG(WARNING) << "Listen on " << msg_name << " is duplicated.";
      return;
    }
    boost::function1<void, const ProtobufLineFormat*> handler =
      boost::bind(&Listener::ReceiveMessage<MessageType>,
                  this,
                  _1,
                  msg,
                  callback);
    handler_table_.insert(make_pair(
        msg_name,
        handler));
  }
 private:
  void Receive(const ProtobufLineFormat *line_format);
  template <typename MessageType>
  void ReceiveMessage(const ProtobufLineFormat *line_format,
                      shared_ptr<MessageType> message_prototype,
                      const boost::function1<void, shared_ptr<MessageType> > &callback) {
    shared_ptr<MessageType> message(message_prototype->New());
    if (!message->ParseFromArray(line_format->body.c_str(),
                                 line_format->body.size())) {
      LOG(WARNING) << "Fail to parse message " << line_format->name;
    }
    callback(message);
  }
  string server_, port_;
  ProtobufEncoder encoder_;
  ProtobufClientConnection client_;
  HandlerTable handler_table_;
};
#endif  // LISTENER_HPP_
