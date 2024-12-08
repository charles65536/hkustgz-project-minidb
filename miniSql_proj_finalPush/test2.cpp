#include <cassert>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "sql_handle.hpp"
#include "test2.hpp"
#include <iostream>

// Helper to run main with specific input/output files
void run_main_with_files(const std::string& input_file, const std::string& output_file) {
    char* args[] = {
        const_cast<char*>("program_name"),  // argv[0] - program name (dummy value)
        const_cast<char*>(input_file.c_str()),
        const_cast<char*>(output_file.c_str()),
        nullptr
    };
    main(3, args);  // Call main with our constructed arguments
}

// In test2.cpp:

void run_tests_with_main() {
    cleanup_test_files();
    std::cout << "Running tests using direct main() calls...\n";
    
    try {
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
        
        run_main_with_files("test1.sql", "test1_output.txt");
        std::string output1 = read_file("test1_output.txt");
        assert(output1.find("id,name,balance") != std::string::npos);
        assert(output1.find("1,'Alice',100.50") != std::string::npos);
        assert(output1.find("2,'Bob',200.75") != std::string::npos);
        
        // Test 2: WHERE clause and multiple queries
        std::cout << "Test 2: WHERE clause and multiple queries...\n";
        write_test_file("test2.sql", R"(
            USE DATABASE test_db;
            SELECT name FROM users WHERE balance > 150;
            SELECT * FROM users WHERE id = 1;
        )");
        run_main_with_files("test2.sql", "test2_output.txt");
        std::string output2 = read_file("test2_output.txt");
        assert(output2.find("name\n'Bob'") != std::string::npos);
        assert(output2.find("id,name,balance\n1,'Alice',100.50") != std::string::npos);
        
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
        
        run_main_with_files("test3.sql", "test3_output.txt");
        std::string output3 = read_file("test3_output.txt");
        assert(output3.find("users.name,orders.product,orders.amount") != std::string::npos);
        assert(output3.find("'Alice','Book',29.99") != std::string::npos);
        assert(output3.find("'Alice','Pen',5.99") != std::string::npos);
        assert(output3.find("'Bob','Notebook',15.99") != std::string::npos);

        // Multiple INNER JOINs test
        std::cout << "Test 11: Multiple INNER JOINs...\n";
        write_test_file("test11.sql", R"(
            USE DATABASE test_db;
            CREATE TABLE students (
                id INTEGER,
                name TEXT,
                age INTEGER
            );
            CREATE TABLE courses (
                id INTEGER,
                name TEXT,
                credits INTEGER
            );
            CREATE TABLE enrollments (
                student_id INTEGER,
                course_id INTEGER,
                grade FLOAT
            );
            
            INSERT INTO students VALUES (1, 'Alice', 20);
            INSERT INTO students VALUES (2, 'Bob', 22);
            INSERT INTO courses VALUES (101, 'Math', 3);
            INSERT INTO courses VALUES (102, 'Physics', 4);
            INSERT INTO enrollments VALUES (1, 101, 85.5);
            INSERT INTO enrollments VALUES (1, 102, 92.0);
            INSERT INTO enrollments VALUES (2, 101, 78.5);
            
            SELECT * FROM students 
            JOIN enrollments ON students.id = enrollments.student_id 
            JOIN courses ON enrollments.course_id = courses.id;
        )");

        run_main_with_files("test11.sql", "test11_output.txt");
        std::string output11 = read_file("test11_output.txt");
        assert(output11.find("students.id,students.name,students.age,enrollments.student_id,enrollments.course_id,enrollments.grade,courses.id,courses.name,courses.credits") != std::string::npos);
        assert(output11.find("1,'Alice',20,1,101,85.50,101,'Math',3") != std::string::npos);
        assert(output11.find("1,'Alice',20,1,102,92.00,102,'Physics',4") != std::string::npos);
        assert(output11.find("2,'Bob',22,2,101,78.50,101,'Math',3") != std::string::npos);

        std::cout << "Test 12: Column selection with multiple JOINs...\n";
        write_test_file("test12.sql", R"(
            USE DATABASE test_db;
            SELECT students.name, courses.name, enrollments.grade 
            FROM students 
            JOIN enrollments ON students.id = enrollments.student_id 
            JOIN courses ON enrollments.course_id = courses.id;
        )");

        run_main_with_files("test12.sql", "test12_output.txt");
        std::string output12 = read_file("test12_output.txt");
        assert(output12.find("students.name,courses.name,enrollments.grade") != std::string::npos);
        assert(output12.find("'Alice','Math',85.50") != std::string::npos);
        assert(output12.find("'Alice','Physics',92.00") != std::string::npos);
        assert(output12.find("'Bob','Math',78.50") != std::string::npos);

        cleanup_test_files();
        std::cout << "All main()-based tests passed successfully!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with error: " << e.what() << std::endl;
        cleanup_test_files();
        throw;
    }
}
// This function would be referenced from main.cpp when arg is "test2"
void run_alternative_tests() {
    try {
        run_tests_with_main();
    } catch (const std::exception& e) {
        std::cerr << "Alternative test suite failed: " << e.what() << std::endl;
        throw;
    }
}
