#ifndef CIRCUIT_H
#define CIRCUIT_H
#include "Element.h"
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <unordered_map>
#include <memory>

class Circuit {
public:
    Circuit();
    ~Circuit();
    void addElement(std::unique_ptr<Element> element);
    void deleteElement(std::unique_ptr<Element> element);
    void renameNode();
    void displayNodes();
    void buildMatrix();

private:
    std::vector<std::unique_ptr<Element>> elements;
    std::map<std::string, int> nodeNameToIndex;
    int numNodes_;
    int numVoltageSources_;
    double time_;
    double timeStep_;

};


#endif //CIRCUIT_H
