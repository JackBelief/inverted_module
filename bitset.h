#pragma once

#include <string>
#include <vector>
#include <boost/dynamic_bitset.hpp>

class DynamicBitSet
{
    public:
        DynamicBitSet() {

        }

        template<class... Args>
        DynamicBitSet(Args... args) : _bits(args...) {

        }

        ~DynamicBitSet() {

        }

    public:
        typedef DynamicBitSet self_type;
        typedef boost::dynamic_bitset<>::size_type size_type;
        static const size_t npos = boost::dynamic_bitset<>::npos;

    public:
        bool any() const { return _bits.any(); }
        bool none() const { return _bits.none(); }

        size_type size() const { return _bits.size(); }
        size_type count() const { return _bits.count(); }

        void clear() { _bits.clear(); }
        void resize(size_type num_bits, bool value = false) { _bits.resize(num_bits, value); }

        inline self_type operator^(const self_type &o) const { return self_type(_bits ^ o._bits); }
        inline self_type operator|(const self_type &o) const { return self_type(_bits | o._bits); }
        inline self_type & operator|=(const self_type &o) { _bits |= o._bits; return *this; }
        inline self_type operator&(const self_type &o) const { return self_type(_bits & o._bits); }
        inline self_type & operator&=(const self_type &o) { _bits &= o._bits; return *this; }
        inline self_type operator-(const self_type &o) { return (_bits - o._bits); }
        inline bool operator[](size_type pos) const { return _bits[pos]; }

        inline self_type& set(size_type n, bool val = true) { _bits.set(n, val); return *this; }
        inline self_type& set() { _bits.set(); return *this; }
        inline self_type& reset(size_type n) { _bits.reset(n); return *this; }
        inline self_type& reset() { _bits.reset(); return *this; }
        inline self_type& flip() {_bits.flip(); return *this;};

        inline size_type first() const { return _bits.find_first(); }
        inline size_type next(size_type pos) const { return _bits.find_next(pos); }
        inline std::string to_string() const {
    	      std::string str;
    	      boost::to_string( _bits, str);
            return str;
        }

    private:
        boost::dynamic_bitset<> _bits;
};

typedef struct MultiDynamicBitSet_t {
    DynamicBitSet w_b_bitset[2];				// [0]: 对应白名单 [1]: 对应黑名单
} MultiDynamicBitSet;





