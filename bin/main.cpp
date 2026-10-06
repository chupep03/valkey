#include "application/Args.hpp"
#include "application/Repl.hpp"
#include "application/ResponseFormatter.hpp"
#include "commands/CommandReg.hpp"
#include "commands/RegisterAll.hpp"
#include "storage/MemoryManager.hpp"
#include "storage/StorageEngine.hpp"

#include <iostream>

int main(int argc, char** argv) {
    auto opts = Application::ParseArgs(argc, argv);

    if (opts.help_requested) {
        std::cout << Application::kHelpResponse;
        return 0;
    }

    Storage::MemoryManager mm(opts.maxmemory);
    Storage::StorageEngine engine(mm);
    Commands::CommandReg reg;
    Commands::RegisterAll(reg);
    Application::ResponseFormatter fmt;

    Application::Repl repl(std::cin, engine, mm, reg, fmt);
    repl.Run();

    return 0;
}