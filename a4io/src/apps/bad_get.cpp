#include <a4/dynamic_message.h>

#include <boost/type_traits.hpp>

// From CERN ROOT Rtypes.h
// typedef unsigned long long ULong64_t;
typedef unsigned long long ULong64_t;


#include <iostream>

int main(int argc, char* argv[]) {

    std::cout << "Convertable? " << (boost::is_convertible<ULong64_t, uint64_t>::value ? "true" : "false") << std::endl;
    std::cout << "Same? " << (boost::is_same<ULong64_t, uint64_t>::value ? "true" : "false") << std::endl;

    a4::io::FieldContent fc;
    
    uint64_t i = 100;
    fc.assign(i);
    
    ULong64_t j = fc;
    
    DEBUG("Variant typeid: ", fc.type());
    DEBUG("j = ", j);
            
    std::cout << "Fieldcontent value: '" << fc.str() << "'" << std::endl;
    
    fc.assign(10.);
    DEBUG("Variant typeid: ", fc.type());
    double x = fc;
    DEBUG("Value: ", x);
    
    return 0;
}
