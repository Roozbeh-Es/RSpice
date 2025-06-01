#ifndef SIMULATIONPARAMETERS_H
#define SIMULATIONPARAMETERS_H
#include <string>

enum class AnalysisType {
    TRANSIENT,
    DC_OPERATING_POINT,
    DC_SWEEP
};

struct TransientParameters {
    double stopTime_;
    double outputTimeStep_;
    double startTime_ = 0.0;
    double maxInternalTimeStep_ = 0.1;
};

struct DCSweepParameters {
    std::string sourceName_;
    double startValue_;
    double stopValue_;
    double increment_;
};

struct SimulationParameters {
    AnalysisType analysisType_;

    TransientParameters transientParameters_;
    DCSweepParameters dcSweepParameters_;

    SimulationParameters() : analysisType_(AnalysisType::TRANSIENT) {}
};

#endif //SIMULATIONPARAMETERS_H
