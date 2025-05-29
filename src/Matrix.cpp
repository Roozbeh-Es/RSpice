#include <iomanip>
#include <Matrix.h>

Matrix::Matrix(int r, int c) : rows_(r), cols_(c) {
    if (r < 0 || c < 0) {
        throw std::invalid_argument("Matrix dimensions cannot be negative.");
    }
    data_.resize(rows_);
    for (int i = 0; i < rows_; i++) {
        data_[i].resize(cols_, 0.0);
    }
}

Matrix::Matrix(const Matrix &other) : rows_(other.rows_), cols_(other.cols_), data_(other.data_) {
}

Matrix& Matrix::operator=(const Matrix& other) {
    if (this != &other) {
        rows_ = other.rows_;
        cols_ = other.cols_;
        data_ = other.data_;
    }
    return *this;
}

double &Matrix::operator()(int r, int c) {
    if (r < 0 || r >= rows_ || c < 0 || c >= cols_) {
        throw std::out_of_range("Matrix indices out of bounds.");
    }
    return data_[r][c];
}
const double &Matrix::operator()(int r, int c) const {
    if (r < 0 || r >= rows_ || c < 0 || c >= cols_) {
        throw std::out_of_range("Matrix indices out of bounds.");
    }
    return data_[r][c];
}

int Matrix::getRows() const {
    return rows_;
}

int Matrix::getCols() const {
    return cols_;
}

void Matrix::resize(int newRows, int newCols) {
    if (newRows < 0 || newCols < 0) {
        throw std::invalid_argument("Matrix dimensions cannot be negative.");
    }
    rows_ = newRows;
    cols_ = newCols;
    for (int i = 0; i < rows_; i++) {
        std:fill(data_[i].begin(), data_[i].end(), 0.0);
    }
}


void Matrix::print() const {
    for (int i = 0; i < rows_; ++i) {
        std::cout << "[";
        for (int j = 0; j < cols_; ++j) {
            std::cout << std::fixed << std::setprecision(6);
            if (j < cols_ - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }
}

void Matrix::zero() {
    for (int i = 0; i < rows_; ++i) {
        std::fill(data_[i].begin(), data_[i].end(), 0.0);
    }
}

