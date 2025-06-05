#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <string>
#include <memory>
#include "NetListExtractor.h"
#include "Circuit.h"
#include "Analysis.h"

class Simulator {
public:
    explicit Simulator(std::string filePath) : netListExtractor_(std::move(filePath)), filePath_(filePath_) {}

    bool runFile(const std::string &netListFilePath);

private:
    std::string filePath_;
    NetListExtractor netListExtractor_;
    std::unique_ptr<Circuit> circuit_;
    std::unique_ptr<Analysis> analysis_;
};


#endif //SIMULATOR_H
