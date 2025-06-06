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

// In Circuit.cpp

#include "DCVoltageSource.h" // Ensure this is included at the top

void Circuit::getInitialConditions(N_Vector y_vec, N_Vector yp_vec) {
    // This function provides an *initial guess* for the IDACalcIC function.
    // IDACalcIC will then solve for the true, consistent initial conditions.

    // Start by setting everything to zero. This is a safe default guess for all scenarios.
    N_VConst(0.0, y_vec);
    N_VConst(0.0, yp_vec);

    sunrealtype* y_data = N_VGetArrayPointer(y_vec);

    // Check if the user wants to use explicitly specified Initial Conditions (UIC).
    // The 'UIC_' flag is parsed from the .TRAN line.
    if (this->simulationParameters_.transientParameters_.UIC_) {
        // --- This path is taken if the netlist contains ".TRAN ... UIC" ---
        std::cout << "Circuit: Applying user-specified initial conditions (UIC)." << std::endl;

        // This is where you would apply any user-defined initial conditions
        // parsed from ".IC" commands or "IC=" parameters on component lines.
        // Since the parser doesn't support that yet, for now this block does
        // nothing, and the simulation starts from y=0, yp=0 as specified by UIC.
        // For example:
        // for (const auto& el : elements_) {
        //     if (auto cap = dynamic_cast<const Capacitor*>(el.get())) {
        //         if (cap->hasInitialVoltage()) { // Assuming such methods exist
        //             y_data[cap->getNode1() - 1] = cap->getInitialVoltage();
        //         }
        //     }
        // }

    } else {
        // --- This is the default path: Provide a smart guess for the DC Operating Point ---
        std::cout << "Circuit: Setting smart initial guess for DC Operating Point calculation." << std::endl;

        // Iterate through all elements to find known DC values to improve the guess.
        for (const auto& el : elements_) {
            // We set known voltages from DC voltage sources.
            if (auto vs = dynamic_cast<const DCVoltageSource*>(el.get())) {

                int p_node_idx = vs->getNode1(); // Positive node index
                int n_node_idx = vs->getNode2(); // Negative node index
                double dc_value = vs->getVoltage(0,y_vec); // Use the new, clean getter

                // If the source is connected between a node and ground, we can set
                // the initial guess for that node's voltage directly.
                if (p_node_idx != 0 && n_node_idx == 0) {
                    // Positive node is not ground, negative node is ground.
                    y_data[p_node_idx - 1] = dc_value;
                } else if (p_node_idx == 0 && n_node_idx != 0) {
                    // Positive node is ground, negative node is not ground.
                    y_data[n_node_idx - 1] = -dc_value;
                }
                // Note: If the source is between two non-ground nodes, we don't set a guess,
                // as we only know the voltage *difference*, not the absolute voltages.
            }
        }
    }
    std::cout << "Circuit: Initial guess prepared." << std::endl;
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

void Circuit::populateIdVector(N_Vector id)
{
    // Start by assuming everything is algebraic (0.0)
    N_VConst(0.0, id);
    sunrealtype* id_data = N_VGetArrayPointer(id);

    // 1) Inductor currents are differential states:
    for (const auto& el : elements_) {
        if (auto inductor = dynamic_cast<const Inductor*>(el.get())) {
            int idx = inductor->getInductorIndex();
            if (idx >= 0 && idx < N_VGetLength(id)) {
                id_data[idx] = 1.0;
            }
        }
    }

    // 2) Every node that has a capacitor attached becomes a differential variable:
    for (const auto& el : elements_) {
        if (auto cap = dynamic_cast<const Capacitor*>(el.get())) {
            int n1 = cap->getNode1();
            int n2 = cap->getNode2();
            // Mark each non‐ground node connected to this C as differential (1.0):
            if (n1 != 0) {
                id_data[n1 - 1] = 1.0;
            }
            if (n2 != 0) {
                id_data[n2 - 1] = 1.0;
            }
        }
    }

    std::cout << "Circuit: ID vector populated for DAE solver.\n";
}

