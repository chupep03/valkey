#pragma once

#include <stdexcept>

namespace Storage {

class StorageException : public std::runtime_error {
public:    
    explicit StorageException(const std::string& msg = "no details") : std::runtime_error("StorageException: " + msg) {} 
};

class OutOfMemoryException : public std::runtime_error {
public:
    explicit OutOfMemoryException(const std::string& msg = "no details") : std::runtime_error("OOMException: " + msg) {}
};

class ParseException : public std::runtime_error {
public:
    explicit ParseException(const std::string& msg = "no details") : std::runtime_error("ParseException: " + msg) {}
};

} // namespace Storage"
