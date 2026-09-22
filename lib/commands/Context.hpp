#include <vector>
#include <string>

namespace Commands {
    class Context {
    private:
        std::vector<std::string> arguments;
        // ResponseFormater& out;
    public:
        void FillContext(std::vector<std::string> tokens) {
            
        }
    };
} // namespace Command 
