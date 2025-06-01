#ifndef SINUSOIDALCURRENTSOURCE_H
#define SINUSOIDALCURRENTSOURCE_H

#include "AbstractCurrentSource.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class SinusoidalCurrentSource : public AbstractCurrentSource {
private:
    double amplitude_;
    double frequency_;
    double offset_;
    double phase_;

public:
    SinusoidalCurrentSource();

    SinusoidalCurrentSource(double amplitude, double frequency, double offset);

    void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) override;

    bool isLinear() override {
        return false;
    }

    [[nodiscard]] std::string getType() const override {
        return "Sinusoidal Current Source";
    }

    sunrealtype getCurrent(sunrealtype t, N_Vector y) override {
        return amplitude_ * std::sin(2 * M_PI * frequency_ * (t-phase_));
    }
};

#endif //SINUSOIDALCURRENTSOURCE_H
