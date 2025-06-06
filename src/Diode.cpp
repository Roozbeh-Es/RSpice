#include "Diode.h"
#include "iostream"


void Diode::DCStamp(N_Vector y, N_Vector F) {
    sunrealtype* y_data = N_VGetArrayPointer(y);
    sunrealtype* F_data = N_VGetArrayPointer(F);

    sunrealtype V_anode   = (this->node1Index_ == 0) ? 0.0 : y_data[this->node1Index_ - 1];
    sunrealtype V_cathode = (this->node2Index_ == 0) ? 0.0 : y_data[this->node2Index_ - 1];
    double Vd = V_anode - V_cathode;

    // Numerical limiting to prevent floating point overflow from std::exp().
    // This is a standard practice in SPICE simulators.
    double Vd_limited = std::min(Vd, forwardVoltage_ + 0.2); // Limit voltage to slightly above V_fwd

    // Calculate diode current using the SHIFTED Shockley Diode Equation
    double Id = IS_ * (std::exp((Vd_limited - forwardVoltage_) / (N_ * VT_)) - 1.0);

    // Stamp current into KCL equations.
    // Current Id flows from anode (node 1) to cathode (node 2).
    // For KCL (sum of currents leaving = 0), current enters the anode and leaves the cathode.
    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] -= Id; // Current ENTERS the anode
    }
    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] += Id; // Current LEAVES the cathode
    }
}

void Diode::ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual)  {
    std::cout << "boop!" << std::endl;
}

