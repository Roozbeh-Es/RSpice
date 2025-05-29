#ifndef TRANSIENTANALYSIS_H
#define TRANSIENTANALYSIS_H
#include "Analysis.h"
#include "SimulationParameters.h"

class TransientAnalysis : public Analysis {
private:
    TransientParameters parameters_;

public:
    TransientAnalysis(const TransientParameters& parameters) : Analysis(), parameters_(parameters) {}

    void solve(Circuit &circuit) override {
        
    }
};

#endif //TRANSIENTANALYSIS_H
