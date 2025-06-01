#ifndef UTILITY_H
#define UTILITY_H
#include <string>
#include <iostream>
#include <stdexcept>
#include <sundials/sundials_types.h>

/*class SundialsSolverException : public std::runtime_error {
private:
    std::string function_name;
    int error_flag_;

public:
    SundialsSolverException(const std::string &function_name, int error_flag) : std::runtime_error(
            "SUNDIALS error in function '" + function_name + "': Flag = " + std::to_string(error_flag)),
        function_name(function_name), error_flag_(error_flag) {
    }

    [[nodiscard]] const std::string &getFunctionName() const {
        return function_name;
    }

    [[nodiscard]] int getErrorFlag() const { return error_flag_; }
};
*/
bool check_sundials_flag(int flag, const std::string& func_name);

#endif //UTILITY_H
