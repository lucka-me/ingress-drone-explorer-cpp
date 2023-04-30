#include "command.hpp"

#include <iostream>

#include <boost/program_options.hpp>

#include "extensions/iostream_extensions.hpp"
#include "explorer/explorer_t.hpp"

namespace ingress_drone_explorer {

void command::execute(const int argc, const char* const argv[]) {
    std::vector<std::string> portal_list_filenames;
    coordinate_t start;

    boost::program_options::options_description options;
    options.add_options()
        (
            "portal-list-files,p",
            boost::program_options::value<std::vector<std::string>>(&portal_list_filenames)->required(),
            "Paths of portal list files."
        )
        (
            "start,s",
            boost::program_options::value<coordinate_t>(&start)->required(),
            "The starting point."
        )
        ("key-list,k", boost::program_options::value<std::string>(), "Path of key list file.")
        ("output-drawn-items", boost::program_options::value<std::string>(), "Path of drawn items file to output.")
        ("help,h", "Show help information.");

    boost::program_options::positional_options_description positional_options;
    positional_options.add("portal-list-files", -1);

    boost::program_options::command_line_parser parser(argc, argv);
    parser.options(options).positional(positional_options);

    boost::program_options::variables_map variables;
    boost::program_options::store(parser.run(), variables);

    if (variables.count("help")) {
        std::cout << options << std::endl;
        return;
    }

    boost::program_options::notify(variables);

    explorer_t explorer;
    explorer.load_portals(portal_list_filenames);
    if (variables.count("key-list")) {
        explorer.load_keys(variables["key-list"].as<std::string>());
    }
    explorer.explore_from(start);
    explorer.report();
    if (variables.count("output-drawn-items")) {
        explorer.save_drawn_items_to(variables["output-drawn-items"].as<std::string>());
    }
}

} // namespace ingress_drone_explorer