#ifndef INDUCTOR_H
#define INDUCTOR_H

#include "Element.h"

class Inductor : public Element {
private:
    double inductance_;
    int inductorEquationIndex_;

public:
    Inductor();

    Inductor(std::string name, std::string node1Name, std::string node2Name, double inductance) : Element(
        std::move(name), std::move(node1Name), std::move(node2Name)) , inductance_(inductance) {
    }

    ~Inductor() final = default;

    [[nodiscard]] std::string getType() const override { return "Inductor"; }

    void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) override;

    bool isLinear() override {
        return true;
    }

    [[nodiscard]] int getInductorIndex() const {
        return inductorEquationIndex_;
    }

    void setInductorEquationIndex(long int index) { inductorEquationIndex_ = index; }
};


#endif
