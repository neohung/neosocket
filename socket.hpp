#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
  #include <winsock2.h>
  #include <windows.h>
  #include <WS2tcpip.h>
#else
  #include <netinet/in.h>
  #include <netinet/tcp.h> //For TCP_NODELAY
#endif
#include <fcntl.h>
#include <cstdlib>
#include <stdio.h>

#include <unistd.h> // close()
#include <vector> 
namespace NEO
{
  class Session{
    public:
      int socketfd;
  	  int rdata_size;
  	  char* rdata;
  	  int wdata_size;
  	  char* wdata;
  	  int connected;
  	  void (*recv_func)(Session* s);
  	  void (*send_func)(Session* s);
  	  void (*parse_func)(Session* s);
  	  Session(){recv_func = NULL;rdata_size=0;};
  	  int GetFD(){return socketfd;};
  	  //Singleton
  	  //Session& GetInstance(){ static Session instance;return instance;};
  	  void SetRecvFunc(void (*recv_callback)(Session* s)){
  	  	recv_func = recv_callback;
  	  }
  };
  class Socket{
    public:
      //Session sessions[1024];
      std::vector<Session*> sessions;     
      fd_set fds;
      int listen_fd;
      int MAX_SOCKET_FD;
      void (*connection_func)(Session* s);
	  Socket(){
	  	listen_fd = -1;
        MAX_SOCKET_FD = 0;
        connection_func = NULL;
	  };
      void abc(void);
      void makelisten(int listen_port, void (*new_connection_func)(Session* s));
      void do_recv_send(int timeout);
  };
}

