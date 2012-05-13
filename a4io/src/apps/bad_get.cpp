#include <boost/variant.hpp>
#include <boost/variant/get.hpp>

// From CERN ROOT Rtypes.h
// typedef unsigned long long ULong64_t;
typedef unsigned long long ULong64_t;

#include <iostream>

typedef boost::variant<uint32_t, uint64_t> TestVariant;

int main(int argc, char* argv[]) {
    std::cout << "Sizeof(uint64_t) = " << sizeof(uint64_t) << std::endl;
    std::cout << "Sizeof(ULong64_t) = " << sizeof(ULong64_t) << std::endl;

    uint64_t value = 1234;
    TestVariant content = value;
    
    std::cout << "Fetching as uint64_t: "; 
    std::cout << boost::get<uint64_t>(content) << std::endl;
    std::cout << "Fetching as ULong64_t: " << std::endl; 
    std::cout << boost::get<ULong64_t>(content) << std::endl;
    return 0;
}
