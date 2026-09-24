#pragma once 

#include "../commands/IResponse.hpp"

#include <iostream>

namespace Application {
    
class ResponseFormatter : public Commands::IResponse {
private:
    std::ostream& out_;
    std::ostream& err_;    

public:
    explicit ResponseFormatter(std::ostream& out = std::cout, std::ostream& err = std::cerr)
        : out_(out), err_(err) {}

    void Ok() override {out_ << "OK\n";}                                                // OK
    void Nil() override {out_ << "(nil)\n";}                                            // NO RESPONSE
    void Int(long long n) override {out_ << "(integer) " << n << '\n';}                 // INT RESPONSE
    void Bulk(const std::string& s) override {out_ << '"' << s << "\"\n";}              // STRING RESPONSE IN BULKS
    void Error(const std::string& msg) override {err_ << "(error) " << msg << '\n';}    // ERROR RESPONSE

    void Array(const std::vector<std::string>& items) override {                        // ARRAY RESPONSE
        if (items.empty()) {
            out_ << "(empty array)\n";
            return;
        }
        for (std::size_t i = 0; i < items.size(); ++i) {
            out_ << (i + 1) << ") \"" << items[i] << "\"\n";
        }
    }

};

} // namespace Application


