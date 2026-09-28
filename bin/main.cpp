#include "../lib/application/Args.hpp"
#include "../lib/application/Repl.hpp"
#include "../lib/application/ResponseFormatter.hpp"
#include "../lib/commands/CommandReg.hpp"
#include "../lib/commands/RegisterAll.hpp"
#include "../lib/storage/MemoryManager.hpp"
#include "../lib/storage/StorageEngine.hpp"

#include <iostream>

int main(int argc, char** argv) {
    auto opts = Application::ParseArgs(argc, argv);

    if (opts.help_requested) {
        std::cout << Application::kUsage;
        return 0;
    }
    if (opts.parse_error) {
        std::cerr << "Error: " << opts.error_message << "\n\n" << Application::kUsage;
        return 1;
    }

    Storage::MemoryManager      mm(opts.maxmemory);
    Storage::StorageEngine      engine(mm);
    Commands::CommandReg        reg;
    Commands::RegisterAll(reg);
    Application::ResponseFormatter fmt;

    Application::Repl repl(std::cin, engine, mm, reg, fmt);
    repl.Run();

    return 0;
}