#ifndef memringbuf_test_hpp
#define memringbuf_test_hpp

#include "memringbuf.hpp"

template<unsigned POWER, typename count_t, size_t TEST_BSIZE >
struct MemRingbufTest {
    MemRingbufTest(): use_read(false) {
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
            // amounts to write from input and to read into output:
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

            if(use_read){
                size_t nr = mrb.read(ob + oc, oa);
                oc += nr;
            } else {
                uint8_t* pdata;
                size_t nr = mrb.get_some(&pdata);
                memcpy(ob + oc,  pdata, nr);
                mrb.consume(nr);
                oc += nr;
            }


        }

        return memcmp(ib, ob, TEST_BSIZE) == 0;
    }


    bool use_read; // use read method instead of read_some() - consume()
    MemRingBuf<POWER, count_t> mrb;

    size_t ic;
    size_t oc;

    uint8_t ib[TEST_BSIZE];
    uint8_t ob[TEST_BSIZE];
};

template <typename count_t>
inline bool memringbif_tests(bool use_read = false){

    MemRingbufTest<7, count_t, 1000> test;
    test.use_read = use_read;

    bool res = true;

    res = res && test.test(5,7);
    res = res && test.test(57,65);
    res = res && test.test(171,53);
    res = res && test.test(37,147);
    res = res && test.test(127,3);

    return res;
}

inline bool memringbif_all_tests(){

    bool res = true;

    res = res && memringbif_tests<uint8_t>(false);
    res = res && memringbif_tests<uint8_t>(true);

    res = res && memringbif_tests<unsigned>(false);
    res = res && memringbif_tests<unsigned>(true);

    res = res && memringbif_tests<size_t>(false);
    res = res && memringbif_tests<size_t>(true);

    return res;
}



#endif
