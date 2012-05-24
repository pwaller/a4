

#include "a4/driver.h"

#include "a4/output_adaptor.h"
using a4::process::OutputAdaptor;

namespace a4 {
namespace process {


void BaseOutputAdaptor::start_block(std::string postfix) {
    if (out and (!outstream or postfix != last_postfix)) {
        outstream = out->get_stream(postfix);
        outstream->set_compression("ZLIB", 1);
        if (forward_metadata) 
            outstream->set_forward_metadata();
    }
    if (res and (!resstream or postfix != last_postfix)) {
        resstream = res->get_stream(postfix);
        resstream->set_compression("ZLIB", 1);
        resstream->set_forward_metadata();
    }
    backstore.reset(new ObjectBackStore());
    driver->set_store(p, backstore->store());
    if (outstream and forward_metadata and current_metadata) {
        current_metadata->unionize();                
        outstream->metadata(*current_metadata->message());
    }
    in_block = true;
}

void BaseOutputAdaptor::end_block() {            
    if (resstream && current_metadata) {
        current_metadata->unionize();
        resstream->metadata(*current_metadata->message());
    }
    if (backstore && resstream) {
        backstore->to_stream(*resstream);
    }
    backstore.reset();
    if (outstream and !forward_metadata and current_metadata) {
        current_metadata->unionize();
        outstream->metadata(*current_metadata->message());
    }
    in_block = false;
}


}
}
