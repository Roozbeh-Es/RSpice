#ifndef DCCURRENTSOURCE_H
#define DCCURRENTSOURCE_H

#include "AbstractCurrentSource.h"

class DCCurrentSource : AbstractCurrentSource {
private:
    double DCCurrent_;

public:
    DCCurrentSource();

    DCCurrentSource(double DCCurrent);

    ~DCCurrentSource() override final = default;

    void ResidualStamp(sunrealtype t, N_Vector y, N_Vector yp, N_Vector F_Residual) override;

    bool isLinear() override {
        return false;
    }

    [[nodiscard]] std::string getType() const override {
        return "DC Current Source";
    }

    sunrealtype getCurrent(sunrealtype  t, N_Vector y) override {
        return DCCurrent_;
    }
};

#endif //DCCURRENTSOURCE_H
