#include "Capacitor.h"

void Capacitor::ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) {
    constexpr sunrealtype GMIN = 1e-9; // Small conductance in parallel (1 GΩ)

    sunrealtype* y_data = N_VGetArrayPointer(y);
    sunrealtype* yp_data = N_VGetArrayPointer(yp);
    sunrealtype* F_data = N_VGetArrayPointer(F_Residual);

    sunrealtype Vp = (this->node1Index_ == 0) ? 0.0 : y_data[this->node1Index_ - 1];
    sunrealtype Vn = (this->node2Index_ == 0) ? 0.0 : y_data[this->node2Index_ - 1];
    sunrealtype dVp = (this->node1Index_ == 0) ? 0.0 : yp_data[this->node1Index_ - 1];
    sunrealtype dVn = (this->node2Index_ == 0) ? 0.0 : yp_data[this->node2Index_ - 1];

    sunrealtype ic = this->capacitance_ * (dVp - dVn); // Capacitive current
    sunrealtype ig = GMIN * (Vp - Vn);                 // GMIN parallel current

    sunrealtype totalCurrent = ic + ig;

    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += totalCurrent;
    }
    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= totalCurrent;
    }
}
