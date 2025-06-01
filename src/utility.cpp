#include "utility.h"

bool check_sundials_flag(int flag, const std::string &func_name) {
    if (flag < 0) {
        std::cerr << "SUNDIALS Error: Function '" << func_name
                 << "' failed with flag = " << flag << std::endl;
        return false;
    }
    return true;
}

