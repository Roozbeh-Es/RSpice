#include "NetListExtractor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "Element.h"
#include "Resistor.h"
#include "Capacitor.h"
#include "Inductor.h"
#include "SinusoidalVoltageSource.h"
#include "DCVoltageSource.h"
#include "DCCurrentSource.h"
#include "SinusoidalCurrentSource.h"
#include "SimulationParameters.h"
#include "Diode.h"
#include "VCVS.h"
#include "VCCS.h"
#include "CCVS.h"
#include "CCCS.h"
#include "VPulse.h"
#include "IPulse.h"
#include "set"

NetListExtractor::NetListExtractor(std::string filePath)
    : filePath_(std::move(filePath)) {
    std::cout << "NetListExtractor created for file: " << filePath_ << std::endl;
}

std::vector<std::unique_ptr<Element> > &&NetListExtractor::getPreparedElements() {
    return std::move(elements_);
}

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
    if (trimmedLine.empty() || trimmedLine[0] == '*') {
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
    size_t i = 0;

    if (value[i] == '+' || value[i] == '-') {
        i++;
    }

    while (i < value.size() && (isdigit(value[i]) || value[i] == '.')) {
        i++;
    }

    long double numberPart = std::stold(value.substr(0, i));
    std::string suffixPart = value.substr(i);

    for (auto &ch: suffixPart) {
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
    numNodes_ = 0;
    numVoltageSources_ = 0;
    numInductors_ = 0;
    numEquations_ = 0;
    //numDiodes_ = 0;
    nodeNameToIndex_.clear();
    elements_.clear();
}

bool NetListExtractor::loadAndProcessNetList() {
    clear();
    std::ifstream netListFile(filePath_);

    std::cout << "[DEBUG] Trying to open file with path: \"" << filePath_ << "\"" << std::endl;

    if (!netListFile.is_open()) {
        std::cerr << "Failed to open file " << filePath_ << std::endl;

        perror("OS-level reason");

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
    performSizingAndIndexing();
    return true;
}

void NetListExtractor::parseResistor(const std::vector<std::string> &tokens) {
    std::string name = tokens[0];
    if (tokens.size() < 4) {
        throw std::runtime_error("Error: Resistor " + name + " has insufficient parameters. Expected at least 4.\n");
    }
    std::string node1Name = tokens[1];
    std::string node2Name = tokens[2];
    long double resistance = extractValueFromString(tokens[3]);

    this->rawElements_.push_back(std::make_unique<Resistor>(name, node1Name, node2Name, resistance));
    std::cout << "NetListExtractor: Parsed Resistor: " << name << std::endl;
}

void NetListExtractor::parseCapacitor(const std::vector<std::string> &tokens) {
    std::string name = tokens[0];
    if (tokens.size() < 4) {
        throw std::runtime_error("Error: Capacitor " + name + " has insufficient parameters. Expected 4.\n");
    }
    std::string node1Name = tokens[1];
    std::string node2Name = tokens[2];
    long double capacitance = extractValueFromString(tokens[3]);

    this->rawElements_.push_back(std::make_unique<Capacitor>(name, node1Name, node2Name, capacitance));
    std::cout << "NetListExtractor: Parsed Capacitor: " << name << std::endl;
}

void NetListExtractor::parseInductor(const std::vector<std::string> &tokens) {
    std::string name = tokens[0];
    if (tokens.size() < 4) {
        throw std::runtime_error("Error: Inductor " + name + " has insufficient parameters. Expected 4.\n");
    }
    std::string node1Name = tokens[1];
    std::string node2Name = tokens[2];
    long double inductance = extractValueFromString(tokens[3]);

    this->rawElements_.push_back(std::make_unique<Inductor>(name, node1Name, node2Name, inductance));
    this->numInductors_++;
    std::cout << "NetListExtractor: Parsed Inductor: " << name << std::endl;
}

void NetListExtractor::parseDiode(const std::vector<std::string> &tokens) {
    std::string name = tokens[0];
    if (tokens.size() < 4) {
        throw std::runtime_error("Error: Diode " + name + " has insufficient parameters. Expected 4.\n");
    }
    std::string node1Name = tokens[1];
    std::string node2Name = tokens[2];
    long double forwardVoltage = extractValueFromString(tokens[3]);
    this->rawElements_.push_back(std::make_unique<Diode>(name, node1Name, node2Name, forwardVoltage));
    //numDiodes_++;
    std::cout << "NetListExtractor: Parsed Diode: " << name << "with forward Voltage : " << forwardVoltage << std::endl;
}

void NetListExtractor::parseVoltageSource(const std::vector<std::string> &tokens) {
    std::string name = tokens[0];
    std::string node1Name = tokens[1];
    std::string node2Name = tokens[2];

    if (tokens.size() < 4) {
        throw std::runtime_error("Error: Voltage source " + name + " requires at least a value/type after nodes.\n");
    }

    std::string paramStartToken = tokens[3];
    std::string paramStartTokenUpper = paramStartToken;
    std::transform(paramStartTokenUpper.begin(), paramStartTokenUpper.end(), paramStartTokenUpper.begin(), ::toupper);

    if (paramStartTokenUpper.rfind("SINE(", 0) == 0) {
        std::string combinedSinParams;
        combinedSinParams += paramStartToken.substr(paramStartTokenUpper.find('(') + 1);

        for (size_t i = 4; i < tokens.size(); ++i) {
            combinedSinParams += " " + tokens[i];
        }

        size_t lastParen = combinedSinParams.rfind(')');
        if (lastParen != std::string::npos) {
            combinedSinParams = combinedSinParams.substr(0, lastParen);
        } else {
            throw std::runtime_error("Error: SIN expression for " + name + " is not correctly terminated with ')'.\n");
        }

        std::stringstream paramStream(combinedSinParams);
        std::string paramTokenStr;
        std::vector<long double> sineParams;
        while (paramStream >> paramTokenStr) {
            if (!paramTokenStr.empty()) {
                try {
                    sineParams.emplace_back(extractValueFromString(paramTokenStr));
                } catch (const std::exception &e) {
                    std::string errMsg = "Error parsing SIN parameter '" + paramTokenStr + "' for " + name + ": " + e.
                                         what() + "\n";
                    std::cerr << errMsg;
                    throw std::runtime_error(errMsg);
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

            this->rawElements_.emplace_back(std::make_unique<SinusoidalVoltageSource>(
                name, node1Name, node2Name,
                offset, amplitude, frequency, timeDelay, dampingFactor, phase));
            this->numVoltageSources_++;
            std::cout << "NetListExtractor: Parsed Sinusoidal Voltage Source: " << name << std::endl;
        } else {
            throw std::runtime_error(
                "Error: Insufficient parameters for SIN source " + name + ". Need at least VO, VA, FREQ.\n");
        }
    } else {
        long double dcValue;
        if (paramStartTokenUpper == "DC") {
            if (tokens.size() < 5) {
                throw std::runtime_error("Error: DC voltage source " + name + " missing value after DC keyword.\n");
            }
            try {
                dcValue = extractValueFromString(tokens[4]);
            } catch (const std::exception &e) {
                std::string errMsg = "Error parsing DC value for " + name + " after DC keyword (token: " + tokens[4] +
                                     "): " + e.what() + "\n";
                std::cerr << errMsg;
                return;
            }
        } else {
            try {
                dcValue = extractValueFromString(paramStartToken);
            } catch (const std::exception &e) {
                std::string errMsg = "Error parsing DC value for " + name + " (token: " + paramStartToken + "): " + e.
                                     what() + "\n";
                std::cerr << errMsg;
                std::cout << "Note: This might also be an AC specification or other unhandled VSource parameter." <<
                        std::endl;
                return;
            }
        }
        this->rawElements_.emplace_back(std::make_unique<DCVoltageSource>(name, node1Name, node2Name, dcValue));
        this->numVoltageSources_++;
        std::cout << "NetListExtractor: Parsed DC Voltage Source: " << name << " Value: " << dcValue << std::endl;
    }
}

void NetListExtractor::parseVCVS(const std::vector<std::string> &tokens) {
    if (tokens.size() < 6) {
        throw std::runtime_error(
            "VCVS " + tokens[0] +
            " has insufficient parameters. Expected 6 tokens: <name> <out+> <out-> <in+> <in-> <gain>");
    }

    const std::string &name = tokens[0];
    const std::string &out_p_node = tokens[1];
    const std::string &out_n_node = tokens[2];
    const std::string &control_p_node = tokens[3];
    const std::string &control_n_node = tokens[4];

    double gain = extractValueFromString(tokens[5]);

    rawElements_.push_back(std::make_unique<VCVS>(
        name,
        out_p_node,
        out_n_node,
        control_p_node,
        control_n_node,
        gain
    ));


    this->numVoltageSources_++;

    std::cout << "NetListExtractor: Parsed VCVS: " << name << std::endl;
}

void NetListExtractor::parseCCVS(const std::vector<std::string> &tokens) {
    if (tokens.size() < 5) {
        throw std::runtime_error("CCVS" + tokens[0] + " has insufficient  parameters");
    }

    const std::string &name = tokens[0];
    const std::string &out_p_node = tokens[1];
    const std::string &out_n_node = tokens[2];
    const std::string &VSensorName = tokens[3];
    double gain = extractValueFromString(tokens[4]);

    rawElements_.push_back(std::make_unique<CCVS>(
        name,
        out_p_node,
        out_n_node,
        VSensorName,
        gain
    ));
    this->numVoltageSources_++;
    std::cout << "NetListExtractor: Parsed CCVS: " << name << std::endl;
}

void NetListExtractor::parseVPulse(const std::vector<std::string> &tokens) {
    if (tokens.size() < 10) {
        throw std::runtime_error("VPulse" + tokens[0] + " has insufficient  parameters");
    }
    const std::string &name = tokens[0];
    const std::string &out_p_node = tokens[1];
    const std::string &out_n_node = tokens[2];
    const double initialVoltage = extractValueFromString(tokens[3]);
    double VON = extractValueFromString(tokens[4]);
    double timeDelay = extractValueFromString(tokens[5]);
    double riseTime = extractValueFromString(tokens[6]);
    double fallTime = extractValueFromString(tokens[7]);
    double TOn = extractValueFromString(tokens[8]);
    double timePeriod = extractValueFromString(tokens[9]);

    rawElements_.push_back(std::make_unique<VPulse>(name,
                                                    out_p_node,
                                                    out_n_node,
                                                    initialVoltage,
                                                    VON,
                                                    timeDelay,
                                                    riseTime,
                                                    fallTime,
                                                    TOn,
                                                    timePeriod));
    this->numVoltageSources_++;
    std::cout << "parsed VPulse element: " << name << std::endl;

}



void NetListExtractor::parseCurrentSource(const std::vector<std::string> &tokens) {
    std::string name = tokens[0];
    std::string node1Name = tokens[1];
    std::string node2Name = tokens[2];

    if (tokens.size() < 4) {
        throw std::runtime_error("Error: Current source " + name + " requires at least a value/type after nodes.\n");
    }

    std::string paramStartToken = tokens[3];
    std::string paramStartTokenUpper = paramStartToken;
    std::transform(paramStartTokenUpper.begin(), paramStartTokenUpper.end(), paramStartTokenUpper.begin(), ::toupper);

    if (paramStartTokenUpper.rfind("SIN(", 0) == 0) {
        std::string combinedSinParams = paramStartToken.substr(paramStartTokenUpper.find('(') + 1);
        for (size_t i = 4; i < tokens.size(); ++i) {
            combinedSinParams += " " + tokens[i];
        }
        size_t lastParen = combinedSinParams.rfind(')');
        if (lastParen != std::string::npos) {
            combinedSinParams = combinedSinParams.substr(0, lastParen);
        } else {
            throw std::runtime_error("Error: SIN expression for " + name + " is not properly terminated with a ')'.\n");
        }

        std::stringstream paramStream(combinedSinParams);
        std::string paramTokenStr;
        std::vector<long double> sineParams;
        while (paramStream >> paramTokenStr) {
            if (!paramTokenStr.empty()) {
                try {
                    sineParams.emplace_back(extractValueFromString(paramTokenStr));
                } catch (const std::exception &e) {
                    std::string errMsg = "Error parsing SIN parameter '" + paramTokenStr + "' for " + name + ": " + e.
                                         what() + "\n";
                    std::cerr << errMsg;
                    throw std::runtime_error(errMsg);
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

            this->rawElements_.emplace_back(std::make_unique<SinusoidalCurrentSource>(
                name, node1Name, node2Name,
                offset, amplitude, frequency, timeDelay, dampingFactor, phase));
            std::cout << "NetListExtractor: Parsed Sinusoidal Current Source: " << name << std::endl;
        } else {
            throw std::runtime_error(
                "Error: Insufficient parameters for SIN source " + name + ". Need at least IO, IA, FREQ.\n");
        }
    } else {
        long double dcValue;
        if (paramStartTokenUpper == "DC") {
            if (tokens.size() < 5) {
                throw std::runtime_error("Error: DC current source " + name + " missing value after DC keyword.\n");
            }
            try {
                dcValue = extractValueFromString(tokens[4]);
            } catch (const std::exception &e) {
                std::string errMsg = "Error parsing DC value for " + name + " after DC keyword (token: " + tokens[4] +
                                     "): " + e.what() + "\n";
                std::cerr << errMsg;
                return;
            }
        } else {
            try {
                dcValue = extractValueFromString(paramStartToken);
            } catch (const std::exception &e) {
                std::string errMsg = "Error parsing DC value for " + name + " (token: " + paramStartToken + "): " + e.
                                     what() + "\n";
                std::cerr << errMsg;
                std::cout << "Note: This might also be an AC specification or other unhandled ISource parameter." <<
                        std::endl;
                return;
            }
        }
        this->rawElements_.emplace_back(std::make_unique<DCCurrentSource>(name, node1Name, node2Name, dcValue));
        std::cout << "NetListExtractor: Parsed DC Current Source: " << name << " Value: " << dcValue << std::endl;
    }
}


void NetListExtractor::parseVCCS(const std::vector<std::string> &tokens) {
    if (tokens.size() < 6) {
        throw std::runtime_error("VCCS " + tokens[0] + " has insufficient parameters. Expected 6 tokens.");
    }
    const std::string &name = tokens[0];
    const std::string &out_p_node = tokens[1];
    const std::string &out_n_node = tokens[2];
    const std::string &control_p_node = tokens[3];
    const std::string &control_n_node = tokens[4];

    double gain = extractValueFromString(tokens[5]);
    rawElements_.push_back(std::make_unique<VCCS>(name,
                                                  out_p_node, out_n_node, control_p_node, control_n_node, gain
    ));
    std::cout << "NetListExtractor: Parsed VCCS: " << name << std::endl;
}

void NetListExtractor::parseCCCS(const std::vector<std::string> &tokens) {
    if (tokens.size() < 5) {
        throw std::runtime_error("CCCS " + tokens[0] + " has insufficient parameters. Expected 5 tokens. ");
    }
    const std::string &name = tokens[0];
    const std::string &out_p_node = tokens[1];
    const std::string &out_n_node = tokens[2];
    const std::string &VSensorName = tokens[3];
    double gain = extractValueFromString(tokens[4]);

    rawElements_.push_back(std::make_unique<CCCS>(name,
                                                  out_p_node,
                                                  out_n_node,
                                                  VSensorName,
                                                  gain
    ));
    std::cout << "NetListExtractor: Parsed CCCS: " << name << std::endl;
}

void NetListExtractor::parseIPulse(const std::vector<std::string> &tokens) {
    if (tokens.size() < 10) {
        throw std::runtime_error("IPulse" + tokens[0] + " has insufficient  parameters");
    }
    const std::string &name = tokens[0];
    const std::string &out_p_node = tokens[1];
    const std::string &out_n_node = tokens[2];
    const double initialVoltage = extractValueFromString(tokens[3]);
    double VON = extractValueFromString(tokens[4]);
    double timeDelay = extractValueFromString(tokens[5]);
    double riseTime = extractValueFromString(tokens[6]);
    double fallTime = extractValueFromString(tokens[7]);
    double TOn = extractValueFromString(tokens[8]);
    double timePeriod = extractValueFromString(tokens[9]);


    rawElements_.push_back(std::make_unique<IPulse>(name,
                                                    out_p_node,
                                                    out_n_node,
                                                    initialVoltage,
                                                    VON,
                                                    timeDelay,
                                                    riseTime,
                                                    fallTime,
                                                    TOn,
                                                    timePeriod));
    std::cout << "parsed IPulse element: " << name << std::endl;
}


void NetListExtractor::parseVDelta(const std::vector<std::string> &tokens) {
    if (tokens.size() < 3) {
        throw std::runtime_error("Special Dirac source requires a name and 2 node names.");
    }

    const std::string& name = tokens[0];
    const std::string& node1 = tokens[1];
    const std::string& node2 = tokens[2];


    const double total_duration = 2e-9;

    const double rise_time = total_duration / 2.0;
    const double fall_time = total_duration / 2.0;
    const double pulse_width = 0.0;


    const double peak_voltage = 2.0 / total_duration;

    const double initial_value = 0.0;
    const double time_delay = 0.0;
    const double period = 1.0;

    std::cout << "NetListExtractor: Parsed Dirac Delta '" << name
              << "' (approximated as a " << total_duration * 1e9 << "ns, "
              << peak_voltage * 1e-9 << "GV triangular pulse)." << std::endl;

    rawElements_.push_back(std::make_unique<VPulse>(
        name,
        node1,
        node2,
        initial_value,
        peak_voltage,
        time_delay,
        rise_time,
        fall_time,
        pulse_width,
        period
    ));

    this->numVoltageSources_++;
}

void NetListExtractor::parseIDelta(const std::vector<std::string> &tokens) {
    if (tokens.size() < 3) {
        throw std::runtime_error("Special Dirac source requires a name and 2 node names.");
    }

    const std::string& name = tokens[0];
    const std::string& node1 = tokens[1];
    const std::string& node2 = tokens[2];


    const double total_duration = 2e-9;

    const double rise_time = total_duration / 2.0;
    const double fall_time = total_duration / 2.0;
    const double pulse_width = 0.0;


    const double peak_voltage = 2.0 / total_duration;

    const double initial_value = 0.0;
    const double time_delay = 0.0;
    const double period = 1.0;

    std::cout << "NetListExtractor: Parsed Dirac Delta '" << name
              << "' (approximated as a " << total_duration * 1e9 << "ns, "
              << peak_voltage * 1e-9 << "GV triangular pulse)." << std::endl;

    rawElements_.push_back(std::make_unique<IPulse>(
        name,
        node1,
        node2,
        initial_value,
        peak_voltage,
        time_delay,
        rise_time,
        fall_time,
        pulse_width,
        period
    ));

    this->numVoltageSources_++;
}




void NetListExtractor::parseLine(const std::string &line) {
    std::stringstream ss(line);
    std::string firstToken;
    std::vector<std::string> tokens;
    std::string token;

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

void NetListExtractor::parseElementLine(const std::string &elementToken, const std::vector<std::string> &tokens) {
    if (tokens.empty()) {
        throw std::runtime_error("parseElementLine called with empty tokens.");
    }

    char typeChar = static_cast<char>(std::toupper(static_cast<unsigned char>(elementToken[0])));

    std::cout << "NetListExtractor: Dispatching parsing for type '" << typeChar << "' with name '" << elementToken <<
            "'" << std::endl;

    switch (typeChar) {
        case 'R':
            parseResistor(tokens);
            break;
        case 'C':
            parseCapacitor(tokens);
            break;
        case 'L':
            parseInductor(tokens);
            break;
        case 'V':
            parseVoltageSource(tokens);
            break;
        case 'I':
            parseCurrentSource(tokens);
            break;
        case 'D':
            parseDiode(tokens);
            break;
        case 'E':
            parseVCVS(tokens);
            break;
        case 'G':
            parseVCCS(tokens);
            break;
        case 'H':
            parseCCVS(tokens);
            break;
        case 'F':
            parseCCCS(tokens);
            break;
        case 'Z':
            parseVPulse(tokens);
        break;
        case 'X':
            parseIPulse(tokens);
        case 'A' :
            parseVDelta(tokens);
        break;
        case 'B':
            parseIDelta(tokens);
        break;
        default:
            std::cout << "NetListExtractor: Element type '" << typeChar << "' with name '" << elementToken <<
                    "' not yet supported for detailed parsing or is a sub-circuit." << std::endl;
            break;
    }
}

void NetListExtractor::parseCommandLine(const std::string &commandToken, const std::vector<std::string> &tokens) {
    if (tokens.empty()) {
        throw std::runtime_error("parseCommandLine called with empty tokens.");
    }
    if (commandToken == ".TRAN") {
        parseTransient(tokens);
    } else if (commandToken == ".DC") {
        parseDC(tokens);
    } else if (commandToken == ".OP") {
        parseOP(tokens);
    } else if (commandToken == ".END") {
        std::cout << "NetListExtractor is done reading the net list" << std::endl;
    }
}

void NetListExtractor::parseTransient(const std::vector<std::string> &tokens) {
    if (tokens.empty()) {
        throw std::runtime_error("parseTransient called with empty tokens.");
    }
    if (tokens.size() < 3) {
        throw std::runtime_error("insufficient information for Transient analysis");
    }
    long double outputTimeStep = extractValueFromString(tokens[1]);

    long double stopTime = extractValueFromString(tokens[2]);

    long double startTime = (tokens.size() > 3) ? extractValueFromString(tokens[3]) : 0.0;

    long double TMaxStep = (tokens.size() > 4) ? extractValueFromString(tokens[4]) : stopTime / 1000.0;

    bool UIC = (tokens.size() > 5 && (tokens[5] == "UIC" || tokens[5] == "Uic" || tokens[5] == "uic")) ? true : false;

    this->simulationParameters_.analysisType_ = AnalysisType::TRANSIENT;

    this->simulationParameters_.transientParameters_ = TransientParameters(
        outputTimeStep, stopTime, startTime, TMaxStep, UIC);

    std::cout << "Transient analysis registered" << std::endl;
}

void NetListExtractor::parseDC(const std::vector<std::string> &tokens) {
    if (tokens.empty()) {
        throw std::runtime_error("parseDC called with empty tokens.");
    }
    if (tokens.size() < 5) {
        throw std::runtime_error("insufficient information for DC analysis");
    }

    const std::string &sourceName = tokens[1];

    long double startValue = extractValueFromString(tokens[2]);

    long double stopValue = extractValueFromString(tokens[3]);

    long double increment = extractValueFromString(tokens[4]);

    this->simulationParameters_.analysisType_ = AnalysisType::DC_SWEEP;

    this->simulationParameters_.DCSweepParameters_ = DCSweepParameters(sourceName, startValue, stopValue, increment);

    std::cout << "DC sweep analysis registered" << std::endl;
}

void NetListExtractor::parseOP(const std::vector<std::string> &tokens) {
    this->simulationParameters_.analysisType_ = AnalysisType::DC_OPERATING_POINT;
    std::cout << "NetListExtractor: Parsed .OP command." << std::endl;
}


void NetListExtractor::performSizingAndIndexing() {
    //zero ground check
    std::set<std::string> uniqueNodeNames;
    bool groundNodeInUse = false;
    for (const auto &elPtr: rawElements_) {
        for (const std::string &name: elPtr->getNodeNames()) {
            uniqueNodeNames.insert(name);
            std::string upperName = name;
            std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
            if (name == "0" || upperName == "GND") {
                groundNodeInUse = true;
            }
        }
    }

    if (!rawElements_.empty() && !groundNodeInUse) {
        throw std::runtime_error(
            "Fatal Error: The circuit has no ground node. Please connect a component to 'GND' or '0'.");
    }

    nodeNameToIndex_.clear();
    nodeNameToIndex_["GND"] = 0;
    nodeNameToIndex_["0"] = 0;
    numNodes_ = 0;
    for (const auto &element_ptr: rawElements_) {
        if (!element_ptr) continue;
        for (std::string name: element_ptr->getNodeNames()) {
            if (nodeNameToIndex_.find(name) == nodeNameToIndex_.end()) {
                nodeNameToIndex_[name] = ++numNodes_;
            }
        }
    }

    for (const auto &element_ptr: rawElements_) {
        element_ptr->setNode1Index(nodeNameToIndex_[element_ptr->getNode1Name()]);
        element_ptr->setNode2Index(nodeNameToIndex_[element_ptr->getNode2Name()]);

        if (auto vcvs_ptr = dynamic_cast<VCVS *>(element_ptr.get())) {
            int control_p_idx = nodeNameToIndex_.at(vcvs_ptr->getControlPNodeName());
            int control_n_idx = nodeNameToIndex_.at(vcvs_ptr->getControlNNodeName());
            vcvs_ptr->setControlNodeIndices(control_p_idx, control_n_idx);
        } else if (auto vccs_ptr = dynamic_cast<VCCS *>(element_ptr.get())) {
            int control_p_idx = nodeNameToIndex_.at(vccs_ptr->getControlPNodeName());
            int control_n_idx = nodeNameToIndex_.at(vccs_ptr->getControlNNodeName());
            vccs_ptr->setControlNodeIndices(control_p_idx, control_n_idx);
        }
    }

    int inductorIndex = numNodes_ + numVoltageSources_;
    int voltageSourceIndex = numNodes_;

    for (const auto &element_ptr: rawElements_) {
        if (auto voltageSource_ptr = dynamic_cast<AbstractVoltageSource *>(element_ptr.get())) {
            voltageSource_ptr->setVoltageSourceEquationIndex(voltageSourceIndex++);
        } else if (auto inductor_ptr = dynamic_cast<Inductor *>(element_ptr.get())) {
            inductor_ptr->setInductorEquationIndex(inductorIndex++);
        }
    }

    std::map<std::string, Element *> nameToElementMap;
    for (const auto &el_ptr: rawElements_) {
        nameToElementMap[el_ptr->getName()] = el_ptr.get();
    }

    for (const auto &el_ptr: rawElements_) {
        if (auto ccvs_ptr = dynamic_cast<CCVS *>(el_ptr.get())) {
            std::string sensorName = ccvs_ptr->getSensorName();

            if (nameToElementMap.find(sensorName) == nameToElementMap.end()) {
                throw std::runtime_error(
                    "CCVS '" + ccvs_ptr->getName() + "' references a non-existent voltage source '" + sensorName +
                    "'.");
            }

            Element *sensorElement = nameToElementMap.at(sensorName);
            if (auto vs_sensor_ptr = dynamic_cast<AbstractVoltageSource *>(sensorElement)) {
                int control_idx = vs_sensor_ptr->getVoltageSourceCurrentIndex();
                ccvs_ptr->setVSensorIndex(control_idx);
            } else {
                throw std::runtime_error(
                    "CCVS '" + ccvs_ptr->getName() + "' must be controlled by the current through a voltage source. '" +
                    sensorName + "' is not a voltage source.");
            }
        } else if (auto cccs_ptr = dynamic_cast<CCCS *>(el_ptr.get())) {
            std::string sensorName = cccs_ptr->getSensorName();
            if (nameToElementMap.find(sensorName) == nameToElementMap.end()) {
                throw std::runtime_error(
                    "CCCS '" + cccs_ptr->getName() + "' references non-existent source '" + sensorName + "'.");
            }
            if (auto vs_sensor = dynamic_cast<AbstractVoltageSource *>(nameToElementMap.at(sensorName))) {
                cccs_ptr->setVSensorIndex(vs_sensor->getVoltageSourceCurrentIndex());
            } else {
                throw std::runtime_error(
                    "CCCS '" + cccs_ptr->getName() + "' must be controlled by the current through a voltage source.");
            }
        }
    }


    numEquations_ = numNodes_ + numInductors_ + numVoltageSources_;
    elements_ = std::move(rawElements_);
}
