#ifndef DCCURRENTSOURCE_H
#define DCCURRENTSOURCE_H

#include "AbstractCurrentSource.h"

class DCCurrentSource : public AbstractCurrentSource {
private:
    double DCCurrent_;
    double saveCurrent_;
    bool first = false;

public:
    DCCurrentSource(std::string name, std::string node1Name, std::string node2Name_,
                    double current) : AbstractCurrentSource(std::move(name), std::move(node1Name),
                                                            std::move(node2Name_)), DCCurrent_(current) {
    };

    DCCurrentSource(double DCCurrent);

    ~DCCurrentSource() override final = default;


    bool isLinear() override {
        return false;
    }

    [[nodiscard]] std::string getType() const override {
        return "DC Current Source";
    }

    sunrealtype getCurrent(sunrealtype t, N_Vector y) const override {
        return DCCurrent_;
    }

    sunrealtype getCurrent() override {
        return DCCurrent_;
    }

    void setCurrent(double newCurrent) {
        if(!first) {
            saveCurrent_ = DCCurrent_;
            first = true;
        }
        DCCurrent_ = newCurrent;
    }

};

#endif //DCCURRENTSOURCE_H
