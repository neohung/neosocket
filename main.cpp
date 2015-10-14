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

#include "socket.hpp"

NEO::Socket neos;

void recv_callback(NEO::Session* s)
{
  printf("recv occur, fd=%d\n",s->GetFD());
  printf("recv_size=%d\n",s->rdata_size);
  s->rdata[s->rdata_size] = '\0';
  printf("recv=[%s]\n",s->rdata);
   
}

void connection_callback(NEO::Session* s)
{
  printf("Connection occur, fd=%d\n",s->GetFD());
  s->SetRecvFunc(recv_callback);
}

int main(void)
{
  //NEO::Session a;
  //a.SetRecvFunc(recv_callback);
  //a.debugfunc();
  
  
  neos.makelisten(1234, connection_callback);
  for( ; ; ) {
    neos.do_recv_send(3);
  }

  exit(0);
}
