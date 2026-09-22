//#include <CommandReg.h>
#include "Context.hpp"

namespace Commands {
    class ICommand {
    private:
        //Context cont;

    public:
        virtual void Execute(Context& cnt /*, StorageEngine& stor*/) {}
    };
} // namespace Command 
