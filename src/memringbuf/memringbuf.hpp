#ifndef memringbuf_hpp
#define memringbuf_hpp

#include <stddef.h>
#include <stdint.h>
#include <array>
#include <memory>
#include <type_traits>


template<
    typename count_t, //assuming size_t or lesser unsigned type
    unsigned POWER // the buffer has capacity POWER of two
>
struct MemRingBuf{
    static_assert(std::is_unsigned<count_t>::value, "MemRingBuf: count_t shall be unsigned");
    static_assert((sizeof(count_t) <= sizeof(size_t)), "MemRingBuf: count_t shall not be longer than size_t");
    // count_t shall have at least one additional bit to distinguish between full and empty buffer state:
    static_assert((POWER + 1 <= sizeof(count_t) * 8), "MemRingBuf: Buffer size is too big for the given count_t");


    enum: count_t {
        SIZE = 1 << POWER,
        MASK = SIZE - 1 // to extract index from read or write counter
    };


    typedef int errcode;

    constexpr MemRingBuf(): wc_(0), rc_(0), data_{} {}

    count_t free_space(){
        return (SIZE - (wc_ - rc_));
    }

    count_t available(){
        return (wc_ - rc_);
    }

    errcode write(uint8_t* d, count_t n){
        if(free_space() < n) {
            return -1; // overrun
        }
        count_t wi = wc_ & MASK;

        if( n <=  SIZE - wi) {
            memcpy(data_.data() + wi, d , n);
        }else{
            count_t n1 = SIZE - wi;
            memcpy(data_.data() + wi, d, n1); // (may copy 0 bytes)
            memcpy(data_.data(), d + n1, n - n1 );
        }

        wc_ += n;
        return 0;
    }

    count_t read(uint8_t* buf, count_t capacity){
        count_t n = std::min(available(), capacity);

        count_t wi = wc_ & MASK;
        count_t ri = rc_ & MASK;

        if(n <= SIZE - ri) {
            memcpy(buf, data_.data(), n);
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

    count_t get_some(uint8_t** ppdata){
        count_t ri = rc_ & MASK;
        count_t n = std::min(available(), (count_t)(SIZE - ri));
        *ppdata = data_.data() + ri;
        return n;
    }

    // amount is usually the value that get_some() returned before:
    count_t consume(count_t amount){
        count_t n = std::min(available(), amount);
        rc_+= n;
        return n;
    }

    void clear(){
        wc_ = 0;
        rc_ = 0;
    }

private:

    count_t wc_; // write counter
    count_t rc_; // read counter

    std::array<uint8_t, SIZE> data_;
};


#endif
