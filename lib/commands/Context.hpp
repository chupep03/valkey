#pragma once 

#include "../storage/MemoryManager.hpp"
#include "../storage/StorageEngine.hpp"
#include "CommandException.hpp"
#include "IResponse.hpp"

#include <vector>
#include <charconv>
#include <string>

namespace Commands {

class Context {
public:
    using ArgsType = std::vector<std::string>;
    using SE_ref = Storage::StorageEngine&;
    using MM_ref = Storage::MemoryManager&;

private:
    std::string cmd_name_;
    ArgsType args_;
    SE_ref storage_;
    MM_ref mm_;
    IResponse& out_;

public:
    Context(std::string cmd_name, ArgsType args, SE_ref storage, MM_ref mm, IResponse& out) :
        cmd_name_(cmd_name), args_(args), storage_(storage), mm_(mm), out_(out) {}

    ArgsType& Args() {return args_;}
    size_t GetArgsCount() {return args_.size();}

    std::string_view GetArgumentAsStr(std::size_t i) const {
        if (i >= args_.size()) {
            throw WrongTypeError();
        }
        return args_[i];
    }

    long long GetArgumentAsLLInt(std::size_t i) const {
        auto sv = GetArgumentAsStr(i);
        long long value = 0;
        auto [ptr, ec] = std::from_chars(
            sv.data(), sv.data() + sv.size(), value);
        if (ec != std::errc{} || ptr != sv.data() + sv.size()) {
            throw WrongTypeError();
        }
        return value;
    }

    double GetArgumentAsDouble(std::size_t i) const {
        auto sv = GetArgumentAsStr(i);
        double value = 0.0;
        auto [ptr, ec] = std::from_chars(
            sv.data(), sv.data() + sv.size(), value);
        if (ec != std::errc{} || ptr != sv.data() + sv.size()) {
            throw WrongTypeError{};;
        }
        return value;
    }

    void RequireArgs(size_t n) {if (args_.size() != n) throw ArgsError("RequireArgs");}
    void RequireMinArgs(size_t n) const {if (args_.size() < n) throw ArgsError("RequireMinArgs");}

    Storage::StorageEngine& Storage() noexcept { return storage_;}
    Storage::MemoryManager& Memory() noexcept { return mm_; }
    IResponse& Out() noexcept { return out_; }
};

} // namespace Command 
