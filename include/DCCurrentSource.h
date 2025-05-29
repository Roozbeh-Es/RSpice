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

    std::string getType() const override {
        return "DC Current Source";
    }
};

#endif //DCCURRENTSOURCE_H
