#ifndef _A4_DRIVER_H_
#define _A4_DRIVER_H_

#include <a4/processor.h>

namespace a4{ namespace process{

class Driver {
    protected:
        void set_instream(Processor* p, shared<a4::io::InputStream> instream) { p->_instream = instream; };
        void set_outstream(Processor* p, shared<a4::io::OutputStream> outstream, bool fw=true) { 
            if (fw) outstream->set_forward_metadata();
            outstream->content_cls(p->content_class_id());
            outstream->metadata_cls(p->metadata_class_id());
            p->_outstream = outstream;
        };
        void set_backstore(Processor* p, shared<ObjectBackStore> bs) { p->_backstore = bs; };
        void set_store_prefix(Processor* p, const char * dir = "") { p->S = (*p->_backstore)(dir); };
        void set_metadata(Processor* p, A4Message md) { p->metadata_message = md; };
};

};}; // namespace a4::process

#endif
