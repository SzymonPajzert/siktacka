//
// Created by svp on 23.08.17.
//

#include <client/GUIConnection.hpp>
#include <parse/parser.hpp>

std::map<part_t, int> max_log_level {};

int main(int argc, char* argv[]) {
    auto params = parse_client(argc, argv);

    GUIConnection gui(params->gui_address);

    bool keep_going = true;
    while(keep_going) {
        std::string line;
        getline(std::cin, line);
        gui.write_line(line);
        std::cout << gui.read_line() << std::endl;
    }
}
