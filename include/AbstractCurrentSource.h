#ifndef ABSTRACTCURRENTSOURCE_H
#define ABSTRACTCURRENTSOURCE_H

#include "Element.h"

class AbstractCurrentSource : public Element {
public:
    AbstractCurrentSource(std::string name, std::string node1Name, std::string node2Name) : Element(
        std::move(name), std::move(node1Name), std::move(node2Name)) {
    }

    [[nodiscard]] std::string getType() const override {
        return "Abstract Current Source";
    }

    ~AbstractCurrentSource() override = default;

    void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) override;

    virtual sunrealtype getCurrent(sunrealtype t, N_Vector y) const = 0;
};

#endif //ABSTRACTCURRENTSOURCE_H
