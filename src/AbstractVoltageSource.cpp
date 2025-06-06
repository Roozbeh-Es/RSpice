#include "AbstractVoltageSource.h"

void AbstractVoltageSource::ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) {
    sunrealtype* y_data = N_VGetArrayPointer(y);
    sunrealtype* F_data = N_VGetArrayPointer(F_Residual);

    sunrealtype Vp = (this->node1Index_ == 0) ? 0.0 : y_data[this->node1Index_ - 1];
    sunrealtype Vn = (this->node2Index_ == 0) ? 0.0 : y_data[this->node2Index_ - 1];

    sunrealtype I_vs = y_data[this->voltageSourceEquationIndex_];

    if(this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += I_vs;
    }

    if(this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= I_vs;
    }


    sunrealtype V = this->getVoltage(t, y);
    //std::cout << "[DEBUG][t=" << t << "] V(t,y) = " << V << "\n";

    F_data[this->voltageSourceEquationIndex_] = Vp - Vn - V;

}
