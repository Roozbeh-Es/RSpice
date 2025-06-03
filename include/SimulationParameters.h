#ifndef SIMULATIONPARAMETERS_H
#define SIMULATIONPARAMETERS_H
#include <string>
#include <utility>

enum class AnalysisType {
    TRANSIENT,
    DC_OPERATING_POINT,
    DC_SWEEP
};

struct TransientParameters {
    TransientParameters() = default;

    TransientParameters(long double outputTimeStep, long double stopTime, long double startTime, long double maxInternalTimeStep,
                        bool UIC) : outputTimeStep_(outputTimeStep), stopTime_(stopTime), startTime_(startTime),
                                    maxInternalTimeStep_(maxInternalTimeStep), UIC_(UIC) {
    }

    long double outputTimeStep_;
    long double stopTime_;
    long double startTime_ = 0.0;
    long double maxInternalTimeStep_ = 0.1;
    bool UIC_;
};

struct DCSweepParameters {
    DCSweepParameters() = default;

    DCSweepParameters(std::string sourceName, long double startValue, long double stopValue,
                      long double increment) : sourceName_(std::move(sourceName)), startValue_(startValue),
                                          stopValue_(stopValue),
                                          increment_(increment) {
    }

    std::string sourceName_;
    long double startValue_;
    long double stopValue_;
    long double increment_;
};

struct SimulationParameters {
    SimulationParameters() = default;

    AnalysisType analysisType_ = AnalysisType::DC_OPERATING_POINT;

    TransientParameters transientParameters_;
    DCSweepParameters DCSweepParameters_;

    [[nodiscard]] bool isTransient() const { return analysisType_ == AnalysisType::TRANSIENT; }
    [[nodiscard]] bool isDCSweep() const { return analysisType_ == AnalysisType::DC_SWEEP; }
    [[nodiscard]] bool isOPPoint() const { return analysisType_ == AnalysisType::DC_OPERATING_POINT; }
};

#endif //SIMULATIONPARAMETERS_H
