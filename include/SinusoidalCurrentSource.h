#ifndef SINUSOIDALCURRENTSOURCE_H
#define SINUSOIDALCURRENTSOURCE_H

#include "AbstractCurrentSource.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class SinusoidalCurrentSource : public AbstractCurrentSource {
private:
    double offset_;
    double amplitude_;
    double frequency_;
    double timeDelay_;
    double dampingFactor_;
    double phase_;

public:
    SinusoidalCurrentSource();

    SinusoidalCurrentSource(std::string name, std::string node1Name, std::string node2Name, double offset,
                            double amplitude, double frequency, double timeDelay, double dampingFactor,
                            double phase) : AbstractCurrentSource(name, node1Name, node2Name), offset_(offset),
                                            amplitude_(amplitude), frequency_(frequency), timeDelay_(timeDelay),
                                            dampingFactor_(dampingFactor), phase_(phase) {
    };

    bool isLinear() override {
        return false;
    }

    [[nodiscard]] std::string getType() const override {
        return "Sinusoidal Current Source";
    }

    sunrealtype getCurrent(sunrealtype t, N_Vector y) const override {
        return amplitude_ * std::sin(2 * M_PI * frequency_ * (t - phase_));
    }
};

#endif //SINUSOIDALCURRENTSOURCE_H
