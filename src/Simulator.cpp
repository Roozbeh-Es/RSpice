#include "Simulator.h"
#include "TransientAnalysis.h"
// #include "DCOpAnalysis.h"        // Include these once you create them
// #include "DCSweepAnalysis.h"
#include <iostream>
#include <stdexcept>


bool Simulator::run() {
    std::cout << "Simulator: Processing netlist file: " << filePath_ << std::endl;

    try {

        if (!netListExtractor_.loadAndProcessNetList()) {
            std::cerr << "Simulator Error: Failed to load and process netlist." << std::endl;
            return false;
        }
        std::cout << "Simulator: Netlist processed successfully by Extractor." << std::endl;

        circuit_ = std::make_unique<Circuit>(
            netListExtractor_.getPreparedElements(),
            netListExtractor_.getSimulationParameters(),
            netListExtractor_.getNumEquations(),
            netListExtractor_.getNumNodes(),
            netListExtractor_.getNumVoltageSources(),
            netListExtractor_.getNumInductors(),
            netListExtractor_.getNodeMap()
        );
        std::cout << "Simulator: Circuit object created." << std::endl;

        const auto& simParams = circuit_->getSimulationParameters();

        if (simParams.isTransient()) {
            analysis_ = std::make_unique<TransientAnalysis>(simParams.transientParameters_);
            std::cout << "Simulator: Transient analysis strategy selected." << std::endl;
        } else if (simParams.isDCSweep()) {
            std::cerr << "Simulator Error: DC Sweep analysis is not yet implemented." << std::endl;
            return false;
        } else if (simParams.isOPPoint()) {
            // analysis_ = std::make_unique<DCOpAnalysis>();
            std::cerr << "Simulator Error: DC Operating Point analysis is not yet implemented." << std::endl;
            return false;
        } else {
            std::cerr << "Simulator Error: No valid analysis type specified in the netlist." << std::endl;
            return false;
        }

        // 4. Run the simulation.
        if (analysis_ && circuit_) {
            analysis_->solve(*circuit_); // Polymorphic call
            std::cout << "\nSimulator: Analysis completed." << std::endl;
        } else {
            std::cerr << "Simulator Error: Failed to create analysis strategy or circuit object." << std::endl;
            return false;
        }

    } catch (const std::exception& e) {
        std::cerr << "\nSimulator FATAL ERROR: Exception caught: " << e.what() << std::endl;
        return false;
    }
    return true;
}