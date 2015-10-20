#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
  #include <winsock2.h>
  #include <windows.h>
  #include <WS2tcpip.h>
#else
  #include <netinet/in.h>
  #include <netinet/tcp.h> //For TCP_NODELAY
  #include <arpa/inet.h>   //For inet_addr()
#endif
#include <fcntl.h>
#include <cstdlib>
#include <stdio.h>

#include <unistd.h> // close()
#include <vector>
#include <assert.h>

#include "Packet_Fixed.hpp"

namespace NEO
{
  struct Buffer
   {
    std::vector<char> bytes;
   };

  class Session{
    public:
      int socketfd;
  	  int rdata_size;
      int max_rdata;
  	  char* rdata;
  	  int wdata_size;
      int max_wdata;
  	  char* wdata;
  	  int connected;
  	  void (*recv_func)(Session* s);
  	  void (*send_func)(Session* s);
  	  void (*parse_func)(Session* s);
  	  Session(){initSession();};
      void initSession(void){
        wdata = NULL;rdata = NULL ;recv_func = NULL;rdata_size=0;parse_func=NULL;send_func=NULL;wdata_size = 0;max_wdata=0;max_rdata = 0;
        //rdata = (char*)malloc(128);
        //rdata_size = 0;
        //max_rdata = 128;
        //memset(rdata,0,128);
      };
  	  int GetFD(){return socketfd;};
  	  //Singleton
  	  //Session& GetInstance(){ static Session instance;return instance;};
  	  void SetRecvFunc(void (*recv_callback)(Session* s)){
  	  	recv_func = recv_callback;
  	  }
      void SetSendFunc(void (*send_callback)(Session* s)){
        send_func = send_callback;
      }
      void SetParseFunc(void (*parse_callback)(Session* s)){
        parse_func = parse_callback;
      }
      unsigned short peek_package_id(void);
      bool send_wdata(const char* data, int len);
      void wdata_dump(void);
      void rdata_dump(void);
      void clean_rdata(void){rdata_size = 0;};
      void clean_wdata(void){wdata_size = 0;};
      void remove_rdata(int len);

  };
  class Socket{
    public:
      //Session sessions[1024];
      std::vector<Session*> sessions;
      fd_set fds;
      int connect_fd;
      struct sockaddr_in client_address;
      int MAX_SOCKET_FD;
      void (*connection_func)(Session* s);
	    Socket(){
	  	 initSocket();
	    };
      void initSocket(void){
        connect_fd = -1;
        MAX_SOCKET_FD = 0;
        connection_func = NULL;
      };
      bool reconnect(void);
      void abc(void);
      void makelisten(int listen_port, void (*new_connection_func)(Session* s));
      void makeclient(const char* url,int port,int timeout_sec, void (*new_connection_func)(Session* s));
      void do_recv_send(int timeout);
      void do_parse_package(void);
      void do_client_event(int timeout_sec);

  };
}

#endif
