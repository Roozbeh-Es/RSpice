#include "Circuit.h"
#include <algorithm>

Circuit::Circuit(
    std::vector<std::unique_ptr<Element> > circuitElements,
    SimulationParameters simParams,
    long int numEquations,
    long int numNonGroundNodes,
    long int numVoltageSources,
    long int numInductors,
    const std::map<std::string, int> &nodeMap
) : elements_(std::move(circuitElements)),
    simulationParameters_(simParams),
    nodeMap_(nodeMap),
    numEquations_(numEquations),
    numNonGroundNodes_(numNonGroundNodes),
    numVoltageSources_(numVoltageSources),
    numInductors_(numInductors) {
    int flag = SUNContext_Create(SUN_COMM_NULL, &suncntx_);
    if (flag != 0) {
        suncntx_ = nullptr;
        throw std::runtime_error("Failed to create SUNDIALS SUNContext.");
    }

    std::cout << "Circuit object constructed and initialized." << std::endl;
    std::cout << "  Total MNA System Size: " << numEquations_ << std::endl;
    std::cout << "  Non-Ground Nodes (KCL equations): " << numNonGroundNodes_ << std::endl;
    std::cout << "  Voltage Source Branches: " << numVoltageSources_ << std::endl;
    std::cout << "  Inductor Branches: " << numInductors_ << std::endl;
}

Circuit::~Circuit() {
    if (suncntx_ != nullptr) {
        SUNContext_Free(&suncntx_);
        suncntx_ = nullptr;
    }
    std::cout << "Circuit object destroyed, SUNDIALS context freed" << std::endl;
}

void Circuit::getInitialConditions(N_Vector y_vec, N_Vector yp_vec) {
    N_VConst(0.0, y_vec);
    N_VConst(0.0, yp_vec);
    std::cout << "Circuit: Initial conditions set to zero for y and yp." << std::endl;
}


std::vector<std::string> Circuit::getOrderedUnknownNames() const {
    std::vector<std::string> names;
    names.reserve(numEquations_);

    // 1. Non-ground node voltages
    // We need a reverse map from MNA index (0 to K-1) to node name
    // Node indices are 1 to K. MNA indices are 0 to K-1.
    std::map<int, std::string> mnaIndexToNodeName;
    for (const auto& pair : numEquations_) {
        if (pair.second != 0) { // Not ground
            mnaIndexToNodeName[pair.second - 1] = pair.first; // map MNA KCL_row_idx to name
        }
    }
    for (int i = 0; i < numNonGroundNodes_; ++i) {
        auto it = mnaIndexToNodeName.find(i);
        if (it != mnaIndexToNodeName.end()) {
            names.push_back("V(" + it->second + ")");
        } else {
            names.push_back("V(unknown_node_" + std::to_string(i+1) + ")");
        }
    }

    // 2. Currents through voltage sources
    // Need to iterate elements and find voltage sources in their MNA index order
    std::vector<const AbstractVoltageSource*> sortedVS;
    for(const auto& el : elements_){
        if(auto vs = dynamic_cast<const AbstractVoltageSource*>(el.get())){
            sortedVS.push_back(vs);
        }
    }
    std::sort(sortedVS.begin(), sortedVS.end(), [](const AbstractVoltageSource* a, const AbstractVoltageSource* b){
        return a->getVoltageSourceCurrentIndex() < b->getVoltageSourceCurrentIndex();
    });
    for(const auto* vs : sortedVS){
        names.push_back("I(" + vs->getName() + ")");
    }


    // 3. Currents through inductors
    std::vector<const Inductor*> sortedL;
     for(const auto& el : elements_){
        if(auto l = dynamic_cast<const Inductor*>(el.get())){
            sortedL.push_back(l);
        }
    }
    std::sort(sortedL.begin(), sortedL.end(), [](const Inductor* a, const Inductor* b){
        return a->getInductorIndex() < b->getInductorIndex(); // Assuming getInductorIndex gives MNA branch index
    });
    for(const auto* l : sortedL){
        names.push_back("I(" + l->getName() + ")");
    }

    return names;
}

void Circuit::printTransientResults(sunrealtype t_current, N_Vector y_vec) {
    // This should use getOrderedUnknownNames() to print headers and then values
    // For now, a simple print:
    std::cout << t_current;
    sunrealtype* y_data = N_VGetArrayPointer(y_vec);
    std::vector<std::string> names = getOrderedUnknownNames();
    for (size_t i = 0; i < names.size() && i < static_cast<size_t>(numEquations_); ++i) {
        std::cout << "," << y_data[i];
    }
    std::cout << std::endl;
}

