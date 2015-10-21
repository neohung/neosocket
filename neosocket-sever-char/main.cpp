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
          printf("0x%04X: Request of connect char server\n",s->peek_package_id());
          auto net_fixed = reinterpret_cast<NEO::Packet_Fixed<0x0065>*>(s->rdata);
          printf("id=0x%04x\n",net_fixed->magic_packet_id);
          printf("AccountID=[0x%x]\n",net_fixed->account_id);
          printf("login_id1=[0x%x]\n",net_fixed->login_id1);
          printf("login_id2=[0x%x]\n",net_fixed->login_id2);
          printf("packet_client_version=[0x%x]\n",net_fixed->packet_client_version);
          printf("sex=[0x%x]\n",net_fixed->sex);
        /*
          //Do account 檢查
		      //send 0x0063 update host package
          char update_url[] = "http://192.168.85.130/updates/";
          std::vector< NEO::Packet_Repeat<0x0063> > repeat_0063;
          for (int i=0;i<sizeof(update_url);i++){
            NEO::Packet_Repeat<0x0063> rep_data;
            rep_data.c = update_url[i];
            repeat_0063.push_back(rep_data);
          }
          uint16_t len = (sizeof(NEO::Packet_Repeat<0x0063>)* repeat_0063.size())+ sizeof(NEO::Packet_Head<0x0063>);
          NEO::Packet_Head<0x0063> head_0063;
          head_0063.magic_packet_length = len;
          send_to_buffer(s,head_0063, repeat_0063);
          //Send 0x0069
          std::vector< NEO::Packet_Repeat<0x0069> > repeat_0069;
          for (int i=0;i<1;i++){
            NEO::Packet_Repeat<0x0069> rep_data;
             rep_data.ip = NEO::IP4Address({127,0,0,1});
             rep_data.port = 6122;
             char servername[23] = "The Mana World";
             auto tmp = reinterpret_cast<NEO::ServerName*>(servername);  
             rep_data.server_name = *tmp;
             rep_data.users = 0;
             rep_data.maintenance = 0;
             rep_data.is_new = 0;
            repeat_0069.push_back(rep_data);
          }
          len = (sizeof(NEO::Packet_Repeat<0x0069>)* repeat_0069.size())+ sizeof(NEO::Packet_Head<0x0069>);
          NEO::Packet_Head<0x0069> head_0069;
          head_0069.magic_packet_length = len; //0x004F
          head_0069.login_id1 = 0xFA437098;
          head_0069.account_id = NEO::AccountId(0x001E8480);
          head_0069.login_id2 = 0xE34F5D87;
          head_0069.unused = 0;
          char login_time[] = "2015-10-16 18:07:18.848";
          auto tmp = reinterpret_cast<NEO::timestamp_milliseconds_buffer*>(login_time);
          head_0069.last_login_string = *tmp;
          head_0069.unused2 = 0;
          head_0069.sex = NEO::SEX::MAN;
          send_to_buffer(s,head_0069, repeat_0069);
          */
          s->remove_rdata(sizeof(NEO::Packet_Fixed<0x0065>));
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
  neos.makelisten(6122, connection_callback);
  for( ; ; ) {
    neos.do_recv_send(3);
    neos.do_parse_package();
  }
  exit(0);
  }
}
