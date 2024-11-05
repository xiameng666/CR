#pragma once
#include <stdexcept>
#include <string>

class StudentManagementException : public std::runtime_error {
public:
    StudentManagementException(const std::string& message) : std::runtime_error(message) {}
};