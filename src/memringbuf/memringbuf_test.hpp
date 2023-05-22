#ifndef memringbuf_test_hpp
#define memringbuf_test_hpp

#include "memringbuf.hpp"

template<typename count_t, unsigned POWER, size_t TEST_BSIZE >
struct MemRingbufTest {
    MemRingbufTest(){
        reinit();
    }

    void reinit(){
        mrb.clear();
        ic = 0;
        oc = 0;
        for(size_t i = 0; i < TEST_BSIZE; ++i){
            ib[i] = (uint8_t)(i+5);
            ob[i] = 0xAA;
        }
    }

    bool test(unsigned imod, unsigned omod) {
        reinit();
        for(size_t k = 0 ;; ++k){
            size_t ia = std::min( imod - (k % imod) + 1, TEST_BSIZE - ic);
            size_t oa = std::min( (k % omod) + 1, TEST_BSIZE - oc);

            if(oa == 0) {
                break;
            }

            if(ia){
                if(mrb.write(ib + ic, ia) == 0){ // if success
                    ic += ia;
                }
            }

            if(k % 5 == 1){
               continue; // sometimes skip reading;
            }

            uint8_t* pdata;
            size_t nr = mrb.get_some(&pdata);
            memcpy(ob + oc,  pdata, nr);
            mrb.consume(nr);
            oc += nr;
        }

        return memcmp(ib, ob, TEST_BSIZE) == 0;
    }


    MemRingBuf<count_t, POWER> mrb;

    size_t ic;
    size_t oc;

    uint8_t ib[TEST_BSIZE];
    uint8_t ob[TEST_BSIZE];
};

inline bool memringbif_tests(){

    MemRingbufTest<uint8_t, 7, 1000> test;
    bool res = true;

    res = res && test.test(5,7);
    res = res && test.test(57,65);
    res = res && test.test(171,53);
    res = res && test.test(37,147);
    res = res && test.test(127,3);

    return res;
}



#endif
