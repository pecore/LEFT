/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
    Christopher M. Kohlhoff (chris at kohlhoff dot com)
*/

#ifndef _LEFTNET_H_
#define _LEFTNET_H_

#ifndef _LEFT_NET_TYPES
#include <ctime>
#include <iostream>
#include <queue>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;
#endif
#define FOURCC(a,b,c,d) ((unsigned int)(a & 0xFF) << 24 | (b & 0xFF) << 16 | (c & 0xFF) << 8 | (d & 0xFF))

#define LEFT_NET_MSG_WUI                    FOURCC('L', 'E', 'F', 'T')
#define LEFT_NET_MSG_BYE                    FOURCC('B', 'Y', 'E', '!')
#define LEFT_NET_MSG_CHAT                   FOURCC('C', 'H', 'A', 'T')
  #define LEFT_NET_MSG_CHAT_MSGLENGTH       140
#define LEFT_NET_MSG_UPDATE_POS             FOURCC('U', 'P', 'O', 'S')
  static const unsigned int left_net_modelcount = 2;
  static const char * left_net_models[left_net_modelcount] = {
    "data\\robotv4.png",
    "data\\robotv5.png",
  };
#define LEFT_NET_MSG_UPDATE_MAP             FOURCC('U', 'M', 'A', 'P')  
#define LEFT_NET_MSG_DESTROY_MAP            FOURCC('B', 'O', 'O', 'M')
#define LEFT_NET_MSG_PROJECTILE             FOURCC('P', 'R', 'O', 'J')
#define LEFT_NET_MSG_UPDATE_STATS           FOURCC('S', 'T', 'A', 'T')

#define uintptr unsigned long
struct left_message {
  struct {
    unsigned int msg;
    unsigned int size;
    unsigned int sender;
  } header;
  
  union {
    // LEFT_NET_MSG_WUI
    struct {
      char name[32];
    } wui;
    
    // LEFT_NET_MSG_CHAT
    struct {
      char name[32];
      char msg[LEFT_NET_MSG_CHAT_MSGLENGTH];
    } chat;

    // LEFT_NET_MSG_UPDATE_POS
    struct {
      float xpos;
      float ypos;
      float angle;
      float weaponangle;
      float robotangle;
    } position;

    // LEFT_NET_MSG_PROJECTILE
    struct {
      unsigned int type;
      float dirx;
      float diry;
    } projectile;

    // LEFT_NET_MSG_UPDATE_STATS
    struct {
      float health;
    } stats;

    char buffer[0xFFFF];
  } msg;
};

inline unsigned int sizeof_message(unsigned int msg)
{
  left_message d;
  switch(msg) {
  default:
  case LEFT_NET_MSG_BYE:
    return 0;
  case LEFT_NET_MSG_WUI:
    return sizeof(d.msg.wui);
  case LEFT_NET_MSG_CHAT:
    return sizeof(d.msg.chat);
  case LEFT_NET_MSG_UPDATE_POS:
    return sizeof(d.msg.position);
  case LEFT_NET_MSG_PROJECTILE:
    return sizeof(d.msg.projectile);
  case LEFT_NET_MSG_UPDATE_STATS:
    return sizeof(d.msg.stats);
  }
}

#define LEFT_NET_POOL_SIZE 2048
class message_pool {
public:
  message_pool() {
    memset(inuse, 0, LEFT_NET_POOL_SIZE * sizeof(bool));
  }

  left_message * get() {
    for(int i = 0; i < LEFT_NET_POOL_SIZE; i++) {
      if(!inuse[i]) {
        inuse[i] = true;
        return &pool[i];
      }
    }
    return 0;
  }

  void del(left_message * m) {
    if(!m) {
      return;
    }
    unsigned int index = ((uintptr) m - (uintptr) pool) / sizeof(left_message);
    inuse[index] = false;
  }

private:
  left_message pool[LEFT_NET_POOL_SIZE];
  bool inuse[LEFT_NET_POOL_SIZE];
};

extern message_pool * gMessagePool;

inline left_message * new_message(unsigned int msg) 
{ 
  left_message * ret = gMessagePool->get();
  memset(ret, 0, sizeof(left_message));
  ret->header.msg = msg;
  ret->header.size = sizeof_message(msg);
  return ret;
}

inline left_message * new_message(left_message * value) 
{ 
  left_message * ret = gMessagePool->get();
  memcpy(ret, value, sizeof(value->header) + value->header.size);
  return ret;
}

#ifndef _LEFT_NET_TYPES
class distributor {
public:
  virtual ~distributor() {}
  virtual void distribute(left_message * m) = 0;
  virtual void push(left_message * m) = 0;
  virtual void remove(unsigned int id) = 0;
};

class tcp_connection : public boost::enable_shared_from_this<tcp_connection> {
public:
  typedef boost::shared_ptr<tcp_connection> pointer;
  tcp::socket& socket() { return socket_; }

  static pointer create(boost::asio::io_service& _io_service, distributor * _dist, unsigned int _id, std::string _name) 
  {
    return pointer(new tcp_connection(_io_service, _dist, _id, _name));
  }

  inline void next_header() {
    boost::asio::async_read(socket_, boost::asio::buffer(&recv.header, sizeof(recv.header)), 
      boost::bind(&tcp_connection::handle_read_header, shared_from_this(), 
      boost::asio::placeholders::error));
  }
  inline void next_msg(unsigned int size) {
    boost::asio::async_read(socket_, boost::asio::buffer(&recv.msg, size), 
      boost::bind(&tcp_connection::handle_read_msg, shared_from_this(), 
      boost::asio::placeholders::error));
  }

  void start()
  {
    left_message * wui = new_message(LEFT_NET_MSG_WUI);
    wui->header.sender = id;
    strcpy(wui->msg.wui.name, name.c_str());
    send_message(wui);
    next_header();
  }

  void send_message(left_message * m) 
  {
    memcpy(&send, m, sizeof(m->header) + m->header.size);
    boost::asio::async_write(socket_, boost::asio::buffer(&send, sizeof(send.header) + send.header.size),
          boost::bind(&tcp_connection::handle_write, shared_from_this(),
          boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
  }

  void handle_read_header(const boost::system::error_code& error)
  {
    if(error) {
      dist->remove(id);
      return;
    }
    unsigned int size = recv.header.size;
    recv.header.sender = id;
    if(size > 0) {
      next_msg(size);  
    } else {
      dist->distribute(&recv);
      dist->push(&recv);
      memset(&recv, 0, sizeof(left_message));
      next_header();
    }
  }

  void handle_read_msg(const boost::system::error_code& error)
  {
    if(error) {
      dist->remove(id);
      return;
    }
    dist->distribute(&recv);
    dist->push(&recv);
    memset(&recv, 0, sizeof(left_message));
    next_header();
  }

  unsigned int getid() { return id; }

private:
  tcp_connection(boost::asio::io_service& _io_service, distributor * _dist, unsigned int _id, std::string _name)
    : socket_(_io_service), dist(_dist), id(_id), name(_name)
  {
  }

  void handle_write(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if(error) {
      dist->remove(id);
      return;
    }
  }

  std::string name;
  left_message recv;
  left_message send;
  distributor * dist;
  tcp::socket socket_;
  unsigned int id;
};

class tcp_server : public distributor {
public:
  typedef std::queue<left_message *> message_fifo;

  tcp_server(boost::asio::io_service& io_service, std::string _name) : acceptor_(io_service, tcp::endpoint(tcp::v4(), 40155)), next_id(1), name(_name)
  {
    next_id = 1;
    start_accept();
  }

  unsigned int clientcount() { return connections.size(); }

  left_message * get_message()
  {
    if(messages.size() == 0) return 0;
    left_message * m = messages.front();
    messages.pop();
    return m;
  }

  void remove(unsigned int id)
  {
    left_message * bye = 0;
    std::list<tcp_connection::pointer>::iterator iter = connections.begin();
    for(; iter != connections.end(); iter++) {
      tcp_connection::pointer client_connection = *iter;
      if(client_connection->getid() == id) {
        iter = connections.erase(iter);

        bye = new_message(LEFT_NET_MSG_BYE);
        bye->header.sender = id;
        break;
      }
    }
    if(bye) {
      distribute(bye);
      push(bye);
    }
  }

  void distribute(left_message * m)
  {
    std::list<tcp_connection::pointer>::iterator iter = connections.begin();
    for(; iter != connections.end(); iter++) {
      tcp_connection::pointer client_connection = *iter;
      if(client_connection->getid() != m->header.sender) {
        client_connection->send_message(m);
      }
    }
  }

  void send_message(left_message * m, unsigned int sender)
  {
    m->header.sender = 0;
    std::list<tcp_connection::pointer>::iterator iter = connections.begin();
    for(; iter != connections.end(); iter++) {
      tcp_connection::pointer client_connection = *iter;
      if(client_connection->getid() == sender) {
        client_connection->send_message(m);
      }
    }
  }

  void push(left_message * m)
  {
    messages.push(new_message(m));
  }

private:
  void start_accept() 
  {
    tcp_connection::pointer new_connection = tcp_connection::create(acceptor_.get_io_service(), this, next_id++, name);
    acceptor_.async_accept(new_connection->socket(), boost::bind(&tcp_server::handle_accept, this, new_connection, boost::asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error) 
  {
    if (!error) {
      new_connection->start();
      connections.push_back(new_connection);
    }
    start_accept();
  }

  std::string name;
  unsigned int next_id;
  tcp::acceptor acceptor_;
  std::list<tcp_connection::pointer> connections;
  message_fifo messages;
};













class tcp_client {
public:
  typedef std::queue<left_message *> message_fifo;

  tcp_client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator) : io_service_(io_service), socket_(io_service)
  {
    connected = false;
    boost::asio::async_connect(socket_, endpoint_iterator,
          boost::bind(&tcp_client::handle_connect, this,
          boost::asio::placeholders::error));
  }

  bool isconnected() { return connected; }

  left_message * get_message()
  {
    if(messages.size() == 0) return 0;
    left_message * m = messages.front();
    messages.pop();
    return m;
  }

  inline void next_header() {
    boost::asio::async_read(socket_, boost::asio::buffer(&recv.header, sizeof(recv.header)), 
      boost::bind(&tcp_client::handle_read_header, this, 
      boost::asio::placeholders::error));
  }
  inline void next_msg(unsigned int size) {
    boost::asio::async_read(socket_, boost::asio::buffer(&recv.msg, size), 
      boost::bind(&tcp_client::handle_read_msg, this, 
      boost::asio::placeholders::error));
  }

  void handle_connect(const boost::system::error_code& error) 
  {
    if(!error) {
      connected = true;
      next_header();
    }
  }

  void send_message(left_message * m) 
  {
    memcpy(&send, m, sizeof(m->header) + m->header.size);
    boost::asio::async_write(socket_, boost::asio::buffer(&send, sizeof(send.header) + send.header.size), 
          boost::bind(&tcp_client::handle_write, this,
          boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
  }

private:
  void handle_read_header(const boost::system::error_code& error)
  {
    if(error) {
      handle_error();
      return;
    }
    unsigned int size = recv.header.size;
    if(size > 0) {
      next_msg(size);  
    } else {
      handle_read_msg(boost::system::error_code());
    }
  }

  void handle_read_msg(const boost::system::error_code& error)
  {
    if(error) {
      handle_error();
      return;
    }
    messages.push(new_message(&recv));
    memset(&recv, 0, sizeof(left_message));
    next_header();
  }

  void handle_write(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if(error) {
      handle_error();
      return;
    }
  }

  void handle_error()
  {
    connected = false;
  }

  bool connected;
  left_message recv;
  left_message send;
  message_fifo messages;
  boost::asio::io_service& io_service_;
  tcp::socket socket_;
};
#endif

#endif