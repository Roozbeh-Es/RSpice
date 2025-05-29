#ifndef ABSTRACTVOLTAGESOURCE_H
#define ABSTRACTVOLTAGESOURCE_H

#include "Element.h"

class AbstractVoltageSource : public Element {
protected:
    int voltageSourceCurrentIndex_;
public:
    AbstractVoltageSource(std::string name, std::string node1Name, std::string node2Name, int node1,
                          int node2) : Element(std::move(name), std::move(node1Name), std::move(node2Name), node1,
                                               node2) {
    }

    virtual ~AbstractVoltageSource() override = default;

    std::string getType() const override {
        return "Abstract Voltage Source";
    }

    void stamp() override;
};


#endif //ABSTRACTVOLTAGESOURCE_H
