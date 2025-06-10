#ifndef IPULSE_H
#define IPULSE_H
#include "AbstractCurrentSource.h"

class IPulse : public AbstractCurrentSource {
private:
    double initialValue_;
    double pulsedValue_;
    double timeDelay_;
    double riseTime_;
    double fallTime_;
    double pulseWidth_;
    double period_;

public:
    IPulse(const std::string &name, const std::string &node1, const std::string &node2,
           double initialValue,
           double pulsedValue,
           double timeDelay,
           double riseTime,
           double fallTime,
           double pulseWidth,
           double period
    ) : AbstractCurrentSource(name, node1, node2),
        initialValue_(initialValue),
        pulsedValue_(pulsedValue),
        timeDelay_(timeDelay),
        riseTime_(std::max(1e-9, riseTime)),
        fallTime_(std::max(1e-9, fallTime)),
        pulseWidth_(pulseWidth),
        period_(period) {
    }

    ~IPulse() final = default;

    [[nodiscard]] std::string getType() const override { return "IPulse"; }

    sunrealtype getCurrent(sunrealtype t, N_Vector y) const override {
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

    sunrealtype getCurrent(N_Vector y) override {
        return initialValue_;
    }
};


#endif //IPULSE_H
