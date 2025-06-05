#include "Simulator.h"
#include "TransientAnalysis.h"
// #include "DCOpAnalysis.h"        // Include these once you create them
// #include "DCSweepAnalysis.h"
#include <iostream>
#include <stdexcept>

// The constructor is defined in your Simulator.h file. It correctly
// initializes filePath_ and netListExtractor_.

// A default destructor is sufficient as unique_ptr members will handle cleanup.
// Simulator::~Simulator() {}

// The run() method uses the filePath_ stored in the Simulator object.
bool Simulator::run() {
    std::cout << "Simulator: Processing netlist file: " << filePath_ << std::endl;

    try {
        // 1. Use the extractor to parse the file.
        //    The call now correctly takes no arguments.
        if (!netListExtractor_.loadAndProcessNetList()) {
            std::cerr << "Simulator Error: Failed to load and process netlist." << std::endl;
            return false;
        }
        std::cout << "Simulator: Netlist processed successfully by Extractor." << std::endl;

        // 2. Create the Circuit object using the data from the now-processed extractor.
        circuit_ = std::make_unique<Circuit>(
            netListExtractor_.getPreparedElements(),       // Moves ownership of elements
            netListExtractor_.getSimulationParameters(),
            netListExtractor_.getNumEquations(),
            netListExtractor_.getNumNodes(),               // This is the count of non-ground nodes
            netListExtractor_.getNumVoltageSources(),
            netListExtractor_.getNumInductors(),
            netListExtractor_.getNodeMap()
        );
        std::cout << "Simulator: Circuit object created." << std::endl;

        // 3. Determine and create the correct analysis strategy.
        const auto& simParams = circuit_->getSimulationParameters();

        if (simParams.isTransient()) {
            analysis_ = std::make_unique<TransientAnalysis>(simParams.transientParameters_);
            std::cout << "Simulator: Transient analysis strategy selected." << std::endl;
        } else if (simParams.isDCSweep()) {
            // analysis_ = std::make_unique<DCSweepAnalysis>(simParams.DCSweepParameters_);
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