#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include <vector>

class Controller {
public:
    Controller();

    int start();

private:
    std::string getFilePathFromUser();
};

#endif // CONTROLLER_H
