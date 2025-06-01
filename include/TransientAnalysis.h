#ifndef TRANSIENTANALYSIS_H
#define TRANSIENTANALYSIS_H
#include <sundials/sundials_nvector.h>

#include "Analysis.h"
#include "SimulationParameters.h"
#include <sundials/sundials_types.h>


int transientIDACallBack(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual, void *user_data);

class TransientAnalysis : public Analysis {
private:
    TransientParameters parameters_;

public:
    explicit TransientAnalysis(const TransientParameters& parameters) : Analysis(), parameters_(parameters) {}

    void solve(Circuit &circuit) override;
};

#endif //TRANSIENTANALYSIS_H
