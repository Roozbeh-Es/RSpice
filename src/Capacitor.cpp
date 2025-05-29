#include "Capacitor.h"

void Capacitor::ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) {
    sunrealtype *yp_data = N_VGetArrayPointer(yp);
    sunrealtype *F_data = N_VGetArrayPointer(F_Residual);

    sunrealtype Vp_prime = (this->node1Index_ == 0) ? 0.0 : yp_data[this->node1Index_ - 1];
    sunrealtype Vn_prime = (this->node2Index_ == 0) ? 0.0 : yp_data[this->node2Index_ - 1];

    sunrealtype current = this->capacitance_ * (Vp_prime - Vn_prime);

    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += current;
    }

    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= current;
    }
}
