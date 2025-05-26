#ifndef RESISTOR_H
#define RESISTOR_H

#include "Element.h"

class Resistor : public Element {
public:
    Resistor();

    Resistor(std::string name, int node1, int node2, double resistance) : Element(std::move(name), node1, node2),
                                                                          resistance_(resistance) {
    };

    [[nodiscard]] double getResistance() const { return resistance_; };

    [[nodiscard]] std::string getType() const override { return "Resistor"; }

    void stamp() override;

    ~Resistor() final;

private:
    double resistance_;
};

#endif //RESISTOR_H
