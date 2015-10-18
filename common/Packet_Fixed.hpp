#ifndef __PACKET_FIXED_HPP__
#define __PACKET_FIXED_HPP__

#include "neotype.h"
namespace NEO
{
//__attribute__((deprecated))
#ifdef _WIN32
 #pragma pack( push, packing )
 #pragma pack( 1 )
 #define __ALIGNED__
#else
 #define __ALIGNED__ __attribute__((packed,, aligned(1)))
#endif

//---------------------------------
template <int>struct Packet_Fixed{} __ALIGNED__;
template <int>struct Packet_Head{} __ALIGNED__;
template <int>struct Packet_Repeat{} __ALIGNED__;
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
}__ALIGNED__;
// Packet 0x7531: "version result" (server回復client version資訊)
// pre:  packet 0x7530  (client送查詢server version要求)
// post: packet 0x0064  (client送登入帳號要求)
// Response to client's request for server version.
//

template<>
struct Packet_Fixed<0x7531>
{
   static const uint16_t PACKET_ID = 0x7531;
   uint16_t magic_packet_id = PACKET_ID;
   Version version;
}__ALIGNED__;


// Packet 0x0064: "account login"
// define: CMSG_LOGIN_REGISTER
// pre:  HUMAN, packet 0x7531
// post: packet 0x0063, packet 0x0069, packet 0x006a, packet 0x0081
// Authenticate a client by user/password.
//
// All clients must now set both defined version 2 flags.
//
template<>
struct Packet_Fixed<0x0064>
{
    static const uint16_t PACKET_ID = 0x0064;
    uint16_t magic_packet_id = 0x0064; //2 bytes
    uint32_t unknown = {} ;                //4bytes = 0x00000005
    AccountName account_name = {};
    AccountPass account_pass = {};
    uint8_t version = {};              //1bytes = 0x03
}__ALIGNED__;

// Packet 0x0063: "update host notify"
// define: SMSG_UPDATE_HOST
// pre:  packet 0x0064
// post: IDLE
// This packet gives the client the location of the update server URL, such as http://tmwdata.org/updates/
//
// It is only sent if an update host is specified for the server (there is one in the default configuration) and the client identifies as accepting an update host (which all supported clients do).
//

//send_packet_repeatonly<0x0063, 4, 1>(s, login_conf.update_host);
template<>
struct Packet_Head<0x0063>
{
    static const uint16_t PACKET_ID = 0x0063;
    // TODO remove this
    uint16_t magic_packet_id = PACKET_ID;
    // TODO remove this
    uint16_t magic_packet_length = {};
}__ALIGNED__;

template<>
struct Packet_Repeat<0x0063>
{
    static const uint16_t PACKET_ID = 0x0063;
    uint8_t c = {};
}__ALIGNED__;


// Packet 0x006a: "account login error"
// define: SMSG_LOGIN_ERROR
// pre:  packet 0x0064
// post: PRETTY
// Failure to log in.
//
// Error codes:
// * 0: unregistered id
// * 1: incorrect password
// * 2: expired id (unused?)
// * 3: rejected from server (unused?)
// * 4: permanently blocked
// * 5: client too old (unused?)
// * 6: temporary ban (date in 'error message' field)
// * 7: server full
// * 8: no message
// * 99: id erased
//
template<>
struct Packet_Fixed<0x006a>
{
    static const uint16_t PACKET_ID = 0x006a;

    // TODO remove this
    uint16_t magic_packet_id = PACKET_ID;
    uint8_t error_code = {};
    timestamp_seconds_buffer error_message = {};
}__ALIGNED__;


// Packet 0x0069: "account login success"
// define: SMSG_LOGIN_DATA
// pre:  packet 0x0064
// post: packet 0x0065
// Big blob of information available once when you authenticate:
//
// * dumb session keys
// * sex and last login
// * list of char server
//
template<>
struct Packet_Head<0x0069>
{
    static const uint16_t PACKET_ID = 0x0069;
    // TODO remove this
    uint16_t magic_packet_id = PACKET_ID;
    // TODO remove this
    uint16_t magic_packet_length = {};
    uint32_t login_id1 = {};
    AccountId account_id = {};
    uint32_t login_id2 = {};
    uint32_t unused = {};
    timestamp_milliseconds_buffer last_login_string = {};
    uint16_t unused2 = {};
    SEX sex = {};
}__ALIGNED__;
template<>
struct Packet_Repeat<0x0069>
{
    static const uint16_t PACKET_ID = 0x0069;
    IP4Address ip = {};
    uint16_t port = {};
    ServerName server_name = {};
    uint16_t users = {};
    uint16_t maintenance = {};
    uint16_t is_new = {};
}__ALIGNED__;



//--------------------------
#ifdef _WIN32
#pragma pack( pop, packing )
#endif
//---------------------------
}

#endif