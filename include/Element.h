#ifndef ELEMENT_H
#define ELEMENT_H
#include <string>

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

    virtual void stamp();

protected:
    std::string name_;
    std::string node1Name_;
    std::string node2Name_;
    int node1Index_;
    int node2Index_;
};

#endif //ELEMENT_H
