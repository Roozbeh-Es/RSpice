#include <iostream>
#include <stdexcept>
#include <string>
#include <regex>
#include <fstream>
#include <vector>
#include <memory>

class Element {
public:
    std::string name;
    std::string Node1;
    std::string Node2;
    Element(const std::string& n, const std::string& n1, const std::string& n2)
        : name(n), Node1(n1), Node2(n2) {}
    virtual ~Element() = default;
};
class Analysis{
    public:
    std::string Type;
    virtual ~Analysis() = default;
};
class DCSweep: public Analysis{
    public:
    std::string SourceName;
    double startVal, endVal, Increment;
    DCSweep(const std::string& source, double start, double end, double inc)
        : SourceName(source), startVal(start), endVal(end), Increment(inc) {
        Type = "DCSweep";
    }
};
class Transient: public Analysis{
    public:
    std::string SourceName;
    double startTime, endTime, timeStep, timeStart;
    Transient(const std::string& source, double start, double end, double step, double startTime = 0)
        : SourceName(source), startTime(start), endTime(end), timeStep(step), timeStart(startTime) {
        Type = "Transient";
    }
};
class OP: public Analysis{
    public:
    std::string SourceName;
    OP(const std::string& source)
        : SourceName(source) {
        Type = "OP";
    }
};


class Resistor : public Element {
public:
    double value;
    int pow;
    Resistor(const std::string& n, const std::string& n1, const std::string& n2, double v, int p = 0)
        : Element(n, n1, n2), value(v), pow(p) {}
};

class Capacitor : public Element {
public:
    double value;
    int pow;
    Capacitor(const std::string& n, const std::string& n1, const std::string& n2, double v, int p = 0)
        : Element(n, n1, n2), value(v), pow(p) {}
};

class Inductor : public Element {
public:
    double value;
    int pow;
    Inductor(const std::string& n, const std::string& n1, const std::string& n2, double v, int p = 0)
        : Element(n, n1, n2), value(v), pow(p) {}
};

class Diode : public Element {
public:
    std::string type;
    Diode(const std::string& n, const std::string& n1, const std::string& n2, const std::string& t)
        : Element(n, n1, n2), type(t) {}
};

class DCVoltageSource : public Element {
public:
    double value;
    DCVoltageSource(const std::string& n, const std::string& n1, const std::string& n2, double v)
        : Element(n, n1, n2), value(v) {}
};

class DCCurrentSource : public Element {
public:
    double value;
    DCCurrentSource(const std::string& n, const std::string& n1, const std::string& n2, double v)
        : Element(n, n1, n2), value(v) {}
};

class ACVoltageSource : public Element {
public:
    double value;
    double frequency;
    double phase;
    ACVoltageSource(const std::string& n, const std::string& n1, const std::string& n2, double v, double f, double p)
        : Element(n, n1, n2), value(v), frequency(f), phase(p) {}
};

class VCCS : public Element {
public:
    std::string ctrlNode1;
    std::string ctrlNode2;
    double value;
    double gain;
    VCCS(const std::string& n, const std::string& n1, const std::string& n2,
         const std::string& cn1, const std::string& cn2, double v, double g)
        : Element(n, n1, n2), ctrlNode1(cn1), ctrlNode2(cn2), value(v), gain(g) {}
};

class VCVS : public Element {
public:
    std::string ctrlNode1;
    std::string ctrlNode2;
    double value;
    double gain;
    VCVS(const std::string& n, const std::string& n1, const std::string& n2,
         const std::string& cn1, const std::string& cn2, double v, double g)
        : Element(n, n1, n2), ctrlNode1(cn1), ctrlNode2(cn2), value(v), gain(g) {}
};

class CCCS : public Element {
public:
    std::string ctrlNode1;
    std::string ctrlNode2;
    double value;
    double gain;
    CCCS(const std::string& n, const std::string& n1, const std::string& n2,
         const std::string& cn1, const std::string& cn2, double v, double g)
        : Element(n, n1, n2), ctrlNode1(cn1), ctrlNode2(cn2), value(v), gain(g) {}
};

class CCVS : public Element {
public:
    std::string ctrlNode1;
    std::string ctrlNode2;
    double value;
    double gain;
    CCVS(const std::string& n, const std::string& n1, const std::string& n2,
         const std::string& cn1, const std::string& cn2, double v, double g)
        : Element(n, n1, n2), ctrlNode1(cn1), ctrlNode2(cn2), value(v), gain(g) {}
};
std::vector<std::unique_ptr<Element>> elements;
std::vector<std::string> GroundNodes;
std::string FileName;
std::unique_ptr<Analysis> analysis = nullptr;
class OpenError: std::exception{
    const char* what() const noexcept override {
        return "An error occurred in InputHandler";
    }
};
class NewError: std::exception{
    const char* what() const noexcept override {
        return "An error occurred in InputHandler";
    }
};
class ResValueError: std::exception{
    const char* what() const noexcept override {
        return "Error: Resistance cannot be zero or negative";
    }
};
class ResSameError: public std::exception {
    std::string message;
public:
    explicit ResSameError(const std::string& name) : message("Error: Resistor " + name + " already exists in the circuit") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class CapValueError: public std::exception {
    const char* what() const noexcept override {
        return "Error: Capacitance cannot be zero or negative";
    }
};
class CapSameError: public std::exception {
    std::string message;
public:
    explicit CapSameError(const std::string& name) : message("Error: Capacitor " + name + " already exists in the circuit") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class IndValueError: public std::exception {
    const char* what() const noexcept override {
        return "Error: Inductance value cannot be zero or negative";
    }
};
class IndSameError: public std::exception {
    std::string message;
public:
    explicit IndSameError(const std::string& name) : message("Error: Inductor " + name + " already exists in the circuit") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class DioSameError: public std::exception {
    std::string message;
public:
    explicit DioSameError(const std::string& name) : message("Error: Diode " + name + " already exists in the circuit") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class DioModelError: public std::exception {
    std::string message;
public:
    explicit DioModelError(const std::string& name) : message("Error: Diode model " + name + " is not supported") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class DCVoltSameError: public std::exception {
    std::string message;
public:
    explicit DCVoltSameError(const std::string& name) : message("Error: DC Voltage Source " + name + " already exists in the circuit") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class DCCurrentSameError: public std::exception {
    std::string message;
public:
    explicit DCCurrentSameError(const std::string& name) : message("Error: DC Current Source " + name + " already exists in the circuit") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class ACVoltSameError: public std::exception {
    std::string message;
public:
    explicit ACVoltSameError(const std::string& name) : message("Error: AC Voltage Source " + name + " already exists in the circuit") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class ResistorNotFoundError: public std::exception {
    std::string message;
public:
    explicit ResistorNotFoundError(const std::string& name) : message("Error: Cannot delete Resistor; component not found") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class CapacitorNotFoundError: public std::exception {
    std::string message;
public:
    explicit CapacitorNotFoundError(const std::string& name) : message("Error: Cannot delete Capacitor; component not found") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class InductorNotFoundError: public std::exception {
    std::string message;
public:
    explicit InductorNotFoundError(const std::string& name) : message("Error: Cannot delete Inductor; component not found") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class DiodeNotFoundError: public std::exception {
    std::string message;
public:
    explicit DiodeNotFoundError(const std::string& name) : message("Error: Cannot delete Diode; component not found") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class GroundNodeNotFoundError: public std::exception {
    std::string message;
public:
    explicit GroundNodeNotFoundError(const std::string& name) : message("Error: Cannot delete Ground Node; component not found") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class VoltageSourceNotFoundError: public std::exception {
    std::string message;
public:
    explicit VoltageSourceNotFoundError(const std::string& name) : message("Error: Cannot delete Voltage Source; component not found") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
class CurrentSourceNotFoundError: public std::exception {
    std::string message;
public:
    explicit CurrentSourceNotFoundError(const std::string& name) : message("Error: Cannot delete Current Source; component not found") {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
void Menu(){
menu_start:
    std::cout << "Welcome to the RASpice Menu!" << std::endl;
    std::cout << "1. Open File" << std::endl;
    std::cout << "2. New File" << std::endl;
    std::cout << "3. Exit" << std::endl;
    std::cout << "Please select an option: ";
    int choice;
    std::cin >> choice;
    if(choice == 1){
        std::ifstream file("../settings/recents.txt");
        if( file.peek() == std::ifstream::traits_type::eof()) {
            std::cout << "No recent files found." << std::endl;
        }else{
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
        if(!file2){
            throw OpenError();
        }
        else{
            std::cout << "File opened successfully." << std::endl;
        }
        FileName = filename;
        return;
        
    }
    else if(choice == 2){
        std::cout << "Enter the new file name: ";
        std::string filename;
        std::cin >> filename;
        std::ofstream file(filename);

        if(!file){
            std::cout << "Error: Could not create file." << std::endl;
            goto menu_start;
        }
        else{
            std::cout << "File created successfully." << std::endl;
            std::ofstream recent("../settings/recents.txt", std::ios::app);
            recent << filename << "\n";
            FileName = filename;
        }
        return;
    }
}

void InputHandler(){
    Menu();
    std::regex addResistor(R"(add R(\S+) (\S+) (\S+) (\S+))");
    std::regex addCapacitor(R"(add C(\S+) (\S+) (\S+) (\S+))");
    std::regex addInductor(R"(add L(\S+) (\S+) (\S+) (\S+))");
    std::regex addDiode(R"(add D(\S+) (\S+) (\S+) (\S+))");
    std::regex addGND(R"(add GND (\S+))");
    std::regex addDCVoltageSource(R"(add V(\S+) (\S+) (\S+) ([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?))");
    std::regex addDCCurrentSource(R"(add I(\S+) (\S+) (\S+) ([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?))");
    std::regex addACVoltageSource(R"(add V(\S+) (\S+) (\S+) SIN\((\S+) (\S+) (\S+)\))");
    std::regex addACCurrentSource(R"(add I(\S+) (\S+) (\S+) SIN\((\S+) (\S+) (\S+)\))");
    std::regex addPulseVoltageSource(R"(add V(\S+) (\S+) (\S+) PULSE (\S+) \((\S+)\))");
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
    std::regex addTransient(R"(^\.TRAN\s+(\d+(?:\.\d+)?)\s+(\d+(?:\.\d+)?)(?:\s+(\d+(?:\.\d+)?))?(?:\s+(\d+(?:\.\d+)?))?$)");
    std::regex addDCSweep(R"(^\.DC\s+(\S+)\s+([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s+([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s+([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)$)");
    std::regex addOP(R"(^\.OP\s*$)");
    std::cin.get();
    while(true){
        try{
        std::string input;
        std::getline(std::cin, input);
        if(input == "END"){
            if(GroundNodes.empty()) {
                throw std::runtime_error("Error: No ground nodes defined");
            }
            if(analysis == nullptr) {
                throw std::runtime_error("Error: No analysis defined");
            }
            std::cout << "Getting Things Ready..." << std::endl;
            break;
        }
        if(std::regex_match(input, addResistor)){
            std::smatch match;
            std::regex_search(input, match, addResistor);
            std::string name = match[1];
            std::string valueStr = match[4];
            int pow = 0;
            std::smatch suffixMatch;
            std::regex suffixRegex(R"(^([-+]?\d*\.?\d+)([uUnNkK]|Mega|MEGA|n|N|p|P)?$)");
            if (std::regex_match(valueStr, suffixMatch, suffixRegex)) {
                valueStr = suffixMatch[1];
                std::string suffix = suffixMatch[2];
                if (!suffix.empty()) {
                    valueStr = valueStr.substr(0, valueStr.size());
                }
                if (suffix == "u" || suffix == "U") pow = -6;
                else if (suffix == "n" || suffix == "N") pow = -9;
                else if (suffix == "p" || suffix == "P") pow = -12;
                else if (suffix == "k" || suffix == "K") pow = 3;
                else if (suffix == "Mega" || suffix == "MEGA") pow = 6;
            }
            name.insert(0, "R");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                throw ResSameError(name);
            }
            std::regex numberRegex(R"(^[-+]?\d*\.?\d+([eE][-+]?\d+)?$)");
            if (!std::regex_match(valueStr, numberRegex)) {
                throw ResValueError();
            }
            double value = std::stod(valueStr);
            if (value <= 0) {
                throw ResValueError();
            }
            elements.push_back(std::make_unique<Resistor>(name, match[2], match[3], value));
        }
        else if(std::regex_match(input, addCapacitor)) {
            std::smatch match;
            std::regex_search(input, match, addCapacitor);
            std::string name = match[1];
            std::string valueStr = match[4];
            int pow = 0;
            std::smatch suffixMatch;
            std::regex suffixRegex(R"(^([-+]?\d*\.?\d+)([uUnNkK]|Mega|MEGA|n|N|p|P)?$)");
            if (std::regex_match(valueStr, suffixMatch, suffixRegex)) {
                valueStr = suffixMatch[1];
                std::string suffix = suffixMatch[2];
                if (!suffix.empty()) {
                    valueStr = valueStr.substr(0, valueStr.size());
                }
                if (suffix == "u" || suffix == "U") pow = -6;
                else if (suffix == "n" || suffix == "N") pow = -9;
                else if (suffix == "p" || suffix == "P") pow = -12;
                else if (suffix == "k" || suffix == "K") pow = 3;
                else if (suffix == "Mega" || suffix == "MEGA") pow = 6;
            }
            name.insert(0, "C");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                throw CapSameError(name);
            }
            std::regex numberRegex(R"(^[-+]?\d*\.?\d+([eE][-+]?\d+)?$)");
            if (!std::regex_match(valueStr, numberRegex)) {
                throw CapValueError();
            }
            double value = std::stod(valueStr);
            if (value <= 0) {
                throw CapValueError();
            }
            elements.push_back(std::make_unique<Capacitor>(name, match[2], match[3], value));
        }
        else if(std::regex_match(input, addInductor)) {
            std::smatch match;
            std::regex_search(input, match, addInductor);
            std::string name =match[1];
            std::string valueStr = match[4];
            int pow = 0;
            std::smatch suffixMatch;
            std::regex suffixRegex(R"(^([-+]?\d*\.?\d+)([uUnNkK]|Mega|MEGA|n|N|p|P)?$)");
            if (std::regex_match(valueStr, suffixMatch, suffixRegex)) {
                valueStr = suffixMatch[1];
                std::string suffix = suffixMatch[2];
                if (!suffix.empty()) {
                    valueStr = valueStr.substr(0, valueStr.size());
                }
                if (suffix == "u" || suffix == "U") pow = -6;
                else if (suffix == "n" || suffix == "N") pow = -9;
                else if (suffix == "p" || suffix == "P") pow = -12;
                else if (suffix == "k" || suffix == "K") pow = 3;
                else if (suffix == "Mega" || suffix == "MEGA") pow = 6;
            }
            name.insert(0, "I");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                throw IndSameError(name);
            }
            std::regex numberRegex(R"(^[-+]?\d*\.?\d+([eE][-+]?\d+)?$)");
            if (!std::regex_match(valueStr, numberRegex)) {
                throw IndValueError();
            }
            double value = std::stod(valueStr);
            if (value <= 0) {
                throw IndValueError();
            }
            elements.push_back(std::make_unique<Inductor>(name, match[2], match[3], value));
        }
        else if(std::regex_match(input, addDiode)) {
            std::smatch match;
            std::regex_search(input, match, addDiode);
            std::string name = match[1];
            name.insert(0, "D");
            std::string type = match[4];
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                throw DioSameError(name);
            }
            if(type != "D" && type != "Z")
                throw DioModelError(type);
            elements.push_back(std::make_unique<Diode>(name, match[2], match[3], type));
        }
        else if(std::regex_match(input, addGND)) {
            std::smatch match;
            std::regex_search(input, match, addGND);
            std::string node = match[1];
            GroundNodes.push_back(node);
        }
        else if(std::regex_match(input, addDCVoltageSource)) {
            std::smatch match;
            std::regex_search(input, match, addDCVoltageSource);
            std::string name = match[1];
            name.insert(0, "V");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                throw DCVoltSameError(name);
            }
            double value = std::stod(match[4]);
            elements.push_back(std::make_unique<DCVoltageSource>(name, match[2], match[3], value));
        }
        else if(std::regex_match(input, addDCCurrentSource)) {
            std::smatch match;
            std::regex_search(input, match, addDCCurrentSource);
            std::string name = match[1];
            name.insert(0, "I");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                throw DCCurrentSameError(name);
            }
            double value = std::stod(match[4]);
            elements.push_back(std::make_unique<DCCurrentSource>(name, match[2], match[3], value));
        }
        else if(std::regex_match(input, addACVoltageSource)) {
            std::smatch match;
            std::regex_search(input, match, addACVoltageSource);
            std::string name = match[1];
            name.insert(0, "V");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                throw ACVoltSameError(name);
            }
            double value = std::stod(match[4]);
            double frequency = std::stod(match[5]);
            double phase = std::stod(match[6]);
            elements.push_back(std::make_unique<ACVoltageSource>(name, match[2], match[3], value, frequency, phase));
        }
        else if(std::regex_match(input, addVCCS)) {
            std::smatch match;
            std::regex_search(input, match, addVCCS);
            std::string name = match[1];
            name.insert(0, "G");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                throw DCVoltSameError(name);
            }
            double value = std::stod(match[6]);
            double gain = std::stod(match[7]);
            elements.push_back(std::make_unique<VCCS>(name, match[2], match[3], match[4], match[5], value, gain));
        }
        else if(std::regex_match(input, addVCVS)) {
            std::smatch match;
            std::regex_search(input, match, addVCVS);
            std::string name = match[1];
            name.insert(0, "E");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                throw DCVoltSameError(name);
            }
            double value = std::stod(match[6]);
            double gain = std::stod(match[7]);
            elements.push_back(std::make_unique<VCVS>(name, match[2], match[3], match[4], match[5], value, gain));
        }
        else if(std::regex_match(input, addCCCS)) {
            std::smatch match;
            std::regex_search(input, match, addCCCS);
            std::string name = match[1];
            name.insert(0, "F");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                throw DCCurrentSameError(name);
            }
            double value = std::stod(match[6]);
            double gain = std::stod(match[7]);
            elements.push_back(std::make_unique<CCCS>(name, match[2], match[3], match[4], match[5], value, gain));

        }
        else if(std::regex_match(input, deleteResistor)){
            std::smatch match;
            std::regex_search(input, match, deleteResistor);
            std::string name = match[1];
            name.insert(0, "R");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                elements.erase(it);
            }
            else {
                throw ResistorNotFoundError(name);
            }
        }
        else if(std::regex_match(input, deleteCapacitor)){
            std::smatch match;
            std::regex_search(input, match, deleteCapacitor);
            std::string name = match[1];
            name.insert(0, "C");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                elements.erase(it);
            }
            else {
                throw CapacitorNotFoundError(name);
            }
        }
        else if(std::regex_match(input, deleteInductor)){
            std::smatch match;
            std::regex_search(input, match, deleteInductor);
            std::string name = match[1];
            name.insert(0, "I");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                elements.erase(it);
            }
            else {
                throw InductorNotFoundError(name);
            }
        }
        else if(std::regex_match(input, deleteDiode)){
            std::smatch match;
            std::regex_search(input, match, deleteDiode);
            std::string name = match[1];
            name.insert(0, "D");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                elements.erase(it);
            }
            else {
                throw DiodeNotFoundError(name);
            }
        }
        else if(std::regex_match(input, deleteGND)){
            std::smatch match;
            std::regex_search(input, match, deleteGND);
            std::string node = match[1];
            auto it = std::find(GroundNodes.begin(), GroundNodes.end(), node);
            if(it != GroundNodes.end()) {
                GroundNodes.erase(it);
            } else {
                throw GroundNodeNotFoundError(node);
            }
        }
        else if(std::regex_match(input, deleteVoltageSource)){
            std::smatch match;
            std::regex_search(input, match, deleteVoltageSource);
            std::string name = match[1];
            name.insert(0, "V");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                elements.erase(it);
            }
            else {
                throw VoltageSourceNotFoundError(name);
            }
        }
        else if(std::regex_match(input, deleteCurrentSource)){
            std::smatch match;
            std::regex_search(input, match, deleteCurrentSource);
            std::string name = match[1];
            name.insert(0, "I");
            auto it = std::find_if(elements.begin(), elements.end(), [&name](const std::unique_ptr<Element>& elem) {
                return elem->name == name;
            });
            if(it != elements.end()) {
                elements.erase(it);
            }
            else {
                throw CurrentSourceNotFoundError(name);
            }
        }
        else if(std::regex_match(input, addTransient)){
            std::smatch match;
            std::regex_search(input, match, addTransient);
            double tstep = std::stod(match[1]);
            double tstop = std::stod(match[2]);
            double tstart = match[3].matched ? std::stod(match[3]) : 0.0;
            double tmaxstep = match[4].matched ? std::stod(match[4]) : 0.0;
            analysis = std::make_unique<Transient>("", tstart, tstop, tstep, tmaxstep);
        }
        else if(std::regex_match(input, addDCSweep)){
            std::smatch match;
            std::regex_search(input, match, addDCSweep);
            std::string source = match[1];
            double start = std::stod(match[2]);
            double end = std::stod(match[3]);
            double inc = std::stod(match[4]);
            analysis = std::make_unique<DCSweep>(source, start, end, inc);
        }
        else if(std::regex_match(input, addOP)){
            std::smatch match;
            std::regex_search(input, match, addOP);
            std::string source = match[1];
            analysis = std::make_unique<OP>("");  
        }
        else {
            std::cout << "Error: Syntax error" << std::endl;
            continue;
        }
    }
    catch(const std::exception& e) {
            std::cout << e.what() << std::endl;
            continue;
        }
    }
}
void OutputFile(){
    std::ofstream out(FileName);

    auto powToSuffix = [](int pow) -> std::string {
        switch (pow) {
            case 3:  return "k";
            case 6:  return "Mega";
            case -6: return "u";
            case -9: return "n";
            case -12: return "p";
            default: return "";
        }
    };
    if (!out) {
        std::cerr << "Error: Could not open output file." << std::endl;
        return;
    }
    for (auto& elem : elements) {
        if (std::find(GroundNodes.begin(), GroundNodes.end(), elem->Node1) != GroundNodes.end()) {
            elem->Node1 = "0";
        }
        if (std::find(GroundNodes.begin(), GroundNodes.end(), elem->Node2) != GroundNodes.end()) {
            elem->Node2 = "0";
        }
    }
    for (const auto& elem : elements) {
        if (auto r = dynamic_cast<Resistor*>(elem.get())) {
            out << r->name << " " << r->Node1 << " " << r->Node2 << " " << r->value;
            if (r->pow != 0) out << powToSuffix(r->pow);
            out << std::endl;
        } else if (auto c = dynamic_cast<Capacitor*>(elem.get())) {
            out << c->name << " " << c->Node1 << " " << c->Node2 << " " << c->value;
            if (c->pow != 0) out << powToSuffix(c->pow);
            out << std::endl;
        } else if (auto l = dynamic_cast<Inductor*>(elem.get())) {
            out << l->name << " " << l->Node1 << " " << l->Node2 << " " << l->value;
            if (l->pow != 0) out << powToSuffix(l->pow);
            out << std::endl;
        } else if (auto d = dynamic_cast<Diode*>(elem.get())) {
            out << d->name << " " << d->Node1 << " " << d->Node2 << " " << d->type << std::endl;
        } else if (auto v = dynamic_cast<DCVoltageSource*>(elem.get())) {
            out << v->name << " " << v->Node1 << " " << v->Node2 << " DC " << v->value << std::endl;
        } else if (auto i = dynamic_cast<DCCurrentSource*>(elem.get())) {
            out << i->name << " " << i->Node1 << " " << i->Node2 << " DC " << i->value << std::endl;
        } else if (auto vac = dynamic_cast<ACVoltageSource*>(elem.get())) {
            out << vac->name << " " << vac->Node1 << " " << vac->Node2
                << " SIN(" << vac->value << " " << vac->frequency << " " << vac->phase << ")" << std::endl;
        } else if (auto vccs = dynamic_cast<VCCS*>(elem.get())) {
            out << vccs->name << " " << vccs->Node1 << " " << vccs->Node2 << " "
                << vccs->ctrlNode1 << " " << vccs->ctrlNode2 << " " << vccs->gain << std::endl;
        } else if (auto vcvs = dynamic_cast<VCVS*>(elem.get())) {
            out << vcvs->name << " " << vcvs->Node1 << " " << vcvs->Node2 << " "
                << vcvs->ctrlNode1 << " " << vcvs->ctrlNode2 << " " << vcvs->gain << std::endl;
        } else if (auto cccs = dynamic_cast<CCCS*>(elem.get())) {
            out << cccs->name << " " << cccs->Node1 << " " << cccs->Node2 << " "
                << cccs->ctrlNode1 << " " << cccs->ctrlNode2 << " " << cccs->gain << std::endl;
        } else if (auto ccvs = dynamic_cast<CCVS*>(elem.get())) {
            out << ccvs->name << " " << ccvs->Node1 << " " << ccvs->Node2 << " "
                << ccvs->ctrlNode1 << " " << ccvs->ctrlNode2 << " " << ccvs->gain << std::endl;
        }
    }



    if (analysis->Type == "DCSweep") {
            auto* dc = dynamic_cast<DCSweep*>(analysis.get());
            out << ".DC " << dc->SourceName << " " << dc->startVal << " " << dc->endVal << " " << dc->Increment << std::endl;
            out << ".DC " << dc->SourceName << " " << dc->startVal << " " << dc->endVal << " " << dc->Increment << std::endl;
        } else if (analysis->Type == "Transient") {
            auto* tr = dynamic_cast<Transient*>(analysis.get());
            out << ".TRAN " << tr->timeStep << " " << tr->endTime;
            if (tr->timeStart != 0.0) out << " " << tr->timeStart;
            if (tr->timeStart != 0.0) out << " " << tr->timeStart;
            out << std::endl;
        } else if (analysis->Type == "OP") {
            out << ".OP" << std::endl;
        }
    

    out << ".END" << std::endl;
    out.close();
    std::cout << "Netlist written to " << FileName << std::endl;
}
int main(){
    try {
        InputHandler();
        OutputFile();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
