#ifndef ABSTRACTVOLTAGESOURCE_H
#define ABSTRACTVOLTAGESOURCE_H

#include "Element.h"

class AbstractVoltageSource : public Element {
protected:
    int voltageSourceEquationIndex_;

public:
    AbstractVoltageSource(std::string name, std::string node1Name, std::string node2Name) : Element(
        std::move(name), std::move(node1Name), std::move(node2Name)) {
    }

    ~AbstractVoltageSource() override = default;

    [[nodiscard]] std::string getType() const override {
        return "Abstract Voltage Source";
    }

    void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) override;

    void DCStamp(N_Vector y, N_Vector F) override;


    [[nodiscard]] int getVoltageSourceCurrentIndex() const {
        return voltageSourceEquationIndex_;
    }

    //y is for dependencies for the dependent sources that we will add later on
    virtual sunrealtype getVoltage(sunrealtype t, N_Vector y) const = 0;

    virtual sunrealtype getVoltage(N_Vector y) = 0;

    void setVoltageSourceEquationIndex(long int index) {
        voltageSourceEquationIndex_ = index;
    }
};


#endif //ABSTRACTVOLTAGESOURCE_H
