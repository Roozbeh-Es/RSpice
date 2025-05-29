#ifndef ANALYSIS_H
#define ANALYSIS_H

class Circuit;
struct SimulationParameters;

class Analysis {
public:
    virtual ~Analysis() = default;

    virtual void solve(Circuit &circuit) = 0;

protected:
    Analysis() = default;
};

#endif //ANALYSIS_H
