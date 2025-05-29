#ifndef SINUSIODALVOLTAGESOURCE_H
#define SINUSIODALVOLTAGESOURCE_H

#include "AbstractVoltageSource.h"

class SinusoidalVoltageSource : public AbstractVoltageSource {
private:
    double amplitude_;
    double frequency_;
    double offset_;

public:
    SinusoidalVoltageSource();
    SinusoidalVoltageSource(double amplitude, double frequency);

    void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) override;

    std::string getType() const override {
        return "Sinusoidal Voltage Source";
    }

    bool isLinear() override {
        return false;
    }
};

#endif //SINUSIODALVOLTAGESOURCE_H
