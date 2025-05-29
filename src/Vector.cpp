#include "Vector.h"
#include <iomanip>

Vector::Vector(int s) : size_(s) {
    if (s < 0) {
        throw std::invalid_argument("Vector size cannot be negative,");
    }
    data_.resize(s, 0.0);
}

Vector::Vector(const Vector &other) :data_(other.data_), size_(other.size_) {
}

Vector& Vector::operator=(const Vector& other) {
    if (this != &other) { // != checks two pointers not the objects themselves
        size_ = other.size_;
        data_ = other.data_;
    }
    return *this;
}

double& Vector::operator()(int i) {
    if(i<0 || i>=size_) {
        throw std::out_of_range("Vector index out of bounds");
    }
    return data_[i];
}

const double& Vector::operator()(int i) const {
    if(i<0 || i>=size_) {
        throw std::out_of_range("Vector index out of bounds");
    }
    return data_[i];
}

int Vector::getSize() const {
    return size_;
}

void Vector::resize(int newSize) {
    if (newSize < 0) {
        throw std::invalid_argument("Vector size cannot be negative");
    }
    size_ = newSize;
    data_.resize(size_, 0.0);
}

void Vector::zero() {
    std::fill(data_.begin(), data_.end(), 0.0);
}

void Vector::print() const {
    std::cout << "[";
    for (int i = 0; i < size_; i++) {
        std::cout << std::fixed << std::setprecision(6) << data_[i];
        if (i < size_ - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

double* Vector::getRawDataPointer() {
    return data_.data(); // .data() returns a pointer to the first element
}

const double* Vector::getRawDataPointer() const {
    return data_.data();
}
