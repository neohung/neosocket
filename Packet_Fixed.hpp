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
  //uint16_t magic_packet_id = PACKET_ID;
};
// Packet 0x7531: "version result"
// pre:  packet 0x7530
// post: packet 0x0064
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
//----
}