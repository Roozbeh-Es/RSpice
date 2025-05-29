#ifndef SINUSIODALVOLTAGESOURCE_H
#define SINUSIODALVOLTAGESOURCE_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "AbstractVoltageSource.h"
#include <cmath>

class SinusoidalVoltageSource : public AbstractVoltageSource {
private:
    double amplitude_;
    double frequency_;
    double offset_;
    double phase_;

public:
    SinusoidalVoltageSource();

    SinusoidalVoltageSource(const std::string& name,
                           const std::string& node1Name,
                           const std::string& node2Name,
                           int node1Index,
                           int node2Index,
                           double amplitude,
                           double frequency,
                           double offset,
                           double phase)
       : AbstractVoltageSource(name, node1Name, node2Name, node1Index, node2Index),
         amplitude_(amplitude),
         frequency_(frequency),
         offset_(offset),
         phase_(phase) {}

    void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) override;

    std::string getType() const override {
        return "Sinusoidal Voltage Source";
    }

    bool isLinear() override {
        return false;
    }

    sunrealtype getVoltage(sunrealtype t, N_Vector y) const override {
        return offset_ + amplitude_ * std::cos(2 * M_PI * frequency_ * (t - phase_));
    }
};

#endif //SINUSIODALVOLTAGESOURCE_H
