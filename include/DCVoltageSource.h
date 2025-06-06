#ifndef DCVOLTAGESOURCE_H
#define DCVOLTAGESOURCE_H
#include <AbstractVoltageSource.h>

class DCVoltageSource : public AbstractVoltageSource {
private:
    double DCVoltage_;
    double saveVoltage_;
    bool first = false;

public:
    DCVoltageSource(std::string name, std::string node1Name, std::string node2Name,
                    double voltage) : AbstractVoltageSource(std::move(name), std::move(node1Name), std::move(node2Name)),
                                      DCVoltage_(voltage) {
    }

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

    sunrealtype getVoltage() override {
        return DCVoltage_;
    }

    void setVoltage(double newVoltage) {
        if(!first) {
            saveVoltage_ = DCVoltage_;
            first = true;
        }
        DCVoltage_ = newVoltage;
    }
};
#endif //DCVOLTAGESOURCE_H
