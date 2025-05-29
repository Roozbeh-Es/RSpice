#ifndef CAPACITOR_H
#define CAPACITOR_H
#include "Element.h"

class Capacitor : public Element {
public:
    Capacitor();

    Capacitor(std::string name, std::string node1Name, std::string node2Name, int node1,
            int node2, double capacitance) : Element(std::move(name), std::move(node1Name), std::move(node2Name), node1, node2), capacitance_(capacitance) {}

    ~Capacitor() final;

    void ResidualStamp() override;

    bool isLinear() override {
        return true;
    }

private:
    double capacitance_;
};


#endif //CAPACITOR_H
