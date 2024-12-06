#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>

// Declare functions in header
void write_test_file(const std::string& filename, const std::string& content);
std::string read_file(const std::string& filename);
bool file_exists(const std::string& filename);
void cleanup_test_files();

#endif
