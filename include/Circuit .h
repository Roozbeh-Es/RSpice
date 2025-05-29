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
#include "SinusiodalVoltageSource.h"
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

enum class AnalysisType {
    DC,
    TRANSIENT,
    AC_SWEEP,
};

class Circuit {
public:
    Circuit();

    ~Circuit();

    void loadComponents(std::vector<std::unique_ptr<Element> > CircuitComponents, const std::map<std::string, int>& initialNodeMap);

    void prepareForSimulation();


private:
    std::vector<std::unique_ptr<Element> > elements_;
    std::map<std::string, int> nodeNameToIndex_;
    int numNodes_;
    int numVoltageSources_;
    double time_;
    double timeStep_;
    Vector currentNodeVoltages_;
    Vector CurrentSourceCurrents_;
};


#endif //CIRCUIT_H
