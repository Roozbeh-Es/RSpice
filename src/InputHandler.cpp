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
    std::string ctrlSourceName;
    double value;
    double gain;
    CCCS(const std::string& n, const std::string& n1, const std::string& n2,
         const std::string& csn, double v, double g)
        : Element(n, n1, n2), ctrlSourceName(csn), value(v), gain(g) {}
};

class CCVS : public Element {
public:
    std::string ctrlSourceName;
    double value;
    double gain;
    CCVS(const std::string& n, const std::string& n1, const std::string& n2,
         const std::string& csn, double v, double g)
        : Element(n, n1, n2), ctrlSourceName(csn), value(v), gain(g) {}
};
std::vector<std::unique_ptr<Element>> elements;
std::vector<std::string> GroundNodes;
std::string FileName;
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
            throw NewError();
        }
        else{
            std::cout << "File created successfully." << std::endl;
            std::ofstream recent("..\\settings\\recents.txt", std::ios::app);
            
            recent << filename + "\n";
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
    std::regex addDCVoltageSource(R"(add V(\S+) (\S+) (\S+))");
    std::regex addDCCurrentSource(R"(add I(\S+) (\S+) (\S+))");
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
    while(true){
        try{
        std::string input;
        std::getline(std::cin, input);
        if(input == "exit"){
            std::cout << "Exiting..." << std::endl;
            break;
        }
        if(std::regex_match(input, addResistor)){
            std::smatch match;
            std::regex_search(input, match, addResistor);
            std::string name = match[1];
            std::string valueStr = match[4];
            int pow = 0;
            // Check for suffix and set pow accordingly
            std::smatch suffixMatch;
            // Match number followed by optional suffix (u, k, Mega, n, p)
            std::regex suffixRegex(R"(^([-+]?\d*\.?\d+)([uUnNkK]|Mega|MEGA|n|N|p|P)?$)");
            if (std::regex_match(valueStr, suffixMatch, suffixRegex)) {
                valueStr = suffixMatch[1];
                std::string suffix = suffixMatch[2];
                if (!suffix.empty()) {
                    // Remove the suffix character(s) from the end of valueStr if present
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
            // Check for suffix and set pow accordingly
            std::smatch suffixMatch;
            // Match number followed by optional suffix (u, k, Mega, n, p)
            std::regex suffixRegex(R"(^([-+]?\d*\.?\d+)([uUnNkK]|Mega|MEGA|n|N|p|P)?$)");
            if (std::regex_match(valueStr, suffixMatch, suffixRegex)) {
                valueStr = suffixMatch[1];
                std::string suffix = suffixMatch[2];
                if (!suffix.empty()) {
                    // Remove the suffix character(s) from the end of valueStr if present
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
            // Check for suffix and set pow accordingly
            std::smatch suffixMatch;
            // Match number followed by optional suffix (u, k, Mega, n, p)
            std::regex suffixRegex(R"(^([-+]?\d*\.?\d+)([uUnNkK]|Mega|MEGA|n|N|p|P)?$)");
            if (std::regex_match(valueStr, suffixMatch, suffixRegex)) {
                valueStr = suffixMatch[1];
                std::string suffix = suffixMatch[2];
                if (!suffix.empty()) {
                    // Remove the suffix character(s) from the end of valueStr if present
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
        
            
    }
    catch(const std::exception& e) {
            std::cout << e.what() << std::endl;
            continue;
        }
    }
}

int main(){
    try {
        InputHandler();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
