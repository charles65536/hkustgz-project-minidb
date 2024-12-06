#include "test_utils.hpp"



void write_test_file(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    file << content;
}

std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool file_exists(const std::string& filename) {
    std::ifstream f(filename.c_str());
    return f.good();
}

void cleanup_test_files() {
    std::filesystem::remove_all("./dbs/test_db");
    std::filesystem::remove_all("./dbs/db_university");
    for (int i = 1; i <= 10; i++) {
        std::filesystem::remove("test" + std::to_string(i) + ".sql");
        std::filesystem::remove("test" + std::to_string(i) + "_output.txt");
    }
}
