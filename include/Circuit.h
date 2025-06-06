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
#include  "NetListExtractor.h"
#include "SimulationParameters.h"
#include <ida/ida.h>
#include <nvector/nvector_serial.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <sunlinsol/sunlinsol_dense.h>
#include <sundials/sundials_types.h>
#include <sundials/sundials_context.h>

class Circuit {
public:
    Circuit(std::vector<std::unique_ptr<Element> > circuitElements,
            SimulationParameters simParams,
            long int numEquations,
            long int numNonGroundNodes,
            long int numVoltageSourceBranches,
            long int numInductorBranches,
            const std::map<std::string, int> &nodeMap
    );

    ~Circuit();


    [[nodiscard ]] int getNumNodes() const {
        return numNonGroundNodes_;
    }

    [[nodiscard]] int getNumVoltageSources() const {
        return numVoltageSources_;
    }

    [[nodiscard]] int getNumInductors() const {
        return numInductors_;
    }

    std::vector<std::unique_ptr<Element> > &getElements() {
        return elements_;
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Element> > &getElements() const {
        return elements_;
    }

    [[nodiscard]] SUNContext getSUNContext() const {
        return suncntx_;
    }

    [[nodiscard]] long int getNumEquations() const {
        return numEquations_;
    }

    void getInitialConditions(N_Vector y_vec, N_Vector yp_vec);

    void printTransientResults(sunrealtype t_current, N_Vector y_vec);

    [[nodiscard]] std::vector<std::string> getOrderedUnknownNames() const;

    [[nodiscard]] const SimulationParameters& getSimulationParameters() const {
        return simulationParameters_;
    }

    void populateIdVector(N_Vector id);


    void printDCResults(N_Vector y) const;

private:
    std::vector<std::unique_ptr<Element> > elements_;
    SimulationParameters simulationParameters_;
    //std::map<std::string, int> nodeNameToIndex_;
    long int numNonGroundNodes_;
    long int numVoltageSources_;
    long int numInductors_;
    long int numEquations_;
    const std::map<std::string, int> nodeMap_;
    SUNContext suncntx_ = nullptr;
};


#endif //CIRCUIT_H
