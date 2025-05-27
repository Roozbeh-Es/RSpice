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

enum class AnalysisType {
    DC,
    TRANSIENT,
    AC_SWEEP,
};

class Circuit {
public:
    Circuit();

    ~Circuit();

    void loadComponents(std::vector<std::unique_ptr<Element> > CircuitComponents, const std::map<std::string, int>& initialNodeMap );

    void buildMNAMatrix(Matrix& G, Matrix& B, Matrix& C, Matrix& D, Vector& J, Vector& E, double timeStep = 0.0);

private:
    std::vector<std::unique_ptr<Element> > elements;
    std::map<std::string, int> nodeNameToIndex;
    int numNodes_;
    int numVoltageSources_;
    double time_;
    double timeStep_;
    Vector currentNodeVoltages_;
    Vector CurrentSourceCurrents_;
};


#endif //CIRCUIT_H
