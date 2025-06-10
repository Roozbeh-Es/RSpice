#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <fstream>
#include <stdexcept>
#include <set>
#include <algorithm>
#include "Controller.h"

// Forward Declarations
class ElementFile;
class AnalysisFile;
class DCSweepFile;
class TransientFile;
class OPFile;
class ResistorFile;
class CapacitorFile;
class InductorFile;
class DiodeFile;
class DCVoltageSourceFile;
class DCCurrentSourceFile;
class ACVoltageSourceFile;
class VCCSFile;
class VCVSFile;
class CCCSFile;
class CCVSFile;
class DeltaVoltageFile;
class DeltaCurrentFile;

// --- Custom Exception Classes ---
bool OpenOrNew;
class OpenError : public std::exception {
    const char* what() const noexcept override {
        return "An error occurred while opening the file.";
    }
};

class NewError : public std::exception {
    const char* what() const noexcept override {
        return "An error occurred while creating a new file.";
    }
};

class ResValueError : public std::exception {
    const char* what() const noexcept override {
        return "Error: Resistance cannot be zero or negative.";
    }
};

class ResSameError : public std::exception {
    std::string message;
public:
    explicit ResSameError(const std::string& name) : message("Error: Resistor " + name + " already exists in the circuit.") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class CapValueError : public std::exception {
    const char* what() const noexcept override {
        return "Error: Capacitance cannot be zero or negative.";
    }
};

class CapSameError : public std::exception {
    std::string message;
public:
    explicit CapSameError(const std::string& name) : message("Error: Capacitor " + name + " already exists in the circuit.") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class IndValueError : public std::exception {
    const char* what() const noexcept override {
        return "Error: Inductance value cannot be zero or negative.";
    }
};

class IndSameError : public std::exception {
    std::string message;
public:
    explicit IndSameError(const std::string& name) : message("Error: Inductor " + name + " already exists in the circuit.") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class DioSameError : public std::exception {
    std::string message;
public:
    explicit DioSameError(const std::string& name) : message("Error: Diode " + name + " already exists in the circuit.") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class DioModelError : public std::exception {
    std::string message;
public:
    explicit DioModelError(const std::string& name) : message("Error: Diode model " + name + " is not supported.") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class DCVoltSameError : public std::exception {
    std::string message;
public:
    explicit DCVoltSameError(const std::string& name) : message("Error: DC Voltage Source " + name + " already exists in the circuit.") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class DCCurrentSameError : public std::exception {
    std::string message;
public:
    explicit DCCurrentSameError(const std::string& name) : message("Error: DC Current Source " + name + " already exists in the circuit.") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class ACVoltSameError : public std::exception {
    std::string message;
public:
    explicit ACVoltSameError(const std::string& name) : message("Error: AC Voltage Source " + name + " already exists in the circuit.") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class ResistorNotFoundError : public std::exception {
public:
    explicit ResistorNotFoundError(const std::string& name) {}
    const char* what() const noexcept override {
        return "Error: Cannot delete Resistor; component not found.";
    }
};

class CapacitorNotFoundError : public std::exception {
public:
    explicit CapacitorNotFoundError(const std::string& name) {}
    const char* what() const noexcept override {
        return "Error: Cannot delete Capacitor; component not found.";
    }
};

class InductorNotFoundError : public std::exception {
public:
    explicit InductorNotFoundError(const std::string& name) {}
    const char* what() const noexcept override {
        return "Error: Cannot delete Inductor; component not found.";
    }
};

class DiodeNotFoundError : public std::exception {
public:
    explicit DiodeNotFoundError(const std::string& name) {}
    const char* what() const noexcept override {
        return "Error: Cannot delete Diode; component not found.";
    }
};

class GroundNodeNotFoundError : public std::exception {
public:
    explicit GroundNodeNotFoundError(const std::string& name) {}
    const char* what() const noexcept override {
        return "Error: Cannot delete Ground Node; component not found.";
    }
};

class VoltageSourceNotFoundError : public std::exception {
public:
    explicit VoltageSourceNotFoundError(const std::string& name) {}
    const char* what() const noexcept override {
        return "Error: Cannot delete Voltage Source; component not found.";
    }
};

class CurrentSourceNotFoundError : public std::exception {
public:
    explicit CurrentSourceNotFoundError(const std::string& name) {}
    const char* what() const noexcept override {
        return "Error: Cannot delete Current Source; component not found.";
    }
};

class OldNodeNotFound : public std::exception {
    std::string message;
public:
    explicit OldNodeNotFound(const std::string& name) : message("ERROR: Node " + name + " does not exist in the circuit.") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};

class SameNodeName : public std::exception {
    std::string message;
public:
    explicit SameNodeName(const std::string& name) : message("ERROR: Node " + name + " already exists in the circuit.") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};


// --- Base Classes ---

class ElementFile {
public:
    std::string name;
    std::string Node1;
    std::string Node2;
    ElementFile(const std::string& n, const std::string& n1, const std::string& n2)
        : name(n), Node1(n1), Node2(n2) {}
    virtual ~ElementFile() = default;
};

class AnalysisFile {
public:
    std::string Type;
    virtual ~AnalysisFile() = default;
};

// --- Analysis-Type Derived Classes ---

class DCSweepFile : public AnalysisFile {
public:
    std::string SourceName;
    double startVal, endVal, Increment;
    DCSweepFile(const std::string& source, double start, double end, double inc)
        : SourceName(source), startVal(start), endVal(end), Increment(inc) {
        Type = "DCSweep";
    }
};

class TransientFile : public AnalysisFile {
public:
    double startTime, endTime, timeStep, timeStart; // Note: timeStart seems to be a duplicate concept of startTime
    TransientFile(const std::string& source, double start, double end, double step, double maxStep = 0)
        : startTime(start), endTime(end), timeStep(step), timeStart(maxStep) {
        Type = "Transient";
    }
};

class OPFile : public AnalysisFile {
public:
    std::string SourceName;
    OPFile(const std::string& source) : SourceName(source) {
        Type = "OP";
    }
};


// --- Element Derived Classes ---

class ResistorFile : public ElementFile {
public:
    double value;
    int pow;
    ResistorFile(const std::string& n, const std::string& n1, const std::string& n2, double v, int p = 0)
        : ElementFile(n, n1, n2), value(v), pow(p) {}
};

class PulseFile : public ElementFile {
public:
    double vi, von , timedelay, rise, fall, ton, timeperiod;
    int pow;
    PulseFile(const std::string& n, const std::string& n1, const std::string& n2, double vi, double von, double timedelay, double rise, double fall, double ton, double timeperiod)
        : ElementFile(n, n1, n2), vi(vi) {}
};
class CapacitorFile : public ElementFile {
public:
    double value;
    int pow;
    CapacitorFile(const std::string& n, const std::string& n1, const std::string& n2, double v, int p = 0)
        : ElementFile(n, n1, n2), value(v), pow(p) {}
};

class InductorFile : public ElementFile {
public:
    double value;
    int pow;
    InductorFile(const std::string& n, const std::string& n1, const std::string& n2, double v, int p = 0)
        : ElementFile(n, n1, n2), value(v), pow(p) {}
};

class DeltaVoltageFile : public ElementFile {
public:
    DeltaVoltageFile(const std::string& n, const std::string& n1, const std::string& n2)
        : ElementFile(n, n1, n2) {}
};
class DeltaCurrentFile : public ElementFile {
public:
    DeltaCurrentFile(const std::string& n, const std::string& n1, const std::string& n2)
        : ElementFile(n, n1, n2) {}
};

class DiodeFile : public ElementFile {
public:
    double type;
    DiodeFile(const std::string& n, const std::string& n1, const std::string& n2, const double t)
        : ElementFile(n, n1, n2), type(t) {}
};

class DCVoltageSourceFile : public ElementFile {
public:
    double value;
    DCVoltageSourceFile(const std::string& n, const std::string& n1, const std::string& n2, double v)
        : ElementFile(n, n1, n2), value(v) {}
};

class DCCurrentSourceFile : public ElementFile {
public:
    double value;
    DCCurrentSourceFile(const std::string& n, const std::string& n1, const std::string& n2, double v)
        : ElementFile(n, n1, n2), value(v) {}
};

class ACVoltageSourceFile : public ElementFile {
public:
    double value;
    double frequency;
    double phase;
    ACVoltageSourceFile(const std::string& n, const std::string& n1, const std::string& n2, double v, double f, double p)
        : ElementFile(n, n1, n2), value(v), frequency(f), phase(p) {}
};

class VCCSFile : public ElementFile {
public:
    std::string ctrlNode1;
    std::string ctrlNode2;
    double value;
    double gain;
    VCCSFile(const std::string& n, const std::string& n1, const std::string& n2,
         const std::string& cn1, const std::string& cn2, double v, double g)
        : ElementFile(n, n1, n2), ctrlNode1(cn1), ctrlNode2(cn2), value(v), gain(g) {}
};

class VCVSFile : public ElementFile {
public:
    std::string ctrlNode1;
    std::string ctrlNode2;
    double value;
    double gain;
    VCVSFile(const std::string& n, const std::string& n1, const std::string& n2,
         const std::string& cn1, const std::string& cn2, double v, double g)
        : ElementFile(n, n1, n2), ctrlNode1(cn1), ctrlNode2(cn2), value(v), gain(g) {}
};

class CCCSFile : public ElementFile {
public:
    std::string ctrlNode1;
    std::string ctrlNode2;
    double value;
    double gain;
    CCCSFile(const std::string& n, const std::string& n1, const std::string& n2,
         const std::string& cn1, const std::string& cn2, double v, double g)
        : ElementFile(n, n1, n2), ctrlNode1(cn1), ctrlNode2(cn2), value(v), gain(g) {}
};

class CCVSFile : public ElementFile {
public:
    std::string ctrlNode1;
    std::string ctrlNode2;
    double value;
    double gain;
    CCVSFile(const std::string& n, const std::string& n1, const std::string& n2,
         const std::string& cn1, const std::string& cn2, double v, double g)
        : ElementFile(n, n1, n2), ctrlNode1(cn1), ctrlNode2(cn2), value(v), gain(g) {}
};

// --- Global Variables ---
std::vector<std::unique_ptr<ElementFile>> elements;
std::vector<std::string> GroundNodes;
std::string FileName;
std::unique_ptr<AnalysisFile> analysis = nullptr;

// --- Function Implementations ---

bool Menu() {
    std::cout << "Welcome to the RASpice Menu!" << std::endl;
menu_start:
    std::cout << "1. Open File" << std::endl;
    std::cout << "2. New File" << std::endl;
    std::cout << "3. Exit" << std::endl;
    std::cout << "Please select an option: ";
    int choice;
    std::cin >> choice;
    if (choice == 1) {
        std::ifstream file("../settings/recents.txt");
        if (!file.is_open() || file.peek() == std::ifstream::traits_type::eof()) {
            std::cout << "No recent files found." << std::endl;
        } else {
            std::cout << "Recent files:" << std::endl;
            std::string line;
            while (std::getline(file, line)) {
                std::cout << line << std::endl;
            }
        }
        std::cout << "Enter the file name to open: ";
        std::string filename;
        std::cin >> filename;
        std::ifstream file2(filename);
        if (!file2) {
            throw OpenError();
        } else {
            std::cout << "File opened successfully." << std::endl;
        }
        FileName = filename;
        OpenOrNew = true;
        return true;

    } else if (choice == 2) {
        std::cout << "Enter the new file name: ";
        std::string filename;
        std::cin >> filename;
        std::ofstream file(filename);

        if (!file) {
            std::cout << "Error: Could not create file." << std::endl;
            goto menu_start;
        } else {
            std::cout << "File created successfully." << std::endl;
            std::ofstream recent("../settings/recents.txt", std::ios::app);
            recent << filename << "\n";
            FileName = filename;
        }
        OpenOrNew = false;
        return true;
    } else if (choice == 3) {
        return false;
    } else {
        std::cout << "Syntax Error!" << std::endl;
        goto menu_start;
    }
}

bool InputHandler() {
    if (!Menu() || OpenOrNew)
        return false;

    // Regex definitions
    std::regex addResistor(R"(add R(\S+) (\S+) (\S+) (\S+))");
    std::regex addCapacitor(R"(add C(\S+) (\S+) (\S+) (\S+))");
    std::regex addInductor(R"(add L(\S+) (\S+) (\S+) (\S+))");
    std::regex addDiode(R"(add D(\S+) (\S+) (\S+) (\S+))");
    std::regex addGND(R"(add GND (\S+))");
    std::regex addDCVoltageSource(R"(add V(\S+) (\S+) (\S+) ([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?))");
    std::regex addDCCurrentSource(R"(add I(\S+) (\S+) (\S+) ([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?))");
    std::regex addACVoltageSource(R"(add V(\S+) (\S+) (\S+) SIN\((\S+) (\S+) (\S+)\))");
    std::regex addVCVS(R"(add E(\S+) (\S+) (\S+) (\S+) (\S+) (\S+))");
    std::regex addVCCS(R"(add G(\S+) (\S+) (\S+) (\S+) (\S+) (\S+))");
    std::regex addCCVS(R"(add H(\S+) (\S+) (\S+) (\S+) (\S+) (\S+))");
    std::regex addCCCS(R"(add F(\S+) (\S+) (\S+) (\S+) (\S+) (\S+))");
    std::regex deleteResistor(R"(delete R(\S+))");
    std::regex deleteCapacitor(R"(delete C(\S+))");
    std::regex deleteInductor(R"(delete L(\S+))");
    std::regex deleteDiode(R"(delete D(\S+))");
    std::regex deleteGND(R"(delete GND (\S+))");
    std::regex deleteVoltageSource(R"(delete V(\S+))");
    std::regex deleteCurrentSource(R"(delete I(\S+))");
    std::regex addTransient(R"(^\.TRAN\s+(\S+)\s+(\S+)(?:\s+(\S+))?(?:\s+(\S+))?$)");
    std::regex addDCSweep(R"(^\.DC\s+(\S+)\s+([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s+([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s+([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)$)");
    std::regex addOP(R"(^\.OP\s*$)");
    std::regex showNodes(R"(\s*\.nodes\s*)");
    std::regex showElements(R"(\s*\.list (\S+)\s*)");
    std::regex showAllElements(R"(\s*\.list\s*)");
    std::regex renameNode(R"(\s*\.rename node (\S+) (\S+)\s*)");
    std::regex addVoltageDelta(R"(add A(\S+) (\S+))");
    std::regex addCurrentDelta(R"(add B(\S+) (\S+))");
    std::cin.get();
    while (true) {
        try {
            std::string input;
            std::cout << "> ";
            std::getline(std::cin, input);
            if (input == "END") {
                if (GroundNodes.empty()) {
                    throw std::runtime_error("Error: No ground nodes defined.");
                }
                if (analysis == nullptr) {
                    throw std::runtime_error("Error: No analysis defined.");
                }
                std::cout << "Getting Things Ready..." << std::endl;
                break;
            }

            std::smatch match;

            if (std::regex_match(input, match, addResistor)) {
                std::string name = "R" + std::string(match[1]);
                if (std::any_of(elements.begin(), elements.end(), [&](const auto& elem){ return elem->name == name; })) throw ResSameError(name);
                double value = std::stod(match[4]);
                if (value <= 0) throw ResValueError();
                elements.push_back(std::make_unique<ResistorFile>(name, match[2], match[3], value));
            } else if (std::regex_match(input, match, addCapacitor)) {
                std::string name = "C" + std::string(match[1]);
                 if (std::any_of(elements.begin(), elements.end(), [&](const auto& elem){ return elem->name == name; })) throw CapSameError(name);
                double value = std::stod(match[4]);
                if (value <= 0) throw CapValueError();
                elements.push_back(std::make_unique<CapacitorFile>(name, match[2], match[3], value));
            } else if (std::regex_match(input, match, addInductor)) {
                std::string name = "L" + std::string(match[1]); // Corrected from "I"
                if (std::any_of(elements.begin(), elements.end(), [&](const auto& elem){ return elem->name == name; })) throw IndSameError(name);
                double value = std::stod(match[4]);
                if (value <= 0) throw IndValueError();
                elements.push_back(std::make_unique<InductorFile>(name, match[2], match[3], value));
            } else if (std::regex_match(input, match, addDiode)) {
                std::string name = "D" + std::string(match[1]);
                if (std::any_of(elements.begin(), elements.end(), [&](const auto& elem){ return elem->name == name; })) throw DioSameError(name);
                std::string type = match[4];
                if (type[0] == '-') throw DioModelError(type);
                elements.push_back(std::make_unique<DiodeFile>(name, match[2], match[3], std::stod(type)));
            } else if (std::regex_match(input, match, addGND)) {
                GroundNodes.push_back(match[1]);
            } else if (std::regex_match(input, match, addDCVoltageSource)) {
                std::string name = "V" + std::string(match[1]);
                if (std::any_of(elements.begin(), elements.end(), [&](const auto& elem){ return elem->name == name; })) throw DCVoltSameError(name);
                elements.push_back(std::make_unique<DCVoltageSourceFile>(name, match[2], match[3], std::stod(match[4])));
            } else if (std::regex_match(input, match, addDCCurrentSource)) {
                std::string name = "I" + std::string(match[1]);
                if (std::any_of(elements.begin(), elements.end(), [&](const auto& elem){ return elem->name == name; })) throw DCCurrentSameError(name);
                elements.push_back(std::make_unique<DCCurrentSourceFile>(name, match[2], match[3], std::stod(match[4])));
            } else if (std::regex_match(input, match, addACVoltageSource)) {
                std::string name = "V" + std::string(match[1]);
                if (std::any_of(elements.begin(), elements.end(), [&](const auto& elem){ return elem->name == name; })) throw ACVoltSameError(name);
                elements.push_back(std::make_unique<ACVoltageSourceFile>(name, match[2], match[3], std::stod(match[4]), std::stod(match[5]), std::stod(match[6])));
            } else if (std::regex_match(input, match, addVCVS)) {
                std::string name = "E" + std::string(match[1]);
                if (std::any_of(elements.begin(), elements.end(), [&](const auto& elem){ return elem->name == name; })) throw DCVoltSameError(name);
                elements.push_back(std::make_unique<VCVSFile>(name, match[2], match[3], match[4], match[5], std::stod(match[6]), 0));
            } else if (std::regex_match(input, match, addVCCS)) {
                 std::string name = "G" + std::string(match[1]);
                if (std::any_of(elements.begin(), elements.end(), [&](const auto& elem){ return elem->name == name; })) throw DCCurrentSameError(name);
                elements.push_back(std::make_unique<VCCSFile>(name, match[2], match[3], match[4], match[5], std::stod(match[6]), 0));
            } else if (std::regex_match(input, match, deleteResistor)) {
                std::string name = "R" + std::string(match[1]);
                auto it = std::remove_if(elements.begin(), elements.end(), [&](const auto& elem){ return elem->name == name; });
                if(it == elements.end()) throw ResistorNotFoundError(name);
                elements.erase(it, elements.end());
            } else if (std::regex_match(input, match, deleteCapacitor)) {
                std::string name = "C" + std::string(match[1]);
                auto it = std::remove_if(elements.begin(), elements.end(), [&](const auto& elem){ return elem->name == name; });
                if(it == elements.end()) throw CapacitorNotFoundError(name);
                elements.erase(it, elements.end());
            } else if (std::regex_match(input, match, deleteInductor)) {
                std::string name = "L" + std::string(match[1]);
                auto it = std::remove_if(elements.begin(), elements.end(), [&](const auto& elem){ return elem->name == name; });
                if(it == elements.end()) throw InductorNotFoundError(name);
                elements.erase(it, elements.end());
            } else if (std::regex_match(input, match, deleteGND)) {
                 std::string node = match[1];
                 auto it = std::find(GroundNodes.begin(), GroundNodes.end(), node);
                 if (it != GroundNodes.end()) GroundNodes.erase(it);
                 else throw GroundNodeNotFoundError(node);
            } else if (std::regex_match(input, match, addTransient)) {
                double tstep = std::stod(match[1]);
                double tstop = std::stod(match[2]);
                double tstart = match[3].matched ? std::stod(match[3]) : 0.0;
                double tmaxstep = match[4].matched ? std::stod(match[4]) : 0.0;
                analysis = std::make_unique<TransientFile>("", tstart, tstop, tstep, tmaxstep);
            } else if (std::regex_match(input, match, addDCSweep)) {
                analysis = std::make_unique<DCSweepFile>(match[1], std::stod(match[2]), std::stod(match[3]), std::stod(match[4]));
            } else if (std::regex_match(input, addOP)) {
                analysis = std::make_unique<OPFile>("");
            }else if(std::regex_match(input, match, addVoltageDelta)) {
                std::string name = "A" + std::string(match[1]);
                elements.push_back(std::make_unique<DeltaVoltageFile>(name, match[2], match[3]));
            }
            else if(std::regex_match(input, match, addCurrentDelta)) {
                std::string name = "B" + std::string(match[1]);
                elements.push_back(std::make_unique<DeltaCurrentFile>(name, match[2], match[3]));
            }
            else if (std::regex_match(input, showNodes)) {
                std::cout << "Ground Nodes: ";
                for (const auto& node : GroundNodes) std::cout << node << " ";
                std::cout << std::endl;
                std::set<std::string> uniqueNodes;
                for (const auto& elem : elements) {
                    uniqueNodes.insert(elem->Node1);
                    uniqueNodes.insert(elem->Node2);
                }
                std::cout << "Nodes in the circuit: ";
                for (const auto& node : uniqueNodes) std::cout << node << " ";
                std::cout << std::endl;
            } else if (std::regex_match(input, match, showAllElements)) {
                std::cout << "All elements in the circuit:" << std::endl;
                for (const auto& elem : elements) {
                    std::cout << elem->name << " (" << elem->Node1 << ", " << elem->Node2 << ")" << std::endl;
                }

            } else if (std::regex_match(input, match, showElements)) {
                std::smatch smatch;
                std::string name = "" + std::string(match[1]);
                std::cout << "Elements in the circuit:" << std::endl;
                for (const auto& elem : elements) {
                    if(elem->name[0] == name[0])
                        std::cout << elem->name << " (" << elem->Node1 << ", " << elem->Node2 << ")" << std::endl;
                }

            }

            else if (std::regex_match(input, match, renameNode)) {
                std::string oldName = match[1];
                std::string newName = match[2];
                bool newNameExists = std::any_of(elements.begin(), elements.end(), [&](const auto& e) { return e->Node1 == newName || e->Node2 == newName; });
                if (newNameExists) throw SameNodeName(newName);
                bool oldNameFound = false;
                for (auto& elem : elements) {
                    if (elem->Node1 == oldName) { elem->Node1 = newName; oldNameFound = true; }
                    if (elem->Node2 == oldName) { elem->Node2 = newName; oldNameFound = true; }
                }
                if (!oldNameFound) throw OldNodeNotFound(oldName);
                std::cout << "Node renamed from " << oldName << " to " << newName << std::endl;
            } else {
                std::cout << "Error: Syntax error" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
    return true;
}

void OutputFile() {
    std::ofstream out(FileName);
    if (!out) {
        std::cerr << "Error: Could not open output file." << std::endl;
        return;
    }

    auto powToSuffix = [](int pow) -> std::string {
        switch (pow) {
            case 3: return "k";
            case 6: return "Mega";
            case -6: return "u";
            case -9: return "n";
            case -12: return "p";
            default: return "";
        }
    };

    // Replace ground node names with "0"
    for (auto& elem : elements) {
        if (std::find(GroundNodes.begin(), GroundNodes.end(), elem->Node1) != GroundNodes.end()) {
            elem->Node1 = "0";
        }
        if (std::find(GroundNodes.begin(), GroundNodes.end(), elem->Node2) != GroundNodes.end()) {
            elem->Node2 = "0";
        }
    }

    // Write elements
    for (const auto& elem : elements) {
        if (auto r = dynamic_cast<ResistorFile*>(elem.get())) {
            out << r->name << " " << r->Node1 << " " << r->Node2 << " " << r->value << powToSuffix(r->pow) << std::endl;
        } else if (auto c = dynamic_cast<CapacitorFile*>(elem.get())) {
            out << c->name << " " << c->Node1 << " " << c->Node2 << " " << c->value << powToSuffix(c->pow) << std::endl;
        } else if (auto l = dynamic_cast<InductorFile*>(elem.get())) {
            out << l->name << " " << l->Node1 << " " << l->Node2 << " " << l->value << powToSuffix(l->pow) << std::endl;
        } else if (auto d = dynamic_cast<DiodeFile*>(elem.get())) {
            out << d->name << " " << d->Node1 << " " << d->Node2 << " " << d->type << std::endl;
        } else if (auto v = dynamic_cast<DCVoltageSourceFile*>(elem.get())) {
            out << v->name << " " << v->Node1 << " " << v->Node2 << " " << v->value << std::endl;
        } else if (auto i = dynamic_cast<DCCurrentSourceFile*>(elem.get())) {
            out << i->name << " " << i->Node1 << " " << i->Node2 << " " << i->value << std::endl;
        } else if (auto vac = dynamic_cast<ACVoltageSourceFile*>(elem.get())) {
            out << vac->name << " " << vac->Node1 << " " << vac->Node2 << " SINE(" << vac->value << " " << vac->frequency << " " << vac->phase << ")" << std::endl;
        } else if (auto vccs = dynamic_cast<VCCSFile*>(elem.get())) {
            out << vccs->name << " " << vccs->Node1 << " " << vccs->Node2 << " " << vccs->ctrlNode1 << " " << vccs->ctrlNode2 << " " << vccs->gain << std::endl;
        } else if (auto vcvs = dynamic_cast<VCVSFile*>(elem.get())) {
            out << vcvs->name << " " << vcvs->Node1 << " " << vcvs->Node2 << " " << vcvs->ctrlNode1 << " " << vcvs->ctrlNode2 << " " << vcvs->gain << std::endl;
        }else if (auto deltavolt = dynamic_cast<DeltaVoltageFile*>(elem.get())) {
            out << deltavolt->name << " " << deltavolt->Node1 << " " << deltavolt->Node2 << std::endl;
        }else if (auto deltacurr = dynamic_cast<DeltaCurrentFile*>(elem.get())) {
            out << deltacurr->name << " " << deltacurr->Node1 << " " << deltacurr->Node2 << std::endl;
        }
    }

    // Write analysis command
    if (analysis->Type == "DCSweep") {
        auto* dc = dynamic_cast<DCSweepFile*>(analysis.get());
        out << ".DC " << dc->SourceName << " " << dc->startVal << " " << dc->endVal << " " << dc->Increment << std::endl;
    } else if (analysis->Type == "Transient") {
        auto* tr = dynamic_cast<TransientFile*>(analysis.get());
        out << ".TRAN " << tr->timeStep << " " << tr->endTime;
        if (tr->startTime != 0.0) out << " " << tr->startTime;
        // The fourth parameter in .TRAN is max timestep, represented by timeStart in the class
        if (tr->timeStart != 0.0) out << " " << tr->timeStart;
        out << std::endl;
    } else if (analysis->Type == "OP") {
        out << ".OP" << std::endl;
    }

    out << ".END" << std::endl;
    out.close();
    std::cout << "Netlist written to " << FileName << std::endl;
}


// --- Main Application ---
int main() {

    try {
        if (InputHandler()) {
                OutputFile();

        }
        else if(OpenOrNew) {
            Controller controller;
            controller.start(FileName);
        }
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "\nApplication finished." << std::endl;
    return 0;
}