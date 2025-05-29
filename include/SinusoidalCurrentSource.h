#ifndef SINUSOIDALCURRENTSOURCE_H
#define SINUSOIDALCURRENTSOURCE_H

#include "AbstractCurrentSource.h"

class SinusoidalCurrentSource : public AbstractCurrentSource {
private:
    double amplitude_;
    double frequency_;
    double offset_;

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
};

#endif //SINUSOIDALCURRENTSOURCE_H
