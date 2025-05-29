#ifndef ELEMENT_H
#define ELEMENT_H
#include <iostream>
#include <cmath>
#include <string>
#include <ida/ida.h>
#include <nvector/nvector_serial.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sundials/sundials_types.h>
#include <sundials/sundials_context.h>

class Element {
public:
    Element() = default;

    Element(std::string name, std::string node1Name, std::string node2Name, int node1,
            int node2) : name_(std::move(name)), node1Name_(std::move(node1Name)), node2Name_(std::move(node2Name)),
                         node1Index_(node1),
                         node2Index_(node2) {
    }

    virtual ~Element() = default;

    [[nodiscard]] std::string getName() const { return name_; }
    [[nodiscard]] int getNode1() const { return node1Index_; }
    [[nodiscard]] int getNode2() const { return node2Index_; }

    [[nodiscard]] virtual std::string getType() const = 0;

    [[nodiscard]] virtual bool isLinear() { return true; }

    virtual void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) = 0;

protected:
    std::string name_;
    std::string node1Name_;
    std::string node2Name_;
    int node1Index_;
    int node2Index_;
};

#endif //ELEMENT_H
