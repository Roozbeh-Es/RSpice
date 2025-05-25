#ifndef ELEMENT_H
#define ELEMENT_H
#include <string>

class Element {
public:
    Element() = default;

    Element(std::string name, int node1, int node2) : name_(std::move(name)), node1_(node1), node2_(node2) {
    }

    virtual ~Element() = default;

    [[nodiscard]] std::string getName() const { return name_; }
    [[nodiscard]] int getNode1() const { return node1_; }
    [[nodiscard]] int getNode2() const { return node2_; }

    [[nodiscard]] virtual std::string getType() const { return "Element"; }

    virtual void stamp();

private:
    std::string name_;
    int node1_;
    int node2_;
};

#endif //ELEMENT_H
