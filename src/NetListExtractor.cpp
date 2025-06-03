#include "NetListExtractor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "Element.h"
#include "Resistor.h"
#include "Capacitor.h"
#include "Inductor.h"
#include "AbstractVoltageSource.h"
#include "SinusoidalVoltageSource.h"
#include "DCVoltageSource.h"
#include "AbstractCurrentSource.h"

std::string trim(const std::string &line) {
    std::string cleaned;
    bool inSpace = false;

    std::string trimmedLine = line;

    // 1. Trim leading spaces for comment check
    size_t firstNonSpace = trimmedLine.find_first_not_of(" \t\r\n\v\f");
    if (firstNonSpace == std::string::npos)
        return "";

    trimmedLine = trimmedLine.substr(firstNonSpace);

    // 2. Skip full-line SPICE comments
    if (trimmedLine.empty() || trimmedLine[0] == '*' || trimmedLine[0] == '.') {
        return "";
    }

    // 3. Remove inline comments (starts at ; or //)
    size_t semicolonPos = trimmedLine.find(';');
    size_t slashCommentPos = trimmedLine.find("//");

    size_t cutPos = std::min(
        semicolonPos != std::string::npos ? semicolonPos : trimmedLine.length(),
        slashCommentPos != std::string::npos ? slashCommentPos : trimmedLine.length()
    );
    trimmedLine = trimmedLine.substr(0, cutPos);

    // 4. Normalize whitespace to single spaces
    for (char ch: trimmedLine) {
        if (isspace(static_cast<unsigned char>(ch))) {
            if (!inSpace) {
                cleaned += ' ';
                inSpace = true;
            }
        } else {
            cleaned += ch;
            inSpace = false;
        }
    }

    // 5. Final trim leading/trailing whitespace
    size_t start = cleaned.find_first_not_of(' ');
    size_t end = cleaned.find_last_not_of(' ');
    if (start == std::string::npos) return "";
    return cleaned.substr(start, end - start + 1);
}

long double extractValueFromString(const std::string &value) {
    long int i = 0;
    while (i < value.size() && (isdigit(value[i]) || value[i] == '.')) {
        i++;
    }
    long double numberPart = std::stold(value.substr(0, i));
    std::string suffixPart = value.substr(i);

    for (auto &ch: suffixPart) {
        //toupper is a part of C and it expects only positive char values
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    if (suffixPart == "T") {
        numberPart *= 1e12;
    } else if (suffixPart == "G") {
        numberPart *= 1e9;
    } else if (suffixPart == "MEG") {
        numberPart *= 1e6;
    } else if (suffixPart == "K") {
        numberPart *= 1e3;
    } else if (suffixPart == "M") {
        numberPart *= 1e-3;
    } else if (suffixPart == "U") {
        numberPart *= 1e-6;
    } else if (suffixPart == "N") {
        numberPart *= 1e-9;
    } else if (suffixPart == "P") {
        numberPart *= 1e-12;
    } else if (suffixPart == "F") {
        numberPart *= 1e-15;
    }

    return numberPart;
}


void NetListExtractor::clear() {
    filePath_.clear();
    numNodes_ = 0;
    numVoltageSources_ = 0;
    numInductors_ = 0;
    numEquations_ = 0;
    nodeNameToIndex_.clear();
    elements_.clear();
}

bool NetListExtractor::loadAndProcessNetList(const std::string &filePath) {
    clear();
    std::ifstream netListFile(filePath);
    if (!netListFile.is_open()) {
        std::cerr << "Failed to open file " << filePath << std::endl;
        return false;
    }
    std::string line;
    int lineNumber = 0;
    while (std::getline(netListFile, line)) {
        lineNumber++;
        std::string trimmedLine = trim(line);
        if (trimmedLine.empty()) {
            continue;
        }
        parseLine(trimmedLine);
    }
    netListFile.close();
    return true;
}

void NetListExtractor::parseLine(const std::string &line) {
    std::stringstream ss(line);
    std::string firstToken;
    std::vector<std::string> tokens;
    std::string token;

    // just to make sure
    if (!(ss >> firstToken)) {
        throw std::runtime_error("line is empty after string stream");
    }

    std::string upperFirstToken = firstToken;
    std::transform(upperFirstToken.begin(), upperFirstToken.end(), upperFirstToken.begin(), ::toupper);
    tokens.push_back(firstToken);

    while (ss >> token) {
        tokens.push_back(token);
    }

    //just to make sure
    if (tokens.empty()) {
        throw std::runtime_error("No tokens found in line.");
    }

    if (upperFirstToken[0] == '.') {
        std::cout << " NetListExtractor : Identified as a command line. calling parseCommandLine" << std::endl;
        parseCommandLine(upperFirstToken, tokens);
    } else if (isalpha(upperFirstToken[0])) {
        std::cout << "NetListExtractor : Identified as component line. calling parseElementLine" << std::endl;
        parseElementLine(upperFirstToken, tokens);
    } else {
        throw std::runtime_error("Unrecognized command line.");
    }
}

void NetListExtractor::parseElementLine(const std::string &componentType, const std::vector<std::string> &tokens) {
    std::cout << "NetListExtractor: parseElementLine called for type '" << componentType[0] << "' with name '" << tokens
            [0] << "'" << std::endl;


    switch (componentType[0]) {
        case 'R': {
            std::string name = tokens[0];
            std::string node1Name = tokens[1];
            std::string node2Name = tokens[2];
            long double resistance = extractValueFromString(tokens[3]);
            rawElements_.push_back(std::make_unique<Resistor>(name, node1Name, node2Name, resistance));
            break;
        }
        case 'C': {
            std::string name = tokens[0];
            std::string node1Name = tokens[1];
            std::string node2Name = tokens[2];
            long double capacitance = extractValueFromString(tokens[3]);
            rawElements_.push_back(std::make_unique<Capacitor>(name, node1Name, node2Name, capacitance));
            break;
        }
        case 'L': {
            std::string name = tokens[0];
            std::string node1Name = tokens[1];
            std::string node2Name = tokens[2];
            long double inductance = extractValueFromString(tokens[3]);
            rawElements_.push_back(std::make_unique<Inductor>(name, node1Name, node2Name, inductance));
            break;
        }
        case 'V': {
            std::string name = tokens[1];
            std::string node1Name = tokens[2];
            std::string node2Name = tokens[3];
            std::string fourthTokenUpper = tokens[3];
            std::transform(fourthTokenUpper.begin(), fourthTokenUpper.end(), fourthTokenUpper.begin(), ::toupper);
            if (fourthTokenUpper.rfind("SIN(", 0) == 0) {
                std::string combinedSinParams;
                combinedSinParams += tokens[3].substr(fourthTokenUpper.find('(') + 1);
                for (size_t i = 4; i < tokens.size(); i++) {
                    combinedSinParams += " " + tokens[i];
                }
                size_t lastParen = combinedSinParams.rfind('(');
                if (lastParen != std::string::npos) {
                    combinedSinParams = combinedSinParams.substr(0, lastParen);
                } else {
                    throw std::runtime_error("Error: SIN command is not currently terminated by a ')'" << std::endl);
                    break;
                }
                std::stringstream paramStream(combinedSinParams);
                std::string paramToken;
                std::vector<long double> sineParams;
                while (paramStream >> paramToken) {
                    if (!paramToken.empty()) {
                        try {
                            sineParams.emplace_back(extractValueFromString(paramToken));
                        } catch (const std::exception &e) {
                            std::cerr << e.what() << std::endl;
                            return;
                        }
                    }
                }
                if (sineParams.size() >= 3) {
                    long double offset = sineParams[0];
                    long double amplitude = sineParams[1];
                    long double frequency = sineParams[2];
                    long double timeDelay = (sineParams.size() > 3) ? sineParams[3] : 0.0;
                    long double dampingFactor = (sineParams.size() > 4) ? sineParams[4] : 0.0;
                    long double phase = (sineParams.size() > 5) ? sineParams[5] : 0.0;
                    rawElements_.emplace_back(std::make_unique<SinusoidalVoltageSource>(
                        name, node1Name, node2Name, offset, amplitude, frequency, timeDelay,
                        dampingFactor, phase));
                    numVoltageSources_++;
                    std::cout << "Sinusoidal voltage source added" << std::endl;
                } else {
                    throw std::runtime_error(
                        "Error: Insufficient parameters for Sin source" + name + ". Need at least VO, VA, FREQ." +
                        '\n');
                }
            } else {
                long double dcValue;
                if (fourthTokenUpper == "DC") {
                    if (tokens.size() < 5) {
                        throw std::runtime_error("Error: DC voltage source " + name + " missing value after DC keyword." + '\n');
                    }
                    try {
                        dcValue = extractValueFromString(tokens[4]);
                    } catch (const std::exception &e) {
                        std::cerr << "Error parsing DC value for " << name << " after DC keyword: " << e.what() << std::endl;
                        break;
                    }
                } else {
                    try {
                        dcValue = extractValueFromString(tokens[3]);
                    } catch (const std::exception &e) {
                        std::cerr << "Error parsing DC value for " << name << " (token: " << tokens[3] << "): " << e.what() << std::endl;
                        std::cout << "Note: This might also be an AC specification or other unhandled VSource parameter." << std::endl;
                        break;
                    }
                }
                rawElements_.emplace_back(std::make_unique<DCVoltageSource>(name, node1Name, node2Name, dcValue));
                numVoltageSources_++;
            }
        }
        default:
            std::cout << "NetListExtractor: Element type '" << componentType[0] << "' with name '" << tokens[1] << "' not yet supported for detailed parsing or is a sub-circuit." << std::endl;
        break;
    }
}
