#include "Capacitor.h"

void Capacitor::ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) {
    sunrealtype* y_data = N_VGetArrayPointer(y);
    sunrealtype* yp_data = N_VGetArrayPointer(yp);
    sunrealtype* F_data = N_VGetArrayPointer(F_Residual);

    sunrealtype Vp = (this->node1Index_ == 0) ? 0.0 : y_data[this->node1Index_ - 1];
    sunrealtype Vn = (this->node2Index_ == 0) ? 0.0 : y_data[this->node2Index_ - 1];
    sunrealtype dVp = (this->node1Index_ == 0) ? 0.0 : yp_data[this->node1Index_ - 1];
    sunrealtype dVn = (this->node2Index_ == 0) ? 0.0 : yp_data[this->node2Index_ - 1];

    sunrealtype current = this->capacitance_ * (dVp - dVn);

    // Stamp into node equations
    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += current;
    }
    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= current;
    }
}
