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
      /*
      case 0x7530:       // Request of the server version
        {       
          printf("0x%04X: Request of the server version\n",s->peek_package_id());
          s->remove_rdata(sizeof(NEO::Packet_Fixed<0x7530>));
          NEO::Packet_Fixed<0x7531> fixed_7531;
          NEO::Version version = CURRENT_LOGIN_SERVER_VERSION;
          version.flags = 1;//login_conf.new_account ? 1 : 0;
          fixed_7531.version = version;
          //
          NEO::Buffer buf;
          buf.bytes.resize(sizeof(NEO::Packet_Fixed<0x7531>));
          auto& net_fixed = reinterpret_cast<NEO::Packet_Fixed<0x7531>&>(*(buf.bytes.begin() + 0));
          net_fixed = fixed_7531;
          s->send_wdata(buf.bytes.data(),buf.bytes.size());
        
        }
        break;
        case 0x0064:
        {
          printf("0x%04X: Request of login account\n",s->peek_package_id());
          printf("Packet_Fixed<0x0064> size=%d\n",sizeof(NEO::Packet_Fixed<0x0064>));
          //printf("rdata_size=%d\n",s->rdata_size );
          //for(int i=0;i<s->rdata_size;i++){
          //  printf("%d:[0x%x]\n",i,s->rdata[i]);
          //}
          auto net_fixed = reinterpret_cast<NEO::Packet_Fixed<0x0064>*>(s->rdata);
          printf("id=0x%04x\n",net_fixed->magic_packet_id);          
          printf("Account=[%s]\n",net_fixed->account_name.c_str());
          printf("PassWord=[%s]\n",net_fixed->account_pass.c_str());
		      //send update host package
          std::vector< NEO::Packet_Repeat<0x0063> > url;
          //char update_url[] = "http://127.0.0.1/aaa";
          //size_t total_size = sizeof(Packet_Head<0x0063>) + update_url.size() * sizeof(Packet_Repeat<0x0063>);
    


          s->remove_rdata(sizeof(NEO::Packet_Fixed<0x0064>));
        
        }

        break;
        */
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
void connection_callback(NEO::Session* s)
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
  neos.makeclient("192.168.85.130",6901, 3, connection_callback);
  //neos.makeclient("127.0.0.1",1234, 3, connection_callback);
  
  for( ; ; ) {
    neos.do_client_event(3);
    //neos.do_recv_send(3);
    neos.do_parse_package();
  }
  exit(0);
}
