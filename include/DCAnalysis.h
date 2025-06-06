// DCAnalysis.h
#ifndef DCANALYSIS_H
#define DCANALYSIS_H

#include "Analysis.h"
#include <sunlinsol/sunlinsol_dense.h>
#include <sunmatrix/sunmatrix_dense.h>
#include <nvector/nvector_serial.h>

class DCAnalysis : public Analysis {
public:
    explicit DCAnalysis() = default;
    void solve(Circuit &circuit) override;
};

#endif // DCANALYSIS_H