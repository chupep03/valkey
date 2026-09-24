#include "../storage/MemoryManager.hpp"
#include "../storage/StorageEngine.hpp"
#include "CommandError.hpp"
#include "IResponse.hpp"

#include <vector>
#include <string>

namespace Commands {

class Context {
public:
    using Args = std::vector<std::string>;
    using SE_ref = Storage::StorageEngine&;
    using MM_ref = Storage::MemoryManager&;

private:
    Args args_;
    std::string cmd_name_;
    SE_ref storage_;
    MM_ref mm_;
    IResponse& out_;

public:
    Context(std::string cmd_name, Args args, SE_ref storage, MM_ref mm, IResponse& out) :
        cmd_name_(cmd_name), args_(args), storage_(storage), mm_(mm), out_(out) {}

    Args& GetArgs() {return args_;}
    size_t GetArgsCount() {return args_.size();}
    std::string_view GetArgumentAsStr(size_t i) const {} // to do
    long long int GetArgumentAsLLInt(size_t i) const {} // to do
    double GetArgumentAsDouble(size_t i) const {} // to do

    void RequireArgs(size_t n) {if (args_.size() != n) throw CommandException(ArgsError{}, "RequireArgs");}
    void RequireMinArgs(size_t n) const {if (args_.size() < n) throw CommandException(ArgsError{}, "RequireMinArgs");}

    Storage::StorageEngine& GetStorage() noexcept { return storage_; }
    Storage::MemoryManager& GetMemory() noexcept { return mm_; }
    IResponse& GetOut() noexcept { return out_; }
};

} // namespace Command 
