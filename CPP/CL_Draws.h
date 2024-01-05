#pragma once
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <vector>
#include <set>
#include <chrono>


template <typename T>
using Matrix = std::vector<std::vector<T>>;

// storing all computed probabilities and dead ends
extern std::unordered_map<std::bitset<128>, Matrix<double>> computedProbabilities;
extern std::unordered_map<std::bitset<128>, bool> computedDeadEnds;

// Return value loadProbabilities function
struct loadedValue {
    bool SavedValue = false;
    bool DeadEnd = false;
};

struct iD {
    std::vector<uint_fast16_t> key;
    std::vector<uint_fast16_t> rowOrder;
    std::vector<uint_fast16_t> columnOrder;
};

template <typename T>
void printMatrix (const Matrix<T>& matrix);

template <typename T>
void sortMatrix(const Matrix<T>& matrix_input,
                const std::vector<uint_fast16_t>& rowOrder,
                const std::vector<uint_fast16_t>& columnOrder,
                Matrix<T>& matrix_output,
                bool inverse = false) noexcept;

void generateUnsortedId(const Matrix<bool>& matrix,
                        const std::vector<uint_fast16_t>& order,
                        const bool rowMode,
                        std::vector<std::pair<uint_fast16_t, uint_fast16_t>>& id) noexcept;

void generateSortedId(const Matrix<bool>& compatibilityMatrix,
                      iD& idOut) noexcept;

std::bitset<128> idGenerate(const std::vector<uint_fast16_t>& id) noexcept;

loadedValue loadProbabilities (const iD& id,
                               Matrix<double>& probabilities) noexcept;

void saveProbabilities(const iD& id,
                       Matrix<double>& probabilities,
                       const bool deadEnd) noexcept;

bool computeProbabilities(const Matrix<bool>& compatibilityMatrix,
                          Matrix<double>& probabilities,
                          int unmatchedRunnerUp=-1) noexcept;


void getProbabilities(const Matrix<std::string>& winners, const Matrix<std::string>& runnersUp);
