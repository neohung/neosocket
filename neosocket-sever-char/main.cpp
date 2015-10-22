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
          printf("0x%04X: Request of connect char server\n",s->peek_package_id());
          auto net_fixed = reinterpret_cast<NEO::Packet_Fixed<0x0065>*>(s->rdata);
          printf("id=0x%04x\n",net_fixed->magic_packet_id);
          printf("AccountID=[0x%x]\n",net_fixed->account_id);
          printf("login_id1=[0x%x]\n",net_fixed->login_id1);
          printf("login_id2=[0x%x]\n",net_fixed->login_id2);
          printf("packet_client_version=[0x%x]\n",net_fixed->packet_client_version);
          printf("sex=[0x%x]\n",net_fixed->sex); 
          s->remove_rdata(sizeof(NEO::Packet_Fixed<0x0065>));
          //Do account 檢查
          //Send 0x8000 to special hold notify
          NEO::Packet_Payload<0x8000> payload_8000;
          payload_8000.magic_packet_length = sizeof(payload_8000);
          send_to_buffer(s, payload_8000);
          //After check AccountID
          //Send 0x006B connect char success
          std::vector< NEO::Packet_Repeat<0x006b> > repeat_006b;
          for (int i=0;i<1;i++){
            NEO::Packet_Repeat<0x006b> rep_data;
            NEO::CharSelect char_select;
            //
              char_select.char_id = char_select.char_id.wrap<NEO::CharId>(0x249f3);
              char_select.base_exp = 5;
              char_select.zeny = 35;
              char_select.job_exp = 5;
              char_select.job_level = 1;
              char_select.shoes = char_select.shoes.wrap<NEO::ItemNameId>(0);
              char_select.gloves = char_select.gloves.wrap<NEO::ItemNameId>(0);
              char_select.cape = char_select.cape.wrap<NEO::ItemNameId>(0);
              char_select.misc1 = char_select.misc1.wrap<NEO::ItemNameId>(0);
              char_select.option = NEO::Opt0::ZERO;
              char_select.unused = 0;
              char_select.karma = 0;
              char_select.manner = 0;
              char_select.status_point = 0;
              char_select.hp = 54;
              char_select.max_hp = 54;
              char_select.sp = 13;
              char_select.max_sp = 13;
              char_select.speed = 150;
              char_select.species = char_select.species.wrap<NEO::Species>(0);
              char_select.hair_style = 20;
              char_select.weapon = 0 ;
              char_select.base_level = 3;
              char_select.skill_point = 0;
              char_select.head_bottom = char_select.head_bottom.wrap<NEO::ItemNameId>(881);
              char_select.shield = char_select.shield.wrap<NEO::ItemNameId>(0);
              char_select.head_top = char_select.head_top.wrap<NEO::ItemNameId>(0);
              char_select.head_mid = char_select.head_mid.wrap<NEO::ItemNameId>(1202);
              char_select.hair_color = 0;
              char_select.misc2 = char_select.misc2.wrap<NEO::ItemNameId>(0);
              NEO::VString<23> cn;
              cn.setString("neo02");
              char_select.char_name = NEO::CharName(cn);
              NEO::Stats6 st;
                st.str=9;
                st.agi=9;
                st.vit=9;
                st.int_=1;
                st.dex=1;
                st.luk=1;        
              char_select.stats = st;
              char_select.char_num = 0;
              char_select.unused2 = 0;
            //
            rep_data.char_select = char_select;
            repeat_006b.push_back(rep_data);
          }
          uint16_t len = (sizeof(NEO::Packet_Repeat<0x006b>)* repeat_006b.size())+ sizeof(NEO::Packet_Head<0x006b>);
          NEO::Packet_Head<0x006b> head_006b;
          head_006b.magic_packet_length = len;
          send_to_buffer(s,head_006b, repeat_006b);
        }
        break;
        case 0x0066:   //select char 
        {
          //
          auto net_fixed = reinterpret_cast<NEO::Packet_Fixed<0x0066>*>(s->rdata);
          printf("Client choice char : %d\n",net_fixed->code);
          s->remove_rdata(sizeof(NEO::Packet_Fixed<0x0066>));
          //Send 0x0071 for MAP server info
          NEO::Packet_Fixed<0x0071> fixed_0071;
          fixed_0071.char_id = fixed_0071.char_id.wrap<NEO::CharId>(0x249f3);
          NEO::VString<15> mn;
          mn.setString("NEO MapSer");
          fixed_0071.map_name = NEO::MapName(mn);
          fixed_0071.ip = NEO::IP4Address({127,0,0,1});
          fixed_0071.port = 5122;
          send_to_buffer(s,fixed_0071);
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
          NEO::Packet_Payload<0x8000> payload_8000;
          payload_8000.magic_packet_length = 4;
          printf("size=%d\n",sizeof(payload_8000));

    }else{
  neos.makelisten(6122, connection_callback);
  for( ; ; ) {
    neos.do_recv_send(3);
    neos.do_parse_package();
  }
  exit(0);
  }
}
