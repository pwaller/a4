#include "a4/results.h"
#include <vector>
#include <string>
#include <boost/program_options.hpp>
using namespace std;

namespace po = boost::program_options;
typedef vector<string> Inputs;

int main(int argc, char ** argv) {
    if (argc == 1) {
        cout << "Usage: merge_results -o <output.result> <result1.result> <result2.result> ..." << endl;
    }
    po::options_description description("Allowed options");
    description.add_options()
        ("input,i", po::value<Inputs>(), "input file(s)")
    ;
    po::positional_options_description positional_options;
    positional_options.add("input", -1);

    po::variables_map arguments;
    po::store(po::command_line_parser(argc, argv).options(description).positional(positional_options).run(), arguments);

    Inputs inputs(arguments["input"].as<Inputs>());
    for (int i = 0; i < inputs.size(); i++) {
        Results::from_file(inputs[i])->print();
    }
    return 0;
};
