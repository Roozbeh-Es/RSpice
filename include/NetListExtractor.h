#ifndef NETLISTMANAGER_H
#define NETLISTMANAGER_H
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Element.h"
#include "SimulationParameters.h"
#include <cctype>

class NetListExtractor {
private:
    std::string filePath_;
    std::vector<std::unique_ptr<Element> > elements_;
    std::map<std::string, int> nodeNameToIndex_;
    long int numNodes_;
    long int numVoltageSources_;
    long int numInductors_;
    long int numEquations_;
    SimulationParameters simulationParameters_;

    //temp vector to hold our elements before indexing them
    std::vector<std::unique_ptr<Element> > rawElements_;

    void parseLine(const std::string &line);

    void parseElementLine(const std::string &line_type, const std::vector<std::string> &tokens);

    void parseCommandLine(const std::string &command, const std::vector<std::string> &tokens);

    void performSizingAndIndexing();

    void parseResistor(const std::vector<std::string> &tokens);

    void parseCapacitor(const std::vector<std::string> &tokens);

    void parseInductor(const std::vector<std::string> &tokens);

    void parseVoltageSource(const std::vector<std::string> &tokens);

    void parseCurrentSource(const std::vector<std::string> &tokens);

public:
    explicit NetListExtractor(std::string filePath);

    bool loadAndProcessNetList(const std::string &filePath);

    std::vector<std::unique_ptr<Element> > &&getPreparedElements();

    [[nodiscard]] const std::map<std::string, int> &getNodeMap() const {
        return nodeNameToIndex_;
    }

    [[nodiscard]] long int getNumNodes() const {
        return numNodes_;
    }

    [[nodiscard]] long int getNumVoltageSources() const {
        return numVoltageSources_;
    }

    [[nodiscard]] long int getNumInductors() const {
        return numInductors_;
    }

    [[nodiscard]] long int getNumEquations() const {
        return numEquations_;
    }

    [[nodiscard]] const SimulationParameters &getSimulationParameters() const {
        return simulationParameters_;
    }

    void clear();
};
#endif //NETLISTMANAGER_H
