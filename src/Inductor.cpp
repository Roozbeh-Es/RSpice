#include "Inductor.h"

void Inductor::ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) {
    constexpr sunrealtype RMIN = 1e-6; // Small resistance in series (1 µΩ)

    sunrealtype* y_data = N_VGetArrayPointer(y);
    sunrealtype* yp_data = N_VGetArrayPointer(yp);
    sunrealtype* F_data = N_VGetArrayPointer(F_Residual);

    sunrealtype Vp = (this->node1Index_ == 0) ? 0.0 : y_data[this->node1Index_ - 1];
    sunrealtype Vn = (this->node2Index_ == 0) ? 0.0 : y_data[this->node2Index_ - 1];
    sunrealtype IL = y_data[this->inductorEquationIndex_];
    sunrealtype dIL = yp_data[this->inductorEquationIndex_];
    sunrealtype VL = Vp - Vn;

    // Stamp KCL (IL flows into node1, out of node2)
    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += IL;
    }
    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= IL;
    }

    // Stamp KVL: Vp - Vn = L dIL/dt + RMIN * IL
    F_data[this->inductorEquationIndex_] += VL - (this->inductance_ * dIL + RMIN * IL);
}

void Inductor::DCStamp(N_Vector y, N_Vector F) {
    sunrealtype *y_data = N_VGetArrayPointer(y);
    sunrealtype *F_data = N_VGetArrayPointer(F);

    sunrealtype Vp = (node1Index_ == 0) ? 0.0 : y_data[this->node1Index_ - 1];

    sunrealtype Vn = node2Index_ == 0 ? 0.0 : y_data[this->node2Index_ - 1];

    sunrealtype resistance = 1e-5;
    sunrealtype current = (Vp - Vn) / resistance;

    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += current;
    }

    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= current;
    }

}
