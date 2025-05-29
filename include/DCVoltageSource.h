#ifndef DCVOLTAGESOURCE_H
#define DCVOLTAGESOURCE_H
#include <AbstractVoltageSource.h>

class DCVoltageSource : AbstractVoltageSource {
private:
    double DCVoltage_;

public:
    DCVoltageSource();

    ~DCVoltageSource() override final = default;

    std::string getType() const override {
        return "DC Voltage Source";
    }

    void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) override;

    bool isLinear() override {
        return true;
    }
};
#endif //DCVOLTAGESOURCE_H
