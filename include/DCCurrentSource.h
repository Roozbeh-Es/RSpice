#ifndef DCCURRENTSOURCE_H
#define DCCURRENTSOURCE_H

#include "AbstractCurrentSource.h"

class DCCurrentSource : public AbstractCurrentSource {
private:
    double DCCurrent_;

public:
    DCCurrentSource();

    DCCurrentSource(double DCCurrent);

    ~DCCurrentSource() override final = default;


    bool isLinear() override {
        return false;
    }

    [[nodiscard]] std::string getType() const override {
        return "DC Current Source";
    }

    sunrealtype getCurrent(sunrealtype  t, N_Vector y) const override {
        return DCCurrent_;
    }
};

#endif //DCCURRENTSOURCE_H
