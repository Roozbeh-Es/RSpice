#include "AbstractCurrentSource.h"
#ifndef VCCS_H
#define VCCS_H

class VCCS : public AbstractCurrentSource {
public:
    VCCS(const std::string &name,
         const std::string &node1Name_,
         const std::string &node2Name,
         const std::string control_p_node_name,
         const std::string control_n_node_name,
         double gain
    ) : AbstractCurrentSource(name, node1Name_, node2Name), control_p_node_name_(control_p_node_name),
        control_n_node_name_(control_n_node_name), gain_(gain) {
    }

    ~VCCS() override final  = default;

    [[nodiscard]] std::string getType() const override { return "VCCS"; }

    [[nodiscard]] std::vector<std::string> getAllNodeNames() const {
        return {node1Name_, node2Name_, control_p_node_name_, control_n_node_name_};
    }



    void setControlNodeIndices(int p_idx, int n_idx) {
        control_p_node_idx_ = p_idx;
        control_n_node_idx_ = n_idx;
    }

    sunrealtype getCurrent(sunrealtype t, N_Vector y) const override {
        sunrealtype* y_data = N_VGetArrayPointer(y);

        sunrealtype Vp = (control_p_node_idx_ == 0) ? 0.0 : y_data[control_p_node_idx_ - 1];
        sunrealtype Vn = (control_n_node_idx_ == 0) ? 0.0 : y_data[control_n_node_idx_ - 1];

        return gain_ * (Vp - Vn);
    }

    sunrealtype getCurrent(N_Vector y) override {
        sunrealtype *y_data = N_VGetArrayPointer(y);
        sunrealtype Vp = (control_p_node_idx_ == 0) ? 0.0 : y_data[control_p_node_idx_ - 1];
        sunrealtype Vn = (control_n_node_idx_ == 0) ? 0.0 : y_data[control_n_node_idx_ - 1];
        return gain_ * (Vp - Vn);
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

#endif //VCCS_H
