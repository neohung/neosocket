namespace NEO
{
//---------------------------------
#define uint8_t unsigned char
#define uint16_t unsigned short

template <int>struct Packet_Fixed{}; 

// Packet 0x7530: "version"
// pre:  ADMIN, BOOT, packet 0x7919
// post: packet 0x7531
// Request from client or ladmin for server version.
//
template<>
struct Packet_Fixed<0x7530>
{
  static const uint16_t PACKET_ID = 0x7530;
  uint16_t magic_packet_id = PACKET_ID;
};
// Packet 0x7531: "version result" (server回復client version資訊)
// pre:  packet 0x7530  (client送查詢server version要求)
// post: packet 0x0064  (client送登入帳號要求)
// Response to client's request for server version.
//
struct Version{
    uint8_t major;
    uint8_t minor; // flavor1
    uint8_t patch; // flavor2
    uint8_t devel; // flavor3
    uint8_t flags;
    uint8_t which;
    uint16_t vend;
};

template<>
struct Packet_Fixed<0x7531>
{
   static const uint16_t PACKET_ID = 0x7531;
   uint16_t magic_packet_id = PACKET_ID;
   Version version;
};


// Packet 0x0064: "account login"
// define: CMSG_LOGIN_REGISTER
// pre:  HUMAN, packet 0x7531
// post: packet 0x0063, packet 0x0069, packet 0x006a, packet 0x0081
// Authenticate a client by user/password.
//
// All clients must now set both defined version 2 flags.
//

#if defined(_MSC_VER)
#pragma pack( push, packing )
#pragma pack( 1 )
#endif

template<uint8_t n>
class VString {
  char _data[n];
//public:
//  char* toChar(void){return _data;};
}
#if defined(__GNUC__)
__attribute__((packed))
#endif
;


struct AccountName : VString<23> {

}
#if defined(__GNUC__)
__attribute__((packed))
#endif
;

struct AccountPass : VString<23> {

}
#if defined(__GNUC__)
__attribute__((packed))
#endif
;


template<>
struct Packet_Fixed<0x0064>
{
    static const uint16_t PACKET_ID = 0x0064;
    uint16_t magic_packet_id = 0x0064; //2 bytes
    uint32_t unknown = {} ;                //4bytes
    AccountName account_name = {};
    AccountPass account_pass = {};
    uint8_t version = {};
};
/*
#if defined(__GNUC__)
__attribute__((packed))
#endif
;
*/
#if defined(_MSC_VER)
#pragma pack( pop, packing )
#endif

//----
}