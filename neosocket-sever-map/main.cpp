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
template<typename  T>
void packet_dump(T t)
{
   char* tmp = reinterpret_cast<char*>(&t);
   printf("packet_dump:\nsize=%d\n",sizeof(T));
   printf(
          "---- 00-01-02-03-04-05-06-07  08-09-0A-0B-0C-0D-0E-0F\n");
    for (int i = 0; i < sizeof(t); i++)
    {
      if ((i & 15) == 0)
            printf("%04X ", i);
      if ((i - 8) % 16 == 0)  // -8 + 1
            printf(" ");
      printf("%02X ", (unsigned char) *(tmp+i));
      if ((i+1)% 16 == 0){
        printf("|\n");
      }
    }
    printf("\n");
    printf(
          "---- -----------------------  ------------------------\n");
}


template<int id>
void send_to_buffer(NEO::Session* s, NEO::Packet_Fixed<id> fixed)
{
  NEO::Buffer buf;
  buf.bytes.resize(sizeof(fixed));
  auto& net_fixed = reinterpret_cast<NEO::Packet_Fixed<id>&>(*(buf.bytes.begin() + 0));
  net_fixed = fixed;
  s->send_wdata(buf.bytes.data(),buf.bytes.size());
}

template<int id>
void send_to_buffer(NEO::Session* s, NEO::Packet_Payload<id> payload)
{
  NEO::Buffer buf;
  buf.bytes.resize(sizeof(payload));
  auto& net_payload = reinterpret_cast<NEO::Packet_Payload<id>&>(*(buf.bytes.begin() + 0));
  net_payload = payload;
  s->send_wdata(buf.bytes.data(),buf.bytes.size());
}

template<int id>
void send_to_buffer(NEO::Session* s, NEO::Packet_Head<id> head, std::vector< NEO::Packet_Repeat<id> > repeats)
{
  //send head
  NEO::Buffer buf;
  buf.bytes.resize(sizeof(NEO::Packet_Head<id>));
  auto& net_head = reinterpret_cast<NEO::Packet_Head<id>&>(*(buf.bytes.begin() + 0));
  net_head = head;
  s->send_wdata(buf.bytes.data(),buf.bytes.size());
  //send repeat
  for(int i=0;i<repeats.size();i++){
    NEO::Packet_Repeat<id> rep_data = repeats[i];
    NEO::Buffer buf;
    buf.bytes.resize(sizeof(NEO::Packet_Repeat<id>));
    auto& net_repeat = reinterpret_cast<NEO::Packet_Repeat<id>&>(*(buf.bytes.begin() + 0));
    net_repeat = rep_data;
    s->send_wdata(buf.bytes.data(),buf.bytes.size());
  }

}

void recv_callback(NEO::Session* s)
{
  s->rdata_dump();
}

void parse_callback(NEO::Session* s)
{
  if (s->rdata_size >=2){
    switch(s->peek_package_id())
    {
        case 0x0065:   //connect char server request
        {
        }
        break;
        default:
          printf("Got ID 0x%04X\n",s->peek_package_id());
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
  if (0){
          NEO::Session* s = (NEO::Session*)malloc(sizeof(NEO::Session));
          s->initSession();
    }else{
  neos.makelisten(5122, connection_callback);
  for( ; ; ) {
    neos.do_recv_send(3);
    neos.do_parse_package();
  }
  exit(0);
  }
}
