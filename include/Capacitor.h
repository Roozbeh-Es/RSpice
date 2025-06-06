#ifndef CAPACITOR_H
#define CAPACITOR_H
#include "Element.h"

class Capacitor : public Element {
public:
    Capacitor();

    Capacitor(std::string name, std::string node1Name, std::string node2Name,
              double capacitance) : Element(std::move(name), std::move(node1Name), std::move(node2Name)),
                                    capacitance_(capacitance) {
    }

    ~Capacitor() final = default;

    void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) override;

    void DCStamp(N_Vector y, N_Vector F) override;


    bool isLinear() override {
        return true;
    }

    std::string getType() const override {
        return "Capacitor";
    }

private:
    double capacitance_;
};


#endif //CAPACITOR_H
