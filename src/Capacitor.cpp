#include "Capacitor.h"

#include "Capacitor.h"


static constexpr sunrealtype GMIN = 1e-9;

void Capacitor::ResidualStamp(sunrealtype t,
                              N_Vector y,
                              N_Vector yp,
                              N_Vector F_Residual)
{
    sunrealtype* y_data  = N_VGetArrayPointer(y);
    sunrealtype* yp_data = N_VGetArrayPointer(yp);
    sunrealtype* F_data  = N_VGetArrayPointer(F_Residual);


    sunrealtype Vp = (this->node1Index_ == 0)
                        ? 0.0
                        : y_data[this->node1Index_ - 1];
    sunrealtype Vn = (this->node2Index_ == 0)
                        ? 0.0
                        : y_data[this->node2Index_ - 1];

    sunrealtype dVp = (this->node1Index_ == 0)
                         ? 0.0
                         : yp_data[this->node1Index_ - 1];
    sunrealtype dVn = (this->node2Index_ == 0)
                         ? 0.0
                         : yp_data[this->node2Index_ - 1];

    sunrealtype iC = this->capacitance_ * (dVp - dVn);

    sunrealtype iG = GMIN * (Vp - Vn);

    sunrealtype iTotal = iC + iG;

    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += iTotal;
    }
    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= iTotal;
    }
}

void Capacitor::DCStamp(N_Vector y, N_Vector F) {
    sunrealtype *y_data = N_VGetArrayPointer(y);
    sunrealtype *F_data = N_VGetArrayPointer(F);

    sunrealtype Vp = (node1Index_ == 0) ? 0.0 : y_data[this->node1Index_ - 1];

    sunrealtype Vn = node2Index_ == 0 ? 0.0 : y_data[this->node2Index_ - 1];

    sunrealtype resistance = 1e6;
    sunrealtype current = (Vp - Vn) / resistance;

    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += current;
    }

    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= current;
    }

}

