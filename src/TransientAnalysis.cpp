#include "TransientAnalysis.h"
#include "Circuit.h"
#include "Element.h"
#include <sundials/sundials_nvector.h>

int transientResidualFunction(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_residual, void *user_data) {
    Circuit* circuit = static_cast<Circuit *>(user_data);
    if(!circuit) {
        std::cerr << "ERROR: User_data in IDA residual callback is NULL!" << std::endl;
        return -1;
    }

    N_VConst(0.0, F_residual);

    for (const auto& element_ptr : circuit->getElements()) {
        if(element_ptr) {
            element_ptr->ResidualStamp(t, y, yp, F_residual);
        }
    }
    return 0;
}