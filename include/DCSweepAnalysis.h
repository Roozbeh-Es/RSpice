#ifndef DCSWEEPANALYSIS_H
#define DCSWEEPANALYSIS_H

#include "Analysis.h"
#include "SimulationParameters.h"


class DCSweepAnalysis : public Analysis {
public:

    explicit DCSweepAnalysis(const DCSweepParameters& parameters);
    void solve(Circuit& circuit) override;

private:
    DCSweepParameters parameters_;
};

#endif // DCSWEEPANALYSIS_H