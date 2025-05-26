#include <iomanip>
#include <Matrix.h>

Matrix::Matrix(int r, int c) : rows(r), cols(c) {
    if (r < 0 || c < 0) {
        throw std::invalid_argument("Matrix dimensions cannot be negative.");
    }
    data.resize(rows);
    for (int i = 0; i < rows; i++) {
        data[i].resize(cols, 0.0);
    }
}

Matrix::Matrix(const Matrix &other) : rows(other.rows), cols(other.cols), data(other.data) {
}

Matrix& Matrix::operator=(const Matrix& other) {
    if (this != &other) {
        rows = other.rows;
        cols = other.cols;
        data = other.data;
    }
    return *this;
}

double &Matrix::operator()(int r, int c) {
    if (r < 0 || r >= rows || c < 0 || c >= cols) {
        throw std::out_of_range("Matrix indices out of bounds.");
    }
    return data[r][c];
}
const double &Matrix::operator()(int r, int c) const {
    if (r < 0 || r >= rows || c < 0 || c >= cols) {
        throw std::out_of_range("Matrix indices out of bounds.");
    }
    return data[r][c];
}

int Matrix::getRows() const {
    return rows;
}

int Matrix::getCols() const {
    return cols;
}

void Matrix::resize(int newRows, int newCols) {
    if (newRows < 0 || newCols < 0) {
        throw std::invalid_argument("Matrix dimensions cannot be negative.");
    }
    rows = newRows;
    cols = newCols;
    for (int i = 0; i < rows; i++) {
        std:fill(data[i].begin(), data[i].end(), 0.0);
    }
}


void Matrix::print() const {
    for (int i = 0; i < rows; ++i) {
        std::cout << "[";
        for (int j = 0; j < cols; ++j) {
            std::cout << std::fixed << std::setprecision(6);
            if (j < cols - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
}

void Matrix::zero() {
    for (int i = 0; i < rows; ++i) {
        std::fill(data[i].begin(), data[i].end(), 0.0);
    }
}

