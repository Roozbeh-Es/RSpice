#include "Diode.h"
#include "iostream"

constexpr double IS_ = 1e-14;
constexpr double N_ = 1.0;
constexpr double VT_ = 0.02585;

void Diode::DCStamp(N_Vector y, N_Vector F) {
    sunrealtype *y_data = N_VGetArrayPointer(y);
    sunrealtype *F_data = N_VGetArrayPointer(F);

    sunrealtype Vp = (node1Index_ == 0) ? 0.0 : y_data[node1Index_ - 1];
    sunrealtype Vn = (node2Index_ == 0) ? 0.0 : y_data[node2Index_ - 1];
    sunrealtype Vd = Vp - Vn;

    const sunrealtype V_limit = 100.0;
    if (Vd > V_limit) Vd = V_limit;
    if (Vd < -V_limit) Vd = -V_limit;

    sunrealtype exp_term = std::exp(Vd / (N_ * VT_));
    sunrealtype I = IS_ * (exp_term - 1.0);
    sunrealtype G = (IS_ / (N_ * VT_)) * exp_term;
    sunrealtype Ieq = I - G * Vd;

    if (node1Index_ != 0) {
        F_data[node1Index_ - 1] += Ieq;
    }
    if (node2Index_ != 0) {
        F_data[node2Index_ - 1] -= Ieq;
    }
}



void Diode::ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) {
    sunrealtype *y_data = N_VGetArrayPointer(y);
    sunrealtype *F_data = N_VGetArrayPointer(F_Residual);

    sunrealtype Vp = (node1Index_ == 0) ? 0.0 : y_data[node1Index_ - 1];
    sunrealtype Vn = (node2Index_ == 0) ? 0.0 : y_data[node2Index_ - 1];
    sunrealtype Vd = Vp - Vn;

    const sunrealtype V_limit = 100.0;
    if (Vd > V_limit) Vd = V_limit;
    if (Vd < -V_limit) Vd = -V_limit;

    sunrealtype exp_term = std::exp(Vd / (N_ * VT_));
    sunrealtype I = IS_ * (exp_term - 1.0);
    sunrealtype G = (IS_ / (N_ * VT_)) * exp_term;
    sunrealtype Ieq = I - G * Vd;

    if (node1Index_ != 0) {
        F_data[node1Index_ - 1] += Ieq;
    }
    if (node2Index_ != 0) {
        F_data[node2Index_ - 1] -= Ieq;
    }}