#include "hash_functions.h"

size_t polynomial_rolling_hash::operator() (std::string const & str) const {
    size_t hash = 0; //set variables
    size_t p = 1;
    size_t b = 19;
    size_t m = 3298534883309ul;
    for (size_t i = 0; i < str.length(); i++) {
        hash += str.at(i) * p;
        p = (p * b) % m;
    }
    return hash;

}

size_t fnv1a_hash::operator() (std::string const & str) const {
    size_t hash = 0xCBF29CE484222325; //set variables
    size_t prime = 0x00000100000001B3;
    for (size_t i = 0; i <str.length(); i++) {
        hash = hash ^ str.at(i);
        hash = hash * prime;
    }
    return hash;
}
