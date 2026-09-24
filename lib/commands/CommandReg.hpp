#include <unordered_map>
#include <string>
#include <memory>

#include "ICommand.hpp"

namespace Commands {
class CommandReg {
private:
    std::unordered_map<std::string, std::unique_ptr<ICommand>> commands_;


    static std::string ToUpper(std::string s) {
        for (char& c : s) {
            c = static_cast<char>(
                std::toupper(static_cast<unsigned char>(c)));
        }
        return s;
    }

    ICommand* FindCmd(const std::string& name) const {
        auto it = commands_.find(ToUpper(name));
        if (it == commands_.end()) return nullptr;
        return it->second.get();
    }

public:
    void RegisterCommand(std::string& name, std::unique_ptr<ICommand> cmd) {
        commands_[ToUpper(name)] = std::move(cmd);
    }

    void RegisterAll(); // to do
};
} // namespace Command 
