#ifndef _A4_PROCESS_CONFIGURATION_H_
#define _A4_PROCESS_CONFIGURATION_H_

namespace boost {
namespace program_options {
    class options_description_easy_init;
    class varables_map;
}
}


namespace a4 {
namespace process {

class Processor;


class Configuration {
    public: 
        virtual ~Configuration() {};
        /// Override this to add options to the command line and configuration file
        virtual void add_options(po::options_description_easy_init) {};
        /// Override this to do further processing of the options from the command line or config file
        virtual void read_arguments(po::variables_map &arguments) {};
        virtual void setup_processor(Processor &g) {};
        virtual Processor* new_processor() = 0;
};


template<class MyProcessor>
class ConfigurationOf : public Configuration {
    public:
        /// Override this to setup your thread-safe Processor!
        virtual void setup_processor(MyProcessor& g) {}

        virtual void setup_processor(Processor& g) { 
            setup_processor(dynamic_cast<MyProcessor&>(g)); 
        }
        virtual Processor* new_processor() {
            Processor* p = new MyProcessor();
            p->my_configuration = this;
            return p;
        }
};


}
}

#endif // _A4_PROCESS_CONFIGURATION_H_
