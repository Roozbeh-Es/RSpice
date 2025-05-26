#include "Vector.h"

#include <iomanip>

Vector::Vector(int s) : size(s) {
    if (s < 0) {
        throw std::invalid_argument("Vector size cannot be negative,");
    }
    data.resize(s, 0.0);
}

Vector::Vector(const Vector &other) :data(other.data), size(other.size) {
}

Vector& Vector::operator=(const Vector& other) {
    if (this != &other) { // != checks two pointers not the objects themselves
        size = other.size;
        data = other.data;
    }
    return *this;
}

double& Vector::operator[](int i) {
    if(i<0 || i>=size) {
        throw std::out_of_range("Vector index out of bounds");
    }
    return data[i];
}

const double& Vector::operator[](int i) const {
    if(i<0 || i>=size) {
        throw std::out_of_range("Vector index out of bounds");
    }
    return data[i];
}

int Vector::getSize() const {
    return size;
}

void Vector::resize(int newSize) {
    if (newSize < 0) {
        throw std::invalid_argument("Vector size cannot be negative");
    }
    size = newSize;
    data.resize(size, 0.0);
}

void Vector::zero() {
    std::fill(data.begin(), data.end(), 0.0);
}

void Vector::print() const {
    std::cout << "[";
    for (int i = 0; i < size; i++) {
        std::cout << std::fixed << std::setprecision(6) << data[i];
        if (i < size - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

double* Vector::getRawDataPointer() {
    return data.data(); // .data() returns a pointer to the first element
}

const double* Vector::getRawDataPointer() const {
    return data.data();
}
