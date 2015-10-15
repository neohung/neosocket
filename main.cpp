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

//
#define VERSION_MAJOR 15
#define VERSION_MINOR 10
#define VERSION_PATCH 01
#define VERSION_DEVEL 0
#define VENDOR_POINT 0
NEO::Version CURRENT_LOGIN_SERVER_VERSION =
{
    VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
    VERSION_DEVEL,
    0, 0,
    VENDOR_POINT,
};
//

NEO::Socket neos;

void recv_callback(NEO::Session* s)
{
 // printf("recv occur, fd=%d\n",s->GetFD());
 // printf("recv_size=%d\n",s->rdata_size);
}

void parse_callback(NEO::Session* s)
{
  //printf("parse_callback\n");
  if (s->rdata_size >=2){
    switch(s->peek_package_id())
    {
      case 0x7530 :       // Request of the server version
        {
          printf("Request of the server version\n");
          NEO::Packet_Fixed<0x7531> fixed_7531;
          NEO::Version version = CURRENT_LOGIN_SERVER_VERSION;
          version.flags = 1;//login_conf.new_account ? 1 : 0;
          fixed_7531.version = version;
          //
          NEO::Buffer buf;
          buf.bytes.resize(sizeof(NEO::Packet_Fixed<0x7531>));
          auto& net_fixed = reinterpret_cast<NEO::Packet_Fixed<0x7531>&>(*(buf.bytes.begin() + 0));
          net_fixed = fixed_7531;
          //
          //send_fpacket<0x7531, 10>(s, fixed_31);    
        }
        break;
    }
  }
  else{
    printf("----------------------------\n");
    printf("rdata_size=%d\n",s->rdata_size);
    for (int i=0;i<s->rdata_size;i++)
    {
      printf("0x%x, ",s->rdata[i]);
    }
    printf("\n");
  }
  //s->rdata[s->rdata_size] = '\0';
  //printf("ASCII=[%s]\n",s->rdata);
 
}
void connection_callback(NEO::Session* s)
{
  printf("Connection occur, fd=%d\n",s->GetFD());
  s->SetRecvFunc(recv_callback);
  s->SetParseFunc(parse_callback);
}

int main(void)
{ 
  neos.makelisten(1234, connection_callback);
  for( ; ; ) {
    neos.do_recv_send(3);
    neos.do_parse_package();
  }
  exit(0);
}
