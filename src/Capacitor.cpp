#include "Capacitor.h"

#include "Capacitor.h"

// A tiny conductance in parallel with every capacitor to keep the DAE index = 1
// (i.e. avoid pure LC floating nodes).
// Typical choice: GMIN = 1e–9 S (1 GΩ)
static constexpr sunrealtype GMIN = 1e-9;

void Capacitor::ResidualStamp(sunrealtype t,
                              N_Vector y,
                              N_Vector yp,
                              N_Vector F_Residual)
{
    // Obtain raw pointers to y, yp, and residual arrays
    sunrealtype* y_data  = N_VGetArrayPointer(y);
    sunrealtype* yp_data = N_VGetArrayPointer(yp);
    sunrealtype* F_data  = N_VGetArrayPointer(F_Residual);

    // Node voltages (algebraic index = nodeIndex − 1), but we treat them as differential states:
    // y[nodeIdx – 1] = V_node,   yp[nodeIdx – 1] = dV_node/dt
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

    // Capacitive current: I_C = C * (dVp - dVn)
    sunrealtype iC = this->capacitance_ * (dVp - dVn);

    // GMIN‐parallel current: I_G = GMIN * (Vp - Vn)
    sunrealtype iG = GMIN * (Vp - Vn);

    // Total branch current from C∥GMIN
    sunrealtype iTotal = iC + iG;

    // Stamp into KCL at node1 and node2 (only if not ground)
    if (this->node1Index_ != 0) {
        F_data[this->node1Index_ - 1] += iTotal;
    }
    if (this->node2Index_ != 0) {
        F_data[this->node2Index_ - 1] -= iTotal;
    }

    // No additional row is needed—each capacitor simply injects iC + iG into its two node KCLs.
    // Because we declared those node voltages as differential (in the ID vector), IDA
    // understands “yp[node]” as a free variable, and the equation F = 0 becomes:
    //    C*(dVp - dVn) + GMIN*(Vp - Vn) + (sum of other currents from other elements) = 0.
    // That is a perfectly index‐1 DAE for nodes that have a capacitor attached.
}

