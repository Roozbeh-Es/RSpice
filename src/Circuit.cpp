#include "Circuit.h"
#include <algorithm>
#include <iomanip>

Circuit::Circuit(
    std::vector<std::unique_ptr<Element> > circuitElements,
    SimulationParameters simParams,
    long int numEquations,
    long int numNonGroundNodes,
    long int numVoltageSources,
    long int numInductors,
    //long int numDiodes,
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


    if (yp_vec != nullptr) {
        N_VConst(0.0, yp_vec);
    }

    sunrealtype *y_data = N_VGetArrayPointer(y_vec);
    std::cout << "Circuit: Setting smart initial conditions..." << std::endl;

    for (const auto &el: elements_) {
        if (auto vs = dynamic_cast<const DCVoltageSource *>(el.get())) {
            int p_node_idx = vs->getNode1();
            int n_node_idx = vs->getNode2();
            double dc_value = vs->getVoltage(0, y_vec); // Using the simpler DC getter

            if (p_node_idx != 0 && n_node_idx == 0) {
                y_data[p_node_idx - 1] = dc_value;
            } else if (p_node_idx == 0 && n_node_idx != 0) {
                y_data[n_node_idx - 1] = -dc_value;
            }
        }
    }
    std::cout << "Circuit: Initial guess prepared." << std::endl;
}


std::vector<std::string> Circuit::getOrderedUnknownNames() const {
    std::vector<std::string> names;
    names.reserve(numEquations_);


    std::map<int, std::string> mnaIndexToNodeName;
    for (const auto &pair: nodeMap_) {
        if (pair.second != 0) {
            mnaIndexToNodeName[pair.second - 1] = pair.first; // map MNA KCL_row_idx to name
        }
    }
    for (int i = 0; i < numNonGroundNodes_; ++i) {
        auto it = mnaIndexToNodeName.find(i);
        if (it != mnaIndexToNodeName.end()) {
            names.push_back("V(" + it->second + ")");
        } else {
            names.push_back("V(unknown_node_" + std::to_string(i + 1) + ")");
        }
    }


    std::vector<const AbstractVoltageSource *> sortedVS;
    for (const auto &el: elements_) {
        if (auto vs = dynamic_cast<const AbstractVoltageSource *>(el.get())) {
            sortedVS.push_back(vs);
        }
    }
    std::sort(sortedVS.begin(), sortedVS.end(), [](const AbstractVoltageSource *a, const AbstractVoltageSource *b) {
        return a->getVoltageSourceCurrentIndex() < b->getVoltageSourceCurrentIndex();
    });
    for (const auto *vs: sortedVS) {
        names.push_back("I(" + vs->getName() + ")");
    }


    std::vector<const Inductor *> sortedL;
    for (const auto &el: elements_) {
        if (auto l = dynamic_cast<const Inductor *>(el.get())) {
            sortedL.push_back(l);
        }
    }
    std::sort(sortedL.begin(), sortedL.end(), [](const Inductor *a, const Inductor *b) {
        return a->getInductorIndex() < b->getInductorIndex(); // Assuming getInductorIndex gives MNA branch index
    });
    for (const auto *l: sortedL) {
        names.push_back("I(" + l->getName() + ")");
    }

    return names;
}

void Circuit::printTransientResults(sunrealtype t_current, N_Vector y_vec) {
    std::cout << t_current;
    sunrealtype *y_data = N_VGetArrayPointer(y_vec);
    std::vector<std::string> names = getOrderedUnknownNames();
    for (size_t i = 0; i < names.size() && i < static_cast<size_t>(numEquations_); ++i) {
        std::cout << "," << y_data[i];
    }
    std::cout << std::endl;
}

void Circuit::populateIdVector(N_Vector id) {
    N_VConst(0.0, id);
    sunrealtype *id_data = N_VGetArrayPointer(id);

    // 1) Inductor currents are differential states:
    for (const auto &el: elements_) {
        if (auto inductor = dynamic_cast<const Inductor *>(el.get())) {
            int idx = inductor->getInductorIndex();
            if (idx >= 0 && idx < N_VGetLength(id)) {
                id_data[idx] = 1.0;
            }
        }
    }

    // 2) Every node that has a capacitor attached becomes a differential variable:
    for (const auto &el: elements_) {
        if (auto cap = dynamic_cast<const Capacitor *>(el.get())) {
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


void Circuit::printDCResults(N_Vector y) const {
    std::cout << "\n--- DC Operating Point Results ---" << std::endl;

    // 1. Get the ordered list of unknown variable names.
    std::vector<std::string> unknownNames = this->getOrderedUnknownNames();

    // 2. Get the pointer to the final solution data.
    const sunrealtype *y_data = N_VGetArrayPointer(y);

    // 3. Loop through the results and print each one descriptively.
    for (long int i = 0; i < this->numEquations_; ++i) {
        // Get the name for the current variable (unknown)
        std::string name = (i < unknownNames.size()) ? unknownNames[i] : "UNKNOWN_" + std::to_string(i);

        // Get the calculated value
        sunrealtype value = y_data[i];

        // Determine the unit based on the variable name
        std::string unit = " ";
        if (!name.empty()) {
            if (name[0] == 'V') {
                unit = " V";
            } else if (name[0] == 'I') {
                unit = " A";
            }
        }

        // Print the formatted line
        std::cout << "  " << std::left << std::setw(15) << name
                << " = " << std::right << std::setw(12) << std::scientific << value
                << unit << std::endl;
    }
    std::cout << "------------------------------------" << std::endl;
}
