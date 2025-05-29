#ifndef INDUCTOR_H
#define INDUCTOR_H

#include "Element.h"

class Inductor : public Element {
private:
    double inductance_;
    int InductorCurrentIndex_;

public:
    Inductor();

    Inductor(std::string name, std::string node1Name, std::string node2Name, int node1,
             int node2, double inductance) : Element(std::move(name), std::move(node1Name), std::move(node2Name), node1,
                                                     node2), inductance_(inductance) {
    }

    ~Inductor() final = default;

    [[nodiscard]] std::string getType() const override { return "Inductor"; }

    void stamp() override;

    bool isLinear() override {
        return true;
    }
};


#endif
