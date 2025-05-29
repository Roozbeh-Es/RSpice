#ifndef VECTOR_H
#define VECTOR_H
#include <vector>
#include <iostream>
#include <stdexcept>


class Vector {
private:
    std::vector<double> data_;
    int size_;

public:
    explicit Vector(int s = 0);
    Vector(const Vector& other);
    ~Vector() = default;
    Vector& operator=(const Vector& other);

    double& operator()(int i);
    const double& operator()(int i) const;

    [[nodiscard]] int getSize() const;

    void resize(int newSize);

    void zero();

    void print() const;

    [[nodiscard]] double* getRawDataPointer();
    [[nodiscard]] const double* getRawDataPointer() const;
};
#endif //VECTOR_H
