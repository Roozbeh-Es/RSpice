#ifndef SINUSIODALVOLTAGESOURCE_H
#define SINUSIODALVOLTAGESOURCE_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "AbstractVoltageSource.h"
#include <cmath>

class SinusoidalVoltageSource : public AbstractVoltageSource {
private:
    double offset_;
    double amplitude_;
    double frequency_;
    double timeDelay_;
    double dampingFactor_;
    double phase_;

public:
    SinusoidalVoltageSource();

    SinusoidalVoltageSource(const std::string &name,
                            const std::string &node1Name,
                            const std::string &node2Name,
                            double offset,
                            double amplitude,
                            double frequency,
                            double timeDelay,
                            double dampingFactor,
                            double phase)
        : AbstractVoltageSource(name, node1Name, node2Name),
          offset_(offset),
          amplitude_(amplitude),
          frequency_(frequency),
          timeDelay_(timeDelay),
          dampingFactor_(dampingFactor),
          phase_(phase) {
    }


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
