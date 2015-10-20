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

NEO::Packet_Fixed<0x0065> connect_char_fixed_0065;
            
NEO::Socket login_client;
NEO::Socket char_client;

template<int id>
void send_to_buffer(NEO::Session* s, NEO::Packet_Fixed<id> fixed)
{
  NEO::Buffer buf;
  buf.bytes.resize(sizeof(fixed));
  auto& net_fixed = reinterpret_cast<NEO::Packet_Fixed<id>&>(*(buf.bytes.begin() + 0));
  net_fixed = fixed;
  s->send_wdata(buf.bytes.data(),buf.bytes.size());
}


void recv_callback(NEO::Session* s)
{
  printf("recv occur, fd=%d\n",s->GetFD());
  s->rdata_dump();
}

void parse_callback(NEO::Session* s);

void char_connection_callback(NEO::Session* s)
{
 printf("char_connection_callback occur, fd=%d\n",s->GetFD());
  s->SetRecvFunc(recv_callback);
  s->SetParseFunc(parse_callback);
  //Request version info 0x7530
 // NEO::Packet_Fixed<0x7530> fixed_7530;
  send_to_buffer(s,connect_char_fixed_0065);
  //s->
}


void parse_callback(NEO::Session* s)
{
  if (s->rdata_size >=2){
    switch(s->peek_package_id())
    {
      case 0x7531:  // Server reply version result
      {
        printf("0x%04X: Server reply version result\n",s->peek_package_id());
        auto net_fixed = reinterpret_cast<NEO::Packet_Fixed<0x7531>*>(s->rdata);
        printf("Server version v%d.%d\n",net_fixed->version.major,net_fixed->version.minor);
        s->remove_rdata(sizeof(NEO::Packet_Fixed<0x7531>));
        //Send Packet_Fixed<0x0064>
        NEO::Packet_Fixed<0x0064> fixed_0064;
        fixed_0064.account_name.setString("neo02");
        fixed_0064.account_pass.setString("000000");
        fixed_0064.unknown = 0x05;
        fixed_0064.version = 0x03;
        send_to_buffer(s,fixed_0064);
      }
      break;
      case 0x006a:       // Account login error
      {
        printf("0x%04X: Account login error\n",s->peek_package_id());
        auto net_fixed = reinterpret_cast<NEO::Packet_Fixed<0x006a>*>(s->rdata);
        printf("error_code[%d]: %s\n",net_fixed->error_code,net_fixed->error_message.c_str());
        s->remove_rdata(sizeof(NEO::Packet_Fixed<0x006a>));
      }
      break;
      case 0x0063:       // receive update host notify
      {
          auto net_fixed = reinterpret_cast<NEO::Packet_Head<0x0063>*>(s->rdata);
          int repeats_size =  net_fixed->magic_packet_length - sizeof(NEO::Packet_Head<0x0063>);//sizeof(net_fixed->magic_packet_id) - sizeof(net_fixed->magic_packet_length);
          int repeat_size =  repeats_size / sizeof(NEO::Packet_Repeat<0x0063>);
          s->remove_rdata(sizeof(NEO::Packet_Head<0x0063>));
          char* update_url = (char*)malloc(repeat_size);
          for(int i=0;i<repeat_size;i++){       
            auto& net_repeat = reinterpret_cast<NEO::Packet_Repeat<0x0063>&>(s->rdata[0]); 
            update_url[i] = net_repeat.c;
            s->remove_rdata(sizeof(NEO::Packet_Repeat<0x0063>));
          }
          printf("Got update_url=[%s]\n",update_url);
          //Do some update
          //s->rdata_dump();
      }
      case 0x0069:       // receive update host notify
      {
        //s->rdata_dump();
        auto net_fixed = reinterpret_cast<NEO::Packet_Head<0x0069>*>(s->rdata);
        int repeats_size =  net_fixed->magic_packet_length - sizeof(NEO::Packet_Head<0x0069>);
        int repeat_size =  repeats_size / sizeof(NEO::Packet_Repeat<0x0069>);
        printf("AccountId: 0x%x\n",net_fixed->account_id.unwrap<NEO::AccountId>(net_fixed->account_id) ); 
        printf("login_id1: 0x%x\n",net_fixed->login_id1);
        printf("login_id2: 0x%x\n",net_fixed->login_id2);
        printf("sex: 0x%x\n", net_fixed->sex);
        printf("last login time: [%s]\n", net_fixed->last_login_string.c_str());
        //
        connect_char_fixed_0065.login_id1 = net_fixed->login_id1;
        connect_char_fixed_0065.login_id2 = net_fixed->login_id2;
        connect_char_fixed_0065.account_id = NEO::AccountId(net_fixed->account_id.unwrap<NEO::AccountId>(net_fixed->account_id));
        connect_char_fixed_0065.sex = net_fixed->sex;
        s->remove_rdata(sizeof(NEO::Packet_Head<0x0069>));
    
        printf("repeats_size=%d,%d,%d\n",repeats_size,sizeof(NEO::Packet_Repeat<0x0069>),repeat_size);
        char char_ip[16];
        int char_port;
        for(int i=0;i<repeat_size;i++){ 
          auto& net_repeat = reinterpret_cast<NEO::Packet_Repeat<0x0069>&>(s->rdata[0]); 
          sprintf(char_ip,"%d.%d.%d.%d",net_repeat.ip._addr[0],net_repeat.ip._addr[1],net_repeat.ip._addr[2],net_repeat.ip._addr[3]);
          char_port = net_repeat.port;
          printf("Got char server[%s:%d]\n",char_ip,char_port);
          printf("server name[%s]\n",net_repeat.server_name.c_str());
          s->remove_rdata(sizeof(NEO::Packet_Repeat<0x0069>));      
        }       
        //Create 0x0065 packet
        connect_char_fixed_0065.packet_client_version = 0x5;
        s->wdata_dump();
        char_client.makeclient(char_ip,char_port, 3, char_connection_callback);
      }
      break;
      case 0x006b:       // connect char success
      {
        s->rdata_dump();
        auto net_head = reinterpret_cast<NEO::Packet_Head<0x006b>*>(s->rdata);
        int repeats_size = net_head->magic_packet_length - sizeof(NEO::Packet_Head<0x006b>);
        int repeat_size =  repeats_size / sizeof(NEO::Packet_Repeat<0x006b>);
        s->remove_rdata(sizeof(NEO::Packet_Head<0x006b>));
    
        for(int i=0;i<repeat_size;i++){ 
          auto& net_repeat = reinterpret_cast<NEO::Packet_Repeat<0x006b>&>(s->rdata[0]); 
          NEO::CharSelect char_select = net_repeat.char_select;
          s->remove_rdata(sizeof(NEO::Packet_Repeat<0x006b>));      
        }
      }
      default:
          printf("Got ID 0x%04X\n",s->peek_package_id());
           s->clean_rdata();
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
void login_connection_callback(NEO::Session* s)
{
  printf("Connection occur, fd=%d\n",s->GetFD());
  s->SetRecvFunc(recv_callback);
  s->SetParseFunc(parse_callback);
  //Request version info 0x7530
  NEO::Packet_Fixed<0x7530> fixed_7530;
  send_to_buffer(s,fixed_7530);
  //s->
}

int main(void)
{
  login_client.makeclient("192.168.85.130",6901, 3, login_connection_callback);
  //login_client.makeclient("127.0.0.1",1234, 3, login_connection_callback);
  //printf("Wait connection.\n");
  for( ; ; ) {
    //printf(".");
    login_client.do_client_event(3);
    login_client.do_parse_package();
    if (char_client.connect_fd > 0){
      char_client.do_client_event(3);
      char_client.do_parse_package();
    }
  
  }
  exit(0);
}
