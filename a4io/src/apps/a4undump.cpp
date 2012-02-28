#include <algorithm>

#include <limits>
using std::numeric_limits;

#include <utility>
#include <string>

#include <iostream>
#include <fstream>
#include <iomanip>

#include <map>
#include <unordered_map>


#include <boost/filesystem.hpp>
using boost::filesystem::current_path;
#include <boost/program_options.hpp>

#include <google/protobuf/text_format.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>

using google::protobuf::TextFormat;

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

using google::protobuf::compiler::DiskSourceTree;
using google::protobuf::compiler::MultiFileErrorCollector;
using google::protobuf::compiler::SourceTreeDescriptorDatabase;
using google::protobuf::Descriptor;
using google::protobuf::DescriptorPool;
using google::protobuf::DescriptorPoolDatabase;
using google::protobuf::DescriptorProto;
using google::protobuf::DynamicMessageFactory;
using google::protobuf::FieldDescriptor;
using google::protobuf::FileDescriptor;
using google::protobuf::MergedDescriptorDatabase;
using google::protobuf::Message;
using google::protobuf::MessageFactory;
using google::protobuf::Reflection;

#include <a4/output.h>
#include <a4/message.h>
#include <dynamic_message.h>

#include <a4/io/A4Stream.pb.h>

//#include <a4/atlas/ntup/photon/Event.pb.h>
//#include <a4/atlas/ntup/smwz/Event.pb.h>
//#include <a4/atlas/Event.pb.h>

int main(int argc, char** argv) {
    a4::Fatal::enable_throw_on_segfault();

    namespace po = boost::program_options;

    std::vector<std::string> input_files;
    std::string output_file("out.a4"), compression_type("ZLIB 9"), proto_type;
        
    po::positional_options_description p;
    p.add("input", -1);

    po::options_description commandline_options("Allowed options");
    commandline_options.add_options()
        ("help,h", "produce help message")
        ("output,o", po::value(&output_file), "destination file (default: out.a4)")
        ("input-proto,p", po::value(&proto_type), ".proto file to construct")
        ("compression-type,C", po::value(&compression_type), "compression level '[TYPE] [LEVEL]'")
        ("input", po::value(&input_files), "input file names (runs once per specified file)")
    ;
    
    po::variables_map arguments;
    po::store(po::command_line_parser(argc, argv).
              options(commandline_options).positional(p).run(), arguments);
    po::notify(arguments);
    
    if (arguments.count("help") || !arguments.count("input"))
    {
        std::cout << "Usage: " << argv[0] << " [Options] input(s)" << std::endl;
        std::cout << commandline_options << std::endl;
        return 1;
    }
    
    
    a4::io::A4Output a4o(output_file, "Event");
    shared<a4::io::OutputStream> stream = a4o.get_stream("", true);
    std::stringstream ss(compression_type);
    std::string ctype; int level;
    ss >> ctype >> level;
    stream->set_compression(ctype, level);
    
    class ErrorCollector : public MultiFileErrorCollector {
        void AddError(const std::string& filename, int line, int column, const std::string& message) {
            FATAL("Proto import error in ", filename, ":", line, ":", column, " ", message);
        }
    };
    
    /// TODO(pwaller) encapsulate this into a class (alongside root2a4)
    DiskSourceTree source_tree;
    source_tree.MapPath("", current_path().string());
    source_tree.MapPath("a4/root/", "");
    
    SourceTreeDescriptorDatabase source_tree_db(&source_tree);
    ErrorCollector e;
    source_tree_db.RecordErrorsTo(&e);
    DescriptorPool source_pool(&source_tree_db, source_tree_db.GetValidationErrorCollector());
    
    DescriptorPoolDatabase builtin_pool(*DescriptorPool::generated_pool());
    MergedDescriptorDatabase merged_pool(&builtin_pool, &source_tree_db);
    
    DescriptorPool pool(&merged_pool, source_tree_db.GetValidationErrorCollector());
    
    DynamicMessageFactory dynamic_factory(&pool);
    dynamic_factory.SetDelegateToGeneratedFactory(true);
    
    const Descriptor* descriptor = NULL;
    /*
    if (tree_type == "test")
        descriptor = a4::root::test::Event::descriptor();
    else if (tree_type == "PHOTON")
        descriptor = a4::atlas::ntup::photon::Event::descriptor();
    else if (tree_type == "SMWZ")
        descriptor = a4::atlas::ntup::smwz::Event::descriptor();
    else
    */
    {        
        const FileDescriptor* file_descriptor = pool.FindFileByName(proto_type);
                
        descriptor = file_descriptor->FindMessageTypeByName("Event");
        
        if (!descriptor)
            FATAL("Couldn't find an \"Event\" class in ", proto_type);
    }
    
    const auto* default_instance = dynamic_factory.GetPrototype(descriptor);
    shared<Message> m(default_instance->New());
    
    foreach (auto& input_file, input_files) {
        std::ifstream input(input_file);    
        std::string line;
        while (std::getline(input, line)) {
            m->Clear();
            //m->ParseFromString(line);
            TextFormat::ParseFromString(line, m.get());
            stream->write(*m);
        }
    }
    
    return 0;
}

