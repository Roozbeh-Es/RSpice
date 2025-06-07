#ifndef VCVS_H
#define VCVS_H

#include "AbstractVoltageSource.h"

class VCVS : public AbstractVoltageSource {
public:
    VCVS(const std::string &name,
         const std::string &out_p_node, const std::string &out_n_node,
         const std::string &control_p_node, const std::string &control_n_node,
         double gain) : AbstractVoltageSource(name, out_p_node, out_n_node), control_p_node_name_(control_p_node),
                        control_n_node_name_(control_n_node), gain_(gain) {
    }

    ~VCVS() override = default;


    [[nodiscard]] std::string getType() const override { return "VCVS"; }

    [[nodiscard]] std::vector<std::string> getAllNodeNames() const {
        return {node1Name_, node2Name_, control_p_node_name_, control_n_node_name_};
    }

    sunrealtype getVoltage(sunrealtype t, N_Vector y) const override {
        sunrealtype *y_data = N_VGetArrayPointer(y);
        sunrealtype Vp = (control_p_node_idx_ == 0) ? 0.0 : y_data[control_p_node_idx_ - 1];
        sunrealtype Vn = (control_n_node_idx_ == 0) ? 0.0 : y_data[control_n_node_idx_ - 1];
        return gain_ * (Vp - Vn);
    }

    sunrealtype getVoltage(N_Vector y) override {
        sunrealtype *y_data = N_VGetArrayPointer(y);
        sunrealtype Vp = (control_p_node_idx_ == 0) ? 0.0 : y_data[control_p_node_idx_ - 1];
        sunrealtype Vn = (control_n_node_idx_ == 0) ? 0.0 : y_data[control_n_node_idx_ - 1];
        return gain_ * (Vp - Vn);
    }


    void setControlNodeIndices(int p_idx, int n_idx) {
        control_p_node_idx_ = p_idx;
        control_n_node_idx_ = n_idx;
    }

    [[nodiscard]] std::string getControlPNodeName() const { return control_p_node_name_; }
    [[nodiscard]] std::string getControlNNodeName() const { return control_n_node_name_; }

private:
    std::string control_p_node_name_;
    std::string control_n_node_name_;

    int control_p_node_idx_ = 0;
    int control_n_node_idx_ = 0;

    double gain_;
};

#endif // VCVS_H
