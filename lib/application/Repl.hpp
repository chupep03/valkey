#include "ResponseFormatter.hpp"
#include "../parser/Tokenizer.hpp"
#include "../commands/CommandReg.hpp"
#include "../commands/Context.hpp"
#include "../commands/CommandException.hpp"
#include "../storage/StorageEngine.hpp"
#include "../storage/MemoryManager.hpp"
#include "../storage/OOMException.hpp"

#include <cctype>
#include <exception>
#include <istream>
#include <string>
#include <utility>
#include <vector>

namespace Application {
   
class Repl {
private:
    using SE_ref = Storage::StorageEngine&;
    using MM_ref = Storage::MemoryManager&;
    using Reg_ref = Commands::CommandReg&;
    using Response_ref = Commands::IResponse&;

    std::istream& input_;
    SE_ref storage_;
    MM_ref mm_;
    Reg_ref register_;
    Response_ref response_;

    static bool IsExit(const std::string& s) {
        if (s.size() != 4) return false;
        return std::toupper(static_cast<unsigned char>(s[0])) == 'E'
            && std::toupper(static_cast<unsigned char>(s[1])) == 'X'
            && std::toupper(static_cast<unsigned char>(s[2])) == 'I'
            && std::toupper(static_cast<unsigned char>(s[3])) == 'T';
    }

    static std::string ToLower(std::string s) {
        for (char& c : s) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return s;
    }

public:
    Repl(std::istream& input, SE_ref se, MM_ref mm, Reg_ref reg, Response_ref resp) :
        input_(input), storage_(se), mm_(mm), register_(reg), response_(resp) {}

    void Run() {
        std::string line;
        while(std::getline(input_, line)) {
            Parser::Tokenizer tokenize;
            auto tokens = tokenize(line);
            if (tokens.empty()) continue;
            if (IsExit(tokens[0])) break;

            const std::string name = tokens[0];
            auto* cmd = register_.FindCommand(name);

            if (cmd == nullptr) {
                response_.Error("unknown command '" + ToLower(name) + "'");
                continue;
            }

             std::vector<std::string> args(tokens.begin() + 1, tokens.end());

            try {
                Commands::Context ctx(
                    ToLower(name),
                    std::move(args),
                    storage_, mm_, response_);
                cmd->Execute(ctx);
            } catch (const Commands::CommandException& e) {
                response_.Error(e.what());
            } catch (const Storage::OOMException& e) {
                response_.Error(e.what());
            } catch (const std::exception& e) {
                response_.Error(std::string("internal error: ") + e.what());
            }
        }
    }

};

}