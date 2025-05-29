#ifndef RESISTOR_H
#define RESISTOR_H

#include "Element.h"

class Resistor : public Element {
public:
    Resistor();

    Resistor(std::string name,std::string node1Name, std::string node2Name, int node1, int node2, double resistance) : Element(std::move(name), node1Name, node2Name, node1, node2),
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
