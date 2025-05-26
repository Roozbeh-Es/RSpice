#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream>
#include <stdexcept>

class Matrix {
private:
    std::vector<std::vector<double> > data_;
    int rows_;
    int cols_;

public:

    explicit Matrix(int rows = 0, int cols = 0);

    Matrix(const Matrix& other);

    Matrix& operator=(const Matrix& other);

    ~Matrix() = default;

    double& operator()(int r,int c);
    const double& operator()(int r,int c) const;

    int getRows() const;
    int getCols() const;

    void resize(int newRows, int newCols);

    void zero();

    void print() const;
};

#endif //MATRIX_H
