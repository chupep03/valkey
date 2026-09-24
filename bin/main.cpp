#include "../lib/storage/MemoryManager.hpp"
#include "../lib/storage/StorageEngine.hpp"
#include "../lib/commands/CommandReg.hpp"
#include "../lib/commands/RegisterAll.hpp"
#include "../lib/application/ResponseFormatter.hpp"

int main() {
    std::cout << "Start\n";

    Storage::MemoryManager mm;
    Storage::StorageEngine engine(mm);
    Commands::CommandReg reg;
    Commands::RegisterAll(reg);
    Application::ResponseFormatter fmt;

    std::cout << "Defenitions\n";

    Commands::Context ctx("set",
        {"k", "hello"},
        engine, mm, fmt);
    reg.FindCommand("SET")->Execute(ctx);

    std::cout << "First\n";

    Commands::Context ctx2("get",
        {"k"},
        engine, mm, fmt);
    reg.FindCommand("GET")->Execute(ctx2);

    return 0;
}