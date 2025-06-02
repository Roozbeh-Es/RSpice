#ifndef DCVOLTAGESOURCE_H
#define DCVOLTAGESOURCE_H
#include <AbstractVoltageSource.h>

class DCVoltageSource : public AbstractVoltageSource {
private:
    double DCVoltage_;

public:
    DCVoltageSource();

    ~DCVoltageSource() override final = default;

    std::string getType() const override {
        return "DC Voltage Source";
    }


    bool isLinear() override {
        return true;
    }

    sunrealtype getVoltage(sunrealtype t, N_Vector y) const override {
        return DCVoltage_;
    }
};
#endif //DCVOLTAGESOURCE_H
