#include "Resistor.h"

void Resistor::ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) {
    sunrealtype *y_data = N_VGetArrayPointer(y);
    sunrealtype *F_data = N_VGetArrayPointer(F_Residual);

    sunrealtype Vp = (this->node1Index_ == 0) ? 0.0 : y_data[this->node1Index_ - 1];
    sunrealtype Vn = (this->node2Index_ == 0) ? 0.0 : y_data[this->node2Index_ - 1];
    sunrealtype current = (Vp - Vn) / this->resistance_;

    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += current;
    }

    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= current;
    }
}

void Resistor::DCStamp(N_Vector y, N_Vector F) {
    sunrealtype *y_data = N_VGetArrayPointer(y);
    sunrealtype *F_data = N_VGetArrayPointer(F);

    sunrealtype Vp = (node1Index_ == 0) ? 0.0 : y_data[this->node1Index_ - 1];

    sunrealtype Vn = node2Index_ == 0 ? 0.0 : y_data[this->node2Index_ - 1];

    sunrealtype current = (Vp - Vn) / this->resistance_;

    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += current;
    }

    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= current;
    }

}

