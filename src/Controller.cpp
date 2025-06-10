    #include "Controller.h"
    #include "Simulator.h"
    #include <iostream>
    #include <string>
    #include <stdexcept>

    Controller::Controller() {
        std::cout << "RASpice Simulator application started." << std::endl;
    }

    void Controller::start(std::string netListFilePath) {

        if (netListFilePath.empty()) {
            throw std::runtime_error("Error: No file path provided. Exiting.");
        }

        try {
            Simulator simulator(netListFilePath);

            if (simulator.run()) {
                std::cout << "\nApplication finished successfully." << std::endl;
            } else {
                throw std::runtime_error("\nApplication failed to start.");
            }
        } catch (const std::exception& e) {
            throw std::runtime_error(e.what());
        }
    }

    /*std::string Controller::getFilePathFromUser() {
        std::string filePath;
        std::cout << "Please enter the path to the netlist file: ";
        std::getline(std::cin, filePath);

        return filePath;
    }*/