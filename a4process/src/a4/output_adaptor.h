#ifndef _A4_PROCESS_OUTPUT_ADAPTOR_H_
#define _A4_PROCESS_OUTPUT_ADAPTOR_H_

#include <a4/types.h>

#include <a4/message.h>
using a4::io::A4Message;

#include <a4/input.h>
using a4::io::A4Input;
#include <a4/output.h>
using a4::io::A4Output;

#include <a4/input_stream.h>
using a4::io::InputStream;
#include <a4/output_stream.h>
using a4::io::OutputStream;

#include <a4/object_store.h>
using a4::store::ObjectStore;
using a4::store::ObjectBackStore;
#include <a4/storable.h>
using a4::store::Storable;


namespace a4 {
namespace process {

class Driver;
class Processor;


class OutputAdaptor {
    public:
        virtual void write(shared<const A4Message> m) = 0;
        virtual void metadata(shared<const A4Message> m) = 0;
        void write(const google::protobuf::Message& m);
        void metadata(const google::protobuf::Message& m);
};

class BaseOutputAdaptor : public OutputAdaptor {
    public:
        shared<A4Message> current_metadata;
        std::string merge_key, split_key; 
        A4Output* out;
        A4Output* res;
        shared<ObjectBackStore> backstore;

        BaseOutputAdaptor(Driver* d, Processor* p, bool forward_metadata, 
                          A4Output* out, A4Output* res) 
            : out(out), res(res), forward_metadata(forward_metadata), 
              in_block(false), driver(d), p(p), last_postfix("") 
        {
            merge_key = split_key = "";
            outstream.reset();
            resstream.reset();
            backstore.reset();
            start_block(); // This writes no metadata
        }
        
        virtual ~BaseOutputAdaptor() {}

        void start_block(std::string postfix="");
        void end_block();

        void new_outgoing_metadata(shared<const A4Message> new_metadata) {
            // Check if we merge the old into the new metadata
            // and hold off on writing it.
            bool merge = false;
            shared<A4Message> old_metadata = current_metadata;

            // Determine if merging is necessary
            if (old_metadata && merge_key != "") {
                merge = old_metadata->check_key_mergable(*new_metadata, merge_key);
            }

            if (merge) {
                //std::cerr << "Merging\n" << old_metadata.message()->ShortDebugString()
                //          << "\n...and...\n" << new_metadata.message()->ShortDebugString() << std::endl;
                *current_metadata += *new_metadata;
                //std::cerr << "...to...\n" << current_metadata->message()->ShortDebugString() << std::endl;
            } else { // Normal action in case of new metadata
                // If we are in charge of metadata, start a new block now...
                end_block();

                std::string postfix = "";
                if (split_key != "") 
                    postfix = new_metadata->assert_field_is_single_value(split_key);
                current_metadata.reset(new A4Message(*new_metadata));

                start_block(postfix);
            } // end of normal action in case of new metadata

        }

        virtual void metadata(shared<const A4Message> m) {
            FATAL("To write metadata manually, you have to change the metadata_behavior of the Processor!");
        }
    
        void write(shared<const A4Message> m) {
            if (!in_block) 
                FATAL("Whoa?? Writing outside of a metadata block? How did you do this?");
            
            if (outstream) 
                outstream->write(m);
        }

    protected:
        bool forward_metadata;
        shared<OutputStream> outstream, resstream;

        bool in_block;
        Driver* driver;
        Processor* p;
        std::string last_postfix;
};

class ManualOutputAdaptor : public BaseOutputAdaptor {
    public:
        ManualOutputAdaptor(Driver* d, Processor* p, bool forward_metadata, A4Output* out, A4Output* res) 
            : BaseOutputAdaptor(d, p, forward_metadata, out, res) {}
        
        void metadata(shared<const A4Message> m) {
            new_outgoing_metadata(m);
        }
};


}
}

#endif // _A4_PROCESS_OUTPUT_ADAPTOR_H_

