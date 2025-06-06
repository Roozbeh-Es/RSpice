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

    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += IL;
    }
    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= IL;
    }

    F_data[this->inductorEquationIndex_] += VL - (this->inductance_ * dIL + RMIN * IL);
}


void Inductor::DCStamp(N_Vector y, N_Vector F) {
    constexpr sunrealtype RMIN = 1e-9; // 1 nΩ, effectively a short circuit

    sunrealtype* y_data = N_VGetArrayPointer(y);
    sunrealtype* F_data = N_VGetArrayPointer(F);

    // Get the variables for this element from the solution vector y
    sunrealtype Vp = (this->node1Index_ == 0) ? 0.0 : y_data[this->node1Index_ - 1];
    sunrealtype Vn = (this->node2Index_ == 0) ? 0.0 : y_data[this->node2Index_ - 1];
    sunrealtype IL = y_data[this->inductorEquationIndex_]; // The inductor's current variable

    // 1. KCL Contribution: Stamp the inductor's current (IL) into the KCL equations
    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += IL;
    }
    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= IL;
    }

    // 2. Branch Equation: This provides the equation for the inductor's own MNA row.
    //    It enforces Ohm's law for the small resistor model: Vp - Vn = RMIN * IL
    //    The residual form is: (Vp - Vn) - (RMIN * IL) = 0
    F_data[this->inductorEquationIndex_] += (Vp - Vn) - (RMIN * IL);
}
