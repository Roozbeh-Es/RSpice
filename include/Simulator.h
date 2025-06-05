#include <utility> // For std::move
#include "string"
#include "memory"
#include "Analysis.h"
#include "NetListExtractor.h"
#include "Circuit.h"

class Simulator {
public:
    // Corrected Constructor
    explicit Simulator(std::string filePath) 
        : filePath_(std::move(filePath)),       // 1. Move the path into the filePath_ member
          netListExtractor_(this->filePath_)  // 2. Initialize extractor with the now-valid member
    {}

    bool run(); // Let's make this take no arguments, since it uses the member filePath_

private:
    std::string filePath_;
    NetListExtractor netListExtractor_;
    std::unique_ptr<Circuit> circuit_;
    std::unique_ptr<Analysis> analysis_;
};