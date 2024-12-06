#include "sql_handle.hpp"
#include "csv_manip.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

// Forward declarations
//void run_tests(const std::string& program_path);  // Original test function
void run_alternative_tests();  // New test2 function

int main(int argc, char* argv[]) {
    if (argc == 2) {
        /*
        if (std::string(argv[1]) == "test") {
            run_tests(argv[0]);
            return 0;
        }
         */
        if (std::string(argv[1]) == "test2") {
            run_alternative_tests();
            return 0;
        }
    }

    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " input.sql output.txt\n";
        std::cerr << "   or: " << argv[0] << " test\n";
        std::cerr << "   or: " << argv[0] << " test2\n";
        return 1;
    }

    try {
        std::ifstream sql_file(argv[1], std::ios::binary);
        if (!sql_file) {
            throw std::runtime_error("Could not open input file " + std::string(argv[1]));
        }
        std::stringstream buffer;
        buffer << sql_file.rdbuf();
        
        std::ofstream output(argv[2]);
        if (!output) {
            throw std::runtime_error("Could not open output file " + std::string(argv[2]));
        }

        SqlInterpreter interpreter;
        interpreter.execute(buffer.str());

        // Simplified output handling
        for (const auto& table : interpreter.outputTables) {
            output << csv_dumps(table, false, true);
            output << "---\n";
        }


    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
