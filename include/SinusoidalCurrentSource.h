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

    void ResidualStamp() override;

    bool isLinear() override {
        return false;
    }

    std::string getType() const override {
        return "Sinusoidal Current Source";
    }
};

#endif //SINUSOIDALCURRENTSOURCE_H
