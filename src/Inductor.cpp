#include  "Inductor.h"

void Inductor::ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) {
    sunrealtype* y_data = N_VGetArrayPointer(y);
    sunrealtype* yp_data = N_VGetArrayPointer(yp);
    sunrealtype* F_data = N_VGetArrayPointer(F_Residual);

    sunrealtype Vp = (this->node1Index_ == 0) ? 0.0 : y_data[this->node1Index_ - 1];
    sunrealtype Vn = (this->node2Index_ == 0) ? 0.0 : y_data[this->node2Index_ - 1];
    sunrealtype IL = y_data[this->inductorEquationIndex_];
    sunrealtype dIL_dt = yp_data[this->inductorEquationIndex_];
    sunrealtype VL = Vp - Vn;

    if (this -> node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += IL;
    }

    if (this -> node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= IL;
    }

    F_data[this->inductorEquationIndex_] += VL - (this->inductance_ * dIL_dt);
}
