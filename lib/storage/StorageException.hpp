#pragma once

#include <stdexcept>
#include <string>

namespace Storage {

class StorageException : public std::runtime_error {
public:    
    explicit StorageException(const std::string& msg = "no details") : std::runtime_error(msg) {} 
};

class OutOfMemoryException : public StorageException {
public:
    explicit OutOfMemoryException(const std::string& msg = "no details") : StorageException("OOMException: " + msg) {}
};

class ParseException : public StorageException {
public:
    explicit ParseException(const std::string& msg = "no details") : StorageException("ParseException: " + msg) {}
};

} // namespace Storage
