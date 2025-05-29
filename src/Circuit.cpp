#include "Circuit .h"


//in this function we're going to  give size to our equations and variables and also index them
void Circuit::prepareForSimulation() {
    int nodeCounter = nodeNameToIndex_.size() - 1;;
    int numOfVoltageSources = 0;
    int numOfInductors = 0;

    for (auto& element : elements_) {
        if (dynamic_cast<AbstractVoltageSource*>(element.get())) {
            numOfVoltageSources++;
        } else if (dynamic_cast<Inductor*>(element.get())) {
            numOfInductors++;
        }
    }
}
