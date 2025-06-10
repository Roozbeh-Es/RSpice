#ifndef VPULSE_H
#define VPULSE_H

#include "AbstractVoltageSource.h"
#include <cmath>
#include <algorithm>

class VPulse : public AbstractVoltageSource {
private:
    double initialValue_;
    double pulsedValue_;
    double timeDelay_;
    double riseTime_;
    double fallTime_;
    double pulseWidth_;
    double period_;

public:
    VPulse(const std::string &name, const std::string &node1, const std::string &node2,
           double initialValue,
           double pulsedValue,
           double timeDelay,
           double riseTime,
           double fallTime,
           double pulseWidth,
           double period
    ) : AbstractVoltageSource(name, node1, node2),
        initialValue_(initialValue),
        pulsedValue_(pulsedValue),
        timeDelay_(timeDelay),
        riseTime_(std::max(1e-9, riseTime)),
        fallTime_(std::max(1e-9, fallTime)),
        pulseWidth_(pulseWidth),
        period_(period) {
    }

    ~VPulse() final = default;


    [[nodiscard]] std::string getType() const override { return "VPulse"; }

    sunrealtype getVoltage(sunrealtype t, N_Vector y) const override {
        if (t <= timeDelay_) {
            return initialValue_;
        }

        double t_cycle = fmod(t - timeDelay_, period_);


        if (t_cycle <= riseTime_) {
            double slope = (pulsedValue_ - initialValue_) / riseTime_;
            return initialValue_ + slope * t_cycle;
        } else if (t_cycle <= riseTime_ + pulseWidth_) {
            return pulsedValue_;
        } else if (t_cycle <= riseTime_ + pulseWidth_ + fallTime_) {
            double t_in_fall = t_cycle - (riseTime_ + pulseWidth_);
            double slope = (initialValue_ - pulsedValue_) / fallTime_;
            return pulsedValue_ + slope * t_in_fall;
        } else {
            return initialValue_;
        }
    }

    sunrealtype getVoltage(N_Vector y) override {
        return initialValue_;
    }
};

#endif // VPULSE_H
