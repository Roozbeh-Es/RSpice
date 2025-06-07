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
#include <vector>

class Element {
public:
    Element() = default;

    Element(std::string name, std::string node1Name, std::string node2Name) : name_(std::move(name)),
                                                                              node1Name_(std::move(node1Name)),
                                                                              node2Name_(std::move(node2Name)) {
    }

    virtual ~Element() = default;

    [[nodiscard]] std::string getName() const { return name_; }
    [[nodiscard]] int getNode1() const { return node1Index_; }
    [[nodiscard]] int getNode2() const { return node2Index_; }

    [[nodiscard]] virtual std::string getType() const = 0;

    [[nodiscard]] virtual bool isLinear() { return true; }

    virtual void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) = 0;

    virtual void DCStamp(N_Vector y,N_Vector F) = 0;

    void setNode1Index(int node1Index) { node1Index_ = node1Index; }
    void setNode2Index(int node2Index) { node2Index_ = node2Index; }

    [[nodiscard]] std::vector<std::string> getNodeNames() const {
        return {node1Name_, node2Name_};
    }

    [[nodiscard]] std::string getNode1Name() const { return node1Name_; }
    [[nodiscard]] std::string getNode2Name() const { return node2Name_; }

protected:
    std::string name_;
    std::string node1Name_;
    std::string node2Name_;
    int node1Index_;
    int node2Index_;
};

#endif //ELEMENT_H