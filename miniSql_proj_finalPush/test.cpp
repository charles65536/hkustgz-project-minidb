/*
#include <cassert>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "sql_handle.hpp"
#include <iostream>

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

void run_tests(const std::string& program_path) {
    cleanup_test_files();
    std::cout << "Running tests with program: " << program_path << "\n";

    // Test 1: Basic database and table operations
    std::cout << "Test 1: Basic database and table operations...\n";
    write_test_file("test1.sql", R"(
        CREATE DATABASE test_db;
        USE DATABASE test_db;
        CREATE TABLE users (
            id INTEGER,
            name TEXT,
            balance FLOAT
        );
        INSERT INTO users VALUES (1, 'Alice', 100.50);
        INSERT INTO users VALUES (2, 'Bob', 200.75);
        SELECT * FROM users;
    )");

    system((program_path + " test1.sql test1_output.txt").c_str());
    std::string output1 = read_file("test1_output.txt");
    assert(output1.find("id,name,balance") != std::string::npos);
    assert(output1.find("1,\"Alice\",100.50") != std::string::npos);
    assert(output1.find("2,\"Bob\",200.75") != std::string::npos);

    // Test 2: WHERE clause and multiple queries
    std::cout << "Test 2: WHERE clause and multiple queries...\n";
    write_test_file("test2.sql", R"(
        USE DATABASE test_db;
        SELECT name FROM users WHERE balance > 150;
        SELECT * FROM users WHERE id = 1;
    )");

    system((program_path + " test2.sql test2_output.txt").c_str());
    std::string output2 = read_file("test2_output.txt");
    assert(output2.find("name\n\"Bob\"") != std::string::npos);
    assert(output2.find("---") != std::string::npos);
    assert(output2.find("id,name,balance\n1,\"Alice\",100.50") != std::string::npos);

    // Test 3: Complex queries with joins
    std::cout << "Test 3: Complex queries with joins...\n";
    write_test_file("test3.sql", R"(
        USE DATABASE test_db;
        CREATE TABLE orders (
            user_id INTEGER,
            product TEXT,
            amount FLOAT
        );
        INSERT INTO orders VALUES (1, 'Book', 29.99);
        INSERT INTO orders VALUES (1, 'Pen', 5.99);
        INSERT INTO orders VALUES (2, 'Notebook', 15.99);
        SELECT users.name, orders.product, orders.amount 
        FROM users 
        INNER JOIN orders 
        ON users.id = orders.user_id;
    )");

    system((program_path + " test3.sql test3_output.txt").c_str());
    std::string output3 = read_file("test3_output.txt");
    assert(output3.find("name,product,amount") != std::string::npos);
    assert(output3.find("\"Alice\",\"Book\",29.99") != std::string::npos);
    assert(output3.find("\"Alice\",\"Pen\",5.99") != std::string::npos);
    assert(output3.find("\"Bob\",\"Notebook\",15.99") != std::string::npos);

    // Test 4: UPDATE and DELETE operations
    std::cout << "Test 4: UPDATE and DELETE operations...\n";
    write_test_file("test4.sql", R"(
        USE DATABASE test_db;
        UPDATE users SET balance = 150.00 WHERE id = 1;
        SELECT * FROM users WHERE id = 1;
        DELETE FROM users WHERE id = 2;
        SELECT * FROM users;
    )");

    system((program_path + " test4.sql test4_output.txt").c_str());
    std::string output4 = read_file("test4_output.txt");
    assert(output4.find("1,\"Alice\",150.00") != std::string::npos);
    assert(output4.find("2,\"Bob\",200.75") == std::string::npos);

    // Test 5: Error handling - syntax errors
    std::cout << "Test 5: Error handling - syntax errors...\n";
    write_test_file("test5.sql", R"(
        USE DATABASE test_db;
        SELEC * FORM users;
    )");

    int result5 = system((program_path + " test5.sql test5_output.txt").c_str());
    assert(result5 != 0);

    // Test 6: Error handling - semantic errors
    std::cout << "Test 6: Error handling - semantic errors...\n";
    write_test_file("test6.sql", R"(
        USE DATABASE test_db;
        SELECT * FROM nonexistent_table;
    )");

    int result6 = system((program_path + " test6.sql test6_output.txt").c_str());
    assert(result6 != 0);

    // Test 7: Database persistence
    std::cout << "Test 7: Database persistence...\n";
    write_test_file("test7.sql", R"(
        USE DATABASE test_db;
        SELECT * FROM users;
    )");

    system((program_path + " test7.sql test7_output.txt").c_str());
    std::string output7 = read_file("test7_output.txt");
    assert(output7.find("1,\"Alice\",150.00") != std::string::npos);

    // Test 8: Complex WHERE conditions
    std::cout << "Test 8: Complex WHERE conditions...\n";
    write_test_file("test8.sql", R"(
        USE DATABASE test_db;
        SELECT * FROM users WHERE id = 1 AND balance > 100;
        SELECT * FROM users WHERE id = 2 OR balance < 200;
    )");

    system((program_path + " test8.sql test8_output.txt").c_str());
    std::string output8 = read_file("test8_output.txt");
    assert(output8.find("1,\"Alice\",150.00") != std::string::npos);

    // Test 9: Multi-line SQL and formatting
    std::cout << "Test 9: Multi-line SQL and formatting...\n";
    write_test_file("test9.sql", R"(
        USE DATABASE 
            test_db;
        SELECT 
            name,
            balance
        FROM 
            users
        WHERE
            balance > 100.00;
    )");

    system((program_path + " test9.sql test9_output.txt").c_str());
    std::string output9 = read_file("test9_output.txt");
    assert(output9.find("name,balance") != std::string::npos);
    assert(output9.find("\"Alice\",150.00") != std::string::npos);

    // Test 10: DROP operations
    std::cout << "Test 10: DROP operations...\n";
    write_test_file("test10.sql", R"(
        USE DATABASE test_db;
        DROP TABLE orders;
        DROP DATABASE test_db;
    )");

    system((program_path + " test10.sql test10_output.txt").c_str());
    assert(!file_exists("./dbs/test_db/orders.csv"));

    // Clean up all test files
    cleanup_test_files();

    std::cout << "All file I/O tests passed successfully!\n";
}
*/
