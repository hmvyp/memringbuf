#ifndef memringbuf_hpp
#define memringbuf_hpp

#include <stddef.h>
#include <stdint.h>
#include <string.h>  // memcpy
#include <array>
#include <algorithm> // std::min
#include <type_traits> // is_unsigned


template<
    unsigned POWER,// the buffer has capacity POWER of two
    typename count_t = unsigned //assuming size_t or lesser unsigned type
>
struct MemRingBuf{
    static_assert(std::is_unsigned<count_t>::value, "MemRingBuf: count_t shall be unsigned");
    static_assert((sizeof(count_t) <= sizeof(size_t)), "MemRingBuf: count_t shall not be longer than size_t");

    // count_t shall have at least one additional bit to distinguish between full and empty buffer state:
    static_assert((POWER + 1 <= sizeof(count_t) * 8), "MemRingBuf: Buffer size is too big for the given count_t");

    typedef int errcode;

    constexpr MemRingBuf(): wc_(0), rc_(0), data_{} {}

    size_t free_space(){
        return (count_t)(SIZE - (wc_ - rc_)); // cast to (count_t) to fix dangerous promotion to signed integer
    }

    size_t available(){
        return (count_t)(wc_ - rc_); // cast to (count_t) to fix dangerous promotion to signed integer
    }

    errcode write(uint8_t* d, size_t n){
        if(free_space() < n) {
            return -1; // overrun
        }
        count_t wi = wc_ & MASK;

        if( n <=  (size_t)(SIZE - wi)) {
            memcpy(data_.data() + wi, d , n);
        }else{
            count_t n1 = SIZE - wi;
            memcpy(data_.data() + wi, d, n1); // (may copy 0 bytes)
            memcpy(data_.data(), d + n1, n - n1 );
        }

        wc_ += (count_t)n;
        return 0;
    }

    count_t read(uint8_t* buf, size_t capacity){
        count_t n = (count_t) std::min(available(), capacity);

        count_t ri = rc_ & MASK;

        if(n <= SIZE - ri) {
            memcpy(buf, data_.data() + ri, n);
        }else{
            count_t n1 = SIZE - ri;
            memcpy(buf, data_.data() + ri, n1);
            memcpy(buf + n1, data_.data(), n - n1);
        }

        rc_ += n;
        return n;
    }

    // get_some() - consume() methods allow the reader to read data without copying
    // (by accessing buffer data directly):
    size_t get_some(uint8_t** ppdata){
        count_t ri = rc_ & MASK;
        *ppdata = data_.data() + ri;
        return std::min(available(), (size_t)(SIZE - ri));
    }

    // amount is usually a value returned by get_some():
    size_t consume(size_t amount){
        count_t n = (count_t)std::min(available(), amount);
        rc_+= n;
        return (size_t)n;
    }

    void clear(){
        wc_ = 0;
        rc_ = 0;
    }

private:

    enum: count_t {
        SIZE = 1 << POWER,
        MASK = SIZE - 1 // to extract index from read or write counter
    };

    count_t wc_; // write counter
    count_t rc_; // read counter

    std::array<uint8_t, SIZE> data_;
};


#endif
