#ifndef CCVS_H
#define CCVS_H
#include "AbstractVoltageSource.h"

class CCVS : public AbstractVoltageSource {
private:
    std::string VSensorName_;
    int VSensorEquationIndex_;
    double gain_;

public:
    CCVS(const std::string &name,
         const std::string &out_p_node,
         const std::string &out_n_node,
         const std::string &VSensorName,
         double gain
    ) : AbstractVoltageSource(name, out_p_node, out_n_node), VSensorName_(VSensorName), gain_(gain) {
    }

    ~CCVS() final = default;

    sunrealtype getVoltage(sunrealtype t, N_Vector y) const override {
        sunrealtype *y_data = N_VGetArrayPointer(y);

        sunrealtype I = y_data[VSensorEquationIndex_];

        return gain_ * I;
    }

    sunrealtype getVoltage(N_Vector y) override {
        sunrealtype *y_data = N_VGetArrayPointer(y);

        sunrealtype I = y_data[VSensorEquationIndex_];

        return gain_ * I;
    }

    void setVSensorIndex(int index) { this->VSensorEquationIndex_ = index; }

    [[nodiscard]] std::string getSensorName() const { return (VSensorName_); }
};

#endif //CCVS_H
