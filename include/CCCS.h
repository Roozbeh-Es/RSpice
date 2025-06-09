#ifndef CCCS_H
#define CCCS_H

#include "AbstractCurrentSource.h"

class CCCS : public AbstractCurrentSource {
private:
    std::string VSensorName_;
    int VSensorEquationIndex_;
    double gain_;

public:
    CCCS(const std::string &name,
         const std::string &out_p_node,
         const std::string &out_n_node,
         const std::string &VSensorName,
         double gain
    ) : AbstractCurrentSource(name, out_p_node, out_n_node), VSensorName_(VSensorName), gain_(gain) {
    }
    ~CCCS()  final  = default;

    sunrealtype getCurrent(sunrealtype t, N_Vector y) const override {
        sunrealtype *y_data = N_VGetArrayPointer(y);

        sunrealtype I = y_data[VSensorEquationIndex_];

        return gain_ * I;
    }

    std::string getType() const override{ return "CCCS"; }

    sunrealtype getCurrent(N_Vector y) override {
        sunrealtype *y_data = N_VGetArrayPointer(y);

        sunrealtype I = y_data[VSensorEquationIndex_];

        return gain_ * I;
    }

    void setVSensorIndex(int index) { this->VSensorEquationIndex_ = index; }

    [[nodiscard]] std::string getSensorName() const { return (VSensorName_); }
};

#endif //CCCS_H
