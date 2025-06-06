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

// In Circuit.cpp

void Circuit::getInitialConditions(N_Vector y_vec, N_Vector yp_vec) {
    // Start by setting everything to zero. This is a good default.
    N_VConst(0.0, y_vec);
    N_VConst(0.0, yp_vec);

    sunrealtype* y_data = N_VGetArrayPointer(y_vec);

    std::cout << "Circuit: Setting smart initial conditions..." << std::endl;

    // Iterate through all elements to find known initial values.
    for (const auto& el : elements_) {
        // Use dynamic_cast to check if the element is a DCVoltageSource
        if (auto vs = dynamic_cast<const DCVoltageSource*>(el.get())) {

            int p_node_idx = vs->getNode1(); // Positive node index
            int n_node_idx = vs->getNode2(); // Negative node index
            double dc_value = vs->getVoltage(0,y_vec)
            ;

            std::cout << "[IC] Found DC Voltage Source " << vs->getName()
                      << " with value " << dc_value
                      << " between nodes " << p_node_idx << " and " << n_node_idx << std::endl;

            // If the source is connected between a node and ground, we can set
            // the initial guess for that node's voltage directly.
            if (p_node_idx != 0 && n_node_idx == 0) {
                // Positive node is not ground, negative node is ground.
                // Set the initial guess for the positive node's voltage.
                y_data[p_node_idx - 1] = dc_value;
                std::cout << "[IC]   Setting initial voltage for node " << p_node_idx
                          << " (MNA index " << p_node_idx - 1 << ") to " << dc_value << std::endl;
            } else if (p_node_idx == 0 && n_node_idx != 0) {
                // Positive node is ground, negative node is not ground.
                // Set the initial guess for the negative node's voltage.
                y_data[n_node_idx - 1] = -dc_value;
                std::cout << "[IC]   Setting initial voltage for node " << n_node_idx
                          << " (MNA index " << n_node_idx - 1 << ") to " << -dc_value << std::endl;
            }
        }
    }

    std::cout << "Circuit: Smart initial conditions set." << std::endl;
}




std::vector<std::string> Circuit::getOrderedUnknownNames() const {
    std::vector<std::string> names;
    names.reserve(numEquations_);

    // 1. Non-ground node voltages
    // We need a reverse map from MNA index (0 to K-1) to node name
    // Node indices are 1 to K. MNA indices are 0 to K-1.
    std::map<int, std::string> mnaIndexToNodeName;
    for (const auto& pair : nodeMap_) {
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

void Circuit::populateIdVector(N_Vector id) {
    // Start by assuming all variables are algebraic (id = 0.0)
    N_VConst(0.0, id);
    sunrealtype* id_data = N_VGetArrayPointer(id);

    // Iterate through elements to find ALL differential variables
    for (const auto& el : elements_) {
        // --- FIX: Check for both Capacitors and Inductors ---

        // The current through an Inductor is a differential variable.
        if (auto inductor = dynamic_cast<const Inductor*>(el.get())) {
            id_data[inductor->getInductorIndex()] = 1.0;
        }
        // The voltages at nodes connected to a Capacitor are differential variables.
        else if (auto capacitor = dynamic_cast<const Capacitor*>(el.get())) {
            if (capacitor->getNode1() != 0) { // getNode1() returns the integer index
                id_data[capacitor->getNode1() - 1] = 1.0;
            }
            if (capacitor->getNode2() != 0) {
                id_data[capacitor->getNode2() - 1] = 1.0;
            }
        }
    }
    std::cout << "Circuit: ID vector populated for DAE solver." << std::endl;
}


