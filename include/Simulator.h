#include <utility> // For std::move
#include "string"
#include "memory"
#include "Analysis.h"
#include "NetListExtractor.h"
#include "Circuit.h"

class Simulator {
public:
    explicit Simulator(std::string filePath)
        : filePath_(std::move(filePath)),
          netListExtractor_(this->filePath_)
    {
    }

    bool run();

private:
    std::string filePath_;
    NetListExtractor netListExtractor_;
    std::unique_ptr<Circuit> circuit_;
    std::unique_ptr<Analysis> analysis_;
};
