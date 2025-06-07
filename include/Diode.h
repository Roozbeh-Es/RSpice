#ifndef DIODE_H
#define DIODE_H

#include "Element.h"
#include <sunmatrix/sunmatrix_dense.h> // For SUNMatrix

class Diode : public Element {
public:
    Diode(const std::string &name, const std::string &anode, const std::string &cathode,
          double forwardVoltage, double IS = 1e-14, double N = 1.0) : Element(name, cathode, anode),
                                                                      forwardVoltage_(forwardVoltage), IS_(IS), N_(N) {
    }

    ~Diode() final = default;

    [[nodiscard]] std::string getType() const override { return "Diode"; }
    [[nodiscard]] bool isLinear() override { return false; }

    void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) override;

    void DCStamp(N_Vector y, N_Vector F) override;

private:
    const double forwardVoltage_;
    const double IS_; // Saturation Current
    const double N_; // Ideality Factor
    const double VT_ = 0.02585; // Thermal Voltage at room temperature (~26mV)
};

#endif // DIODE_H