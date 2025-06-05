#include "Controller.h"
#include "Simulator.h"
#include <iostream>
#include <string>
#include <stdexcept>

Controller::Controller() {
    std::cout << "RASpice Simulator application started." << std::endl;
}

int Controller::start() {
    std::string netlistFilePath = getFilePathFromUser();

    if (netlistFilePath.empty()) {
        std::cerr << "Error: No file path provided. Exiting." << std::endl;
        return 1;
    }

    try {
        Simulator simulator(netlistFilePath);

        if (simulator.run()) {
            std::cout << "\nApplication finished successfully." << std::endl;
            return 0;
        } else {
            std::cerr << "\nApplication failed." << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "\nFATAL ERROR: An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    }
}

std::string Controller::getFilePathFromUser() {
    std::string filePath;
    std::cout << "Please enter the path to the netlist file: ";
    std::getline(std::cin, filePath);

    return filePath;
}