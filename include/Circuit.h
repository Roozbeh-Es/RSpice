#ifndef CIRCUIT_H
#define CIRCUIT_H
#include "Element.h"
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <unordered_map>
#include <memory>
#include "Vector.h"
#include "Matrix.h"
#include "Resistor.h"
#include "Capacitor.h"
#include "Inductor.h"
#include "AbstractCurrentSource.h"
#include "AbstractVoltageSource.h"
#include "SinusoidalVoltageSource.h"
#include "SinusoidalCurrentSource.h"
#include "DCCurrentSource.h"
#include "DCVoltageSource.h"
#include <iostream>
#include <cmath>
#include <string>
#include <ida/ida.h>
#include <nvector/nvector_serial.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sundials/sundials_types.h>
#include <sundials/sundials_context.h>

class Circuit {
public:
    Circuit();

    ~Circuit();


    [[nodiscard ]]int getNumNodes() const {
        return numNodes_;
    }

    [[nodiscard]]int getNumVoltageSources() const {
        return  numVoltageSources_;
    }

    [[nodiscard]]int getNumInductors() const {
        return numInductors_;
    }

     std::vector<std::unique_ptr<Element>>& getElements()  {
        return elements_;
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Element>>& getElements() const {
        return elements_;
    }

private:
    std::vector<std::unique_ptr<Element> > elements_;
    std::map<std::string, int> nodeNameToIndex_;
    int numNodes_;
    int numVoltageSources_;
    int numInductors_;
    int numEquations_;
    SUNContext suncntx_;
};


#endif //CIRCUIT_H
