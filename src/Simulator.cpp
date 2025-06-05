#include "Simulator.h"
#include "iostream"
#include "Analysis.h"
#include "TransientAnalysis.h"
#include <iostream>



bool Simulator::runFile(const std::string &netlistFilePath) {
    std::cout << "Simulator: Processing netlist file: " << netlistFilePath << std::endl;
    // NetListExtractor extractor; // Create here if it doesn't need to persist across runs in Simulator

    try {
        if (!netListExtractor_.loadAndProcessNetList()) {
            // Extractor must call performSizingAndIndexing
            std::cerr << "Simulator: Failed to load and process netlist." << std::endl;
            return false;
        }
        std::cout << "Simulator: Netlist processed successfully." << std::endl;

        // Create Circuit object
        circuit_ = std::make_unique<Circuit>(
            netListExtractor_.getPreparedElements(),
            netListExtractor_.getSimulationParameters(),
            netListExtractor_.getNumEquations(),
            netListExtractor_.getNumNodes(), // Count of non-GND nodes
            netListExtractor_.getNumVoltageSources(),
            netListExtractor_.getNumInductors(),
            netListExtractor_.getNodeMap()
        );
        std::cout << "Simulator: Circuit object created." << std::endl;

        // Determine and create the analysis strategy
        const auto &simParams = netListExtractor_.getSimulationParameters();
        if (simParams.isTransient()) {
            analysis_ = std::make_unique<TransientAnalysis>(simParams.transientParameters_);
            std::cout << "Simulator: Transient analysis strategy selected." << std::endl;
        } else if (simParams.isDCSweep()) {
            // analysisStrategy_ = std::make_unique<DCSweepAnalysis>(simParams.dcSweepParameters_);
            std::cerr << "Simulator: DC Sweep analysis not yet fully implemented in Simulator class." << std::endl;
            return false;
        } else if (simParams.isOPPoint()) {
            // analysisStrategy_ = std::make_unique<DCOpAnalysis>(/* simParams if needed */);
            std::cerr << "Simulator: DC Operating Point analysis not yet fully implemented in Simulator class." <<
                    std::endl;
            return false;
        } else {
            std::cerr << "Simulator: No valid analysis type specified." << std::endl;
            return false;
        }

        if (analysis_ && circuit_) {
            analysis_->solve(*circuit_); // Polymorphic call to solve
            std::cout << "Simulator: Analysis completed." << std::endl;
        } else {
            std::cerr << "Simulator: Failed to create analysis strategy or circuit." << std::endl;
            return false;
        }
    } catch (const std::exception &e) {
        std::cerr << "Simulator: Exception caught: " << e.what() << std::endl;
        return false;
    }
    return true;
}
