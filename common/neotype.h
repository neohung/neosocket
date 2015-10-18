#ifndef __NEOTYPE_H__
#define __NEOTYPE_H__

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int
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

enum class SEX : uint8_t {MAN=0,FEMAN=1};

struct Version{
    uint8_t major;
    uint8_t minor; // flavor1
    uint8_t patch; // flavor2
    uint8_t devel; // flavor3
    uint8_t flags;
    uint8_t which;
    uint16_t vend;
}__ALIGNED__;

template<uint8_t n>
class VString {
  char _data[n];
  unsigned char _special;
public:
  const char *c_str() const{return _data;}; 
  void setString(const char* src){memcpy(_data,src,n);};
 

}__ALIGNED__;

struct AccountName : VString<23> {}__ALIGNED__;
struct AccountPass : VString<23> {}__ALIGNED__;
struct timestamp_seconds_buffer : VString<19> {}__ALIGNED__;
struct timestamp_milliseconds_buffer : VString<23> {}__ALIGNED__;
struct ServerName : VString<19> {}__ALIGNED__;

class RString 
    {
        struct Rep
        {
            size_t count;
            size_t size;
            char body[];
        };

        union
        {
            Rep *owned;
            const char *begin;
        } u;
        const char *maybe_end;
}__ALIGNED__;

class IP4Address
{
    uint8_t _addr[4];
}__ALIGNED__;

//Should implement Wrapped function
template<class R>
struct Wrapped
{
  R _value;
  typedef R wrapped_type;
//  
  template<class T> 
  struct Sub : T
  {
    constexpr Sub(typename T::wrapped_type v2): T(v2){}
  };
  template<class T> 
  constexpr typename T::wrapped_type unwrap(typename std::enable_if<true, T>::type w)
  {
    //傳回型態為wrapped_type的值
    return w._value;
  }
  template<class T>
  constexpr T wrap(typename T::wrapped_type v)
  {
    //建立新的wrap結構並傳回
    return Sub<T>(v);
  }
  template<class W>
  constexpr W next(W w)
  {
    return wrap<W>(unwrap<W>(w) + 1);
  }
  template<class W>
  constexpr W prev(W w)
  {
    return wrap<W>(unwrap<W>(w) - 1);
  }
    
protected:
  //constexpr for c++11 常量表達式
  constexpr Wrapped(R v=0) : _value(v){}
public:
  //explicit 禁止編譯時自動隱性轉換
  explicit operator bool () const { return _value; }
  bool operator !() const { return !_value; }   
       
}__ALIGNED__;

class AccountId : public Wrapped<uint32_t>{
public: 
  constexpr AccountId() : Wrapped<uint32_t>() {} 
//protected: 
  constexpr explicit AccountId(uint32_t a) : Wrapped<uint32_t>(a) {} 
}__ALIGNED__;

//----------------------------------
#ifdef _WIN32
#pragma pack( pop, packing )
#endif
}

#endif