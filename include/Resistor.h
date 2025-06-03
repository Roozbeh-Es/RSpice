#ifndef RESISTOR_H
#define RESISTOR_H

#include "Element.h"

class Resistor : public Element {
public:
    Resistor();

    Resistor(std::string name, std::string node1Name, std::string node2Name, double resistance) : Element(
            std::move(name), std::move(node1Name), std::move(node2Name)),
        resistance_(resistance) {
    };

    [[nodiscard]] double getResistance() const { return resistance_; };

    [[nodiscard]] std::string getType() const override { return "Resistor"; }

    void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) override;

    ~Resistor() final = default;

    bool isLinear() override {
        return true;
    }

private:
    double resistance_;
};

#endif //RESISTOR_H
