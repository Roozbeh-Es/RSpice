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
};
#endif //DCVOLTAGESOURCE_H
