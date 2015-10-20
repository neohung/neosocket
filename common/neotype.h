#ifndef __NEOTYPE_H__
#define __NEOTYPE_H__

#include <chrono> // For clock
#include <string.h> //For memcpy()
#include <functional>

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
enum class SEX : uint8_t {MAN=1,FEMAN=2};

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
    VString<n> to_lower() const;
    VString<n> to_upper() const;
  template<uint8_t len>
    __attribute__((format(printf, 2, 0)))
    int do_vprint(VString<len>& out, const char *fmt, va_list ap);
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
public:
    uint8_t _addr[4];
    constexpr IP4Address() : _addr{}
    {}
    constexpr explicit IP4Address(const uint8_t (&a)[4]) : _addr{a[0], a[1], a[2], a[3]}
    {}
    explicit IP4Address(struct in_addr addr)
    {
      static_assert(sizeof(addr) == sizeof(_addr), "4 bytes");
      *this = IP4Address(reinterpret_cast<const uint8_t (&)[4]>(addr));
    }
   
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


//
struct CharName
{
private:
    VString<23> _impl;
public:
    CharName() = default;
    explicit CharName(VString<23> name)
    : _impl(name)
    {}

    VString<23> to__actual() const
    {
        return _impl;
    }
    VString<23> to__lower() const
    {
        return _impl.to_lower();
    }
    VString<23> to__upper() const
    {
        return _impl.to_upper();
    }
    VString<23> to__canonical() const
    {
//#if NAME_IGNORING_CASE == 0
        return to__actual();
//#endif
//#if NAME_IGNORING_CASE == 1
//      return to__lower();
//#endif
    }

/*
    friend bool operator == (const CharName& l, const CharName& r)
    { return l.to__canonical() == r.to__canonical(); }
    friend bool operator != (const CharName& l, const CharName& r)
    { return l.to__canonical() != r.to__canonical(); }
    friend bool operator < (const CharName& l, const CharName& r)
    { return l.to__canonical() < r.to__canonical(); }
    friend bool operator <= (const CharName& l, const CharName& r)
    { return l.to__canonical() <= r.to__canonical(); }
    friend bool operator > (const CharName& l, const CharName& r)
    { return l.to__canonical() > r.to__canonical(); }
    friend bool operator >= (const CharName& l, const CharName& r)
    { return l.to__canonical() >= r.to__canonical(); }
*/
    friend
    VString<23> convert_for_printf(const CharName& vs) { return vs.to__actual(); }
};

class CharId : public Wrapped<uint32_t> 
{ 
  public: 
    constexpr CharId() : Wrapped<uint32_t>() {} 
  protected: 
    constexpr explicit CharId(uint32_t a) : Wrapped<uint32_t>(a) {} 
};

class ItemNameId : public Wrapped<uint16_t> 
{ 
  public: 
    constexpr ItemNameId() : Wrapped<uint16_t>() {} 
  protected: 
    constexpr explicit ItemNameId(uint16_t a) : Wrapped<uint16_t>(a) {} 
};

class Species : public Wrapped<uint16_t> 
{ 
  public: 
    explicit operator bool() const = delete; 
    bool operator !() const = delete; 
    constexpr Species() : Wrapped<uint16_t>() {} 
  protected: 
    constexpr explicit Species(uint16_t a) : Wrapped<uint16_t>(a) {} 
};


enum class Opt0 : uint16_t
{
    ZERO            = 0x0000,
    // [Fate] This is the GM `@hide' flag
    HIDE            = 0x0040,
    // [Fate] Complete invisibility to other clients
    INVISIBILITY    = 0x1000,
    // ?
    REAL_ANY_HIDE   = HIDE,
};

struct Stats6
{
    uint8_t str = {};
    uint8_t agi = {};
    uint8_t vit = {};
    uint8_t int_ = {};
    uint8_t dex = {};
    uint8_t luk = {};
};

struct CharSelect
{
    CharId char_id = {};
    uint32_t base_exp = {};
    uint32_t zeny = {};
    uint32_t job_exp = {};
    uint32_t job_level = {};
    ItemNameId shoes = {};
    ItemNameId gloves = {};
    ItemNameId cape = {};
    ItemNameId misc1 = {};
    Opt0 option = {};
    uint16_t unused = {};
    uint32_t karma = {};
    uint32_t manner = {};
    uint16_t status_point = {};
    uint16_t hp = {};
    uint16_t max_hp = {};
    uint16_t sp = {};
    uint16_t max_sp = {};
    uint16_t speed = {};
    Species species = {};
    uint16_t hair_style = {};
    uint16_t weapon = {};
    uint16_t base_level = {};
    uint16_t skill_point = {};
    ItemNameId head_bottom = {};
    ItemNameId shield = {};
    ItemNameId head_top = {};
    ItemNameId head_mid = {};
    uint16_t hair_color = {};
    ItemNameId misc2 = {};
    CharName char_name = {};
    Stats6 stats = {};
    uint8_t char_num = {};
    uint8_t unused2 = {};
};



class milli_clock
{
public:
    typedef std::chrono::milliseconds duration;
    typedef duration::rep rep;
    typedef duration::period period;
    typedef std::chrono::time_point<milli_clock, duration> time_point;
    static const bool is_steady = true; // assumed - not necessarily true
    static time_point now() noexcept;
} __ALIGNED__;
typedef milli_clock::time_point tick_t __ALIGNED__;
/// The difference between two points in time.
typedef milli_clock::duration interval_t __ALIGNED__;
/*
typedef std::function<void (TimerData *, tick_t)> timer_func;

struct TimerData
{
    /// This will be reset on call, to avoid problems.
    //Timer *owner;
    /// When it will be triggered
    tick_t tick;
    /// What will be done
    timer_func func;
    /// Repeat rate - 0 for oneshot
    interval_t interval;
    TimerData(Timer *o, tick_t t, timer_func f, interval_t i) : owner(o)
                                                               tick(t)
                                                              , func(std::move(f))
                                                              , interval(i)
    {}
};


class Timer
{
    friend struct TimerData;
    //dumb_ptr<TimerData> td;

    Timer(const Timer&) = delete;
    Timer& operator = (const Timer&) = delete;
public:
    /// Don't own anything yet.
    Timer() = default;
    /// Schedule a timer for the given tick.
    /// If you do not wish to keep track of it, call disconnect().
    /// Otherwise, you may cancel() or replace (operator =) it later.
    ///
    /// If the interval argument is given, the timer will reschedule
    /// itself again forever. Otherwise, it will disconnect() itself
    /// just BEFORE it is called.
    Timer(tick_t tick, timer_func func, interval_t interval=interval_t::zero());

    Timer(Timer&& t);
    Timer& operator = (Timer&& t);
    ~Timer() { cancel(); }

    /// Cancel the delivery of this timer's function, and make it falsy.
    /// Implementation note: this doesn't actually remove it, just sets
    /// the functor to do_nothing, and waits for the tick before removing.
    void cancel();
    /// Make it falsy without cancelling the timer,
    void detach();

    /// Check if there is a timer connected.
    explicit operator bool() { return bool(td); }
    /// Check if there is no connected timer.
    bool operator !() { return !td; }
};
*/
//----------------------------------
#ifdef _WIN32
#pragma pack( pop, packing )
#endif
}

#endif
