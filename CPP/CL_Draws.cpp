
#include "CL_Draws.h"

std::unordered_map<std::bitset<128>, Matrix<double>> computedProbabilities;
std::unordered_map<std::bitset<128>, bool> computedDeadEnds;

template <typename T>
void printMatrix (const Matrix<T>& matrix) {
    for (uint i = 0; i < matrix.size(); ++i){
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "[";
        for (uint j = 0; j < matrix.size(); ++j) {
            std::cout << matrix[i][j] << ", ";
        }
        std::cout << "]" << "\n";
    }
    if (matrix.size() == 0) {
        std::cout << "[]" << "\n";
    }
}

template <typename T>
void sortMatrix(const Matrix<T>& matrix_input,
                const std::vector<uint_fast16_t>& rowOrder,
                const std::vector<uint_fast16_t>& columnOrder,
                Matrix<T>& matrix_output,
                bool inverse) noexcept
{
    if (matrix_output.size() != matrix_input.size()) {
        matrix_output = matrix_input;
    }
    const auto size = static_cast<uint_fast16_t>(rowOrder.size());
    if (!inverse) {
        for (uint_fast16_t i = 0; i < size; ++i) {
            for (uint_fast16_t j = 0; j < size; ++j) {
                matrix_output[i][j] = matrix_input[rowOrder[i]][columnOrder[j]];
            }
        }
    }
    else {
        for (uint_fast16_t i = 0; i < size; ++i) {
            for (uint_fast16_t j = 0; j < size; ++j) {
                matrix_output[rowOrder[i]][columnOrder[j]] = matrix_input[i][j];
            }
        }
    }
}

void generateUnsortedId(const Matrix<bool>& matrix,
                        const std::vector<uint_fast16_t>& order,
                        const bool rowMode,
                        std::vector<std::pair<uint_fast16_t, uint_fast16_t>>& id) noexcept {

    const auto size = static_cast<uint_fast16_t>(matrix.size());
    bool entry;
    uint_fast16_t temp;
    for (uint_fast16_t i = 0; i < size; ++i) {
        temp = 0;
        for (uint_fast16_t j = 0; j < size; ++j) {
            temp <<= 1;
            if (rowMode) {
                entry = matrix[i][j];
            } else {
                entry = matrix[j][i];
            }
            if (entry) {
                temp |= 1;
            }
        }
        id[i].first = temp;
        id[i].second = order[i];
    }
}

void generateSortedId(const Matrix<bool>& compatibilityMatrix,
                      iD& idOut) noexcept
{
    bool row = true;
    bool sorted[2] = {false, false};
    auto size = static_cast<uint_fast16_t>(compatibilityMatrix.size());
    auto id = std::vector<std::pair<uint_fast16_t, uint_fast16_t>>(size, {0, 0});
    auto subId = std::vector<std::pair<uint_fast16_t, uint_fast16_t>>(size, {0, 0});
    //Matrix<uint_fast16_t> id(compatibilityMatrix.size(), std::vector<uint_fast16_t>(2, 0));
    //Matrix<uint_fast16_t> subId(compatibilityMatrix.size(), std::vector<uint_fast16_t>(2, 0));

    auto matrix2 = compatibilityMatrix;
    if(idOut.key.empty()) {
        for (uint_fast16_t i = 0; i < size; ++i) {
            idOut.rowOrder.push_back(i);
            idOut.columnOrder.push_back(i);
            idOut.key.push_back(0);
        }
    }
    uint_fast16_t maximum;

    while (true) {
        std::vector<uint_fast16_t>& order = row ? idOut.rowOrder : idOut.columnOrder;
        std::vector<std::pair<uint_fast16_t, uint_fast16_t>>& id_ref = row ? id : subId;

        generateUnsortedId(matrix2, order, row, id_ref);
        sorted[row ? 0 : 1] = true;
        maximum = 0;


        for (uint_fast16_t i = 0; i < size; ++i) {
            if (id_ref[i].first < maximum) {
                sorted[row ? 0 : 1] = false;
                break;
            } else {
                maximum = id_ref[i].first;
            }
        }
        if (!sorted[row ? 0 : 1]) {
            std::sort(id_ref.begin(), id_ref.end(), [](const auto& a, const auto& b) {
                return a.first < b.first;
            });

            for (uint_fast16_t i = 0; i < size; ++i) {
                order[i] = id_ref[i].second;
            }
        }

        if (sorted[0] && sorted[1]) {
            break;
        }

        sortMatrix(compatibilityMatrix, idOut.rowOrder, idOut.columnOrder, matrix2);
        row = !row;
    }

    for (uint_fast16_t i = 0; i < size; ++i) {
        idOut.key[i] = id[i].first;
    }
}

std::bitset<128> idGenerate(const std::vector<uint_fast16_t>& id) noexcept {
    auto size = static_cast<uint_fast16_t>(id.size());
    std::bitset<128> output;
    output |= static_cast<uint8_t>(size);
    output <<= 8;
    if (!id.empty()) {
        for (uint_fast16_t i = 0; i < size; ++i) {
        output |= static_cast<uint_fast16_t>(id[i]);
        if ((size - i) > 1) output <<= 16;
        }
        return output;
    }
    else {
        output.set();
        return output;
    }
}

loadedValue loadProbabilities(const iD& id,
                               Matrix<double>& probabilities) noexcept
{
    loadedValue output;
    const std::bitset<128> s = idGenerate(id.key);
    //const std::string s = idToString(key);
    auto it = computedProbabilities.find(s);
    //bool deadEnd = computedDeadEnds[s];
    if (it == computedProbabilities.end()) {
        // No saved value
        return output;
    }
    else {
        // saved value but deadEnd
        if (computedDeadEnds[s]) {
            output.SavedValue = true;
            output.DeadEnd = true;
            probabilities.clear();
            return output;
        }
    }
    // saved value and no deadEnd
    sortMatrix(it->second, id.rowOrder, id.columnOrder, probabilities, true);
    output.SavedValue = true;
    return output;
}

void saveProbabilities(const iD& id,
                       Matrix<double>& probabilities,
                       const bool deadEnd) noexcept
{
    const std::bitset<128> s = idGenerate(id.key);
    //const std::string s = idToString(key);
    computedProbabilities[s] = probabilities;
    computedDeadEnds[s] = deadEnd;
    if (!probabilities.empty()) {
        sortMatrix(probabilities, id.rowOrder, id.columnOrder, computedProbabilities[s]);
    }
}

bool computeProbabilities(const Matrix<bool>& compatibilityMatrix,
                          Matrix<double>& probabilities,
                          int unmatchedRunnerUp) noexcept
{
    auto size = static_cast<uint_fast16_t>(compatibilityMatrix.size());
    probabilities.clear();

    iD id;
    Matrix<double> conditionalProbabilities;
    bool output = false;

    if (unmatchedRunnerUp == -1) {
        generateSortedId(compatibilityMatrix, id);
        auto check = loadProbabilities(id, probabilities);
        if (check.SavedValue){
            if (check.DeadEnd) {
                return true;
            }
            else {
                return false;
            }
        }
    }

    for (uint_fast16_t i = 0; i < size; ++i) {
    probabilities.emplace_back(size, 0);
    }


    int options = 0;
    if (unmatchedRunnerUp == -1) {
        for(uint_fast16_t i = 0; i < size; ++i) {
            options++;
            auto deadEnd = computeProbabilities(compatibilityMatrix, conditionalProbabilities, i);
            if (deadEnd) {
                options--;
            }
            else {
                for (uint_fast16_t j = 0; j < size; ++j) {
                    for (uint_fast16_t k = 0; k < size; ++k) {
                        probabilities[j][k] += conditionalProbabilities[j][k];
                    }
                }
            }
        }
        if (options == 0 && size > 0) {
            probabilities.clear();
            output = true;
        }
    }
    else {
        for (uint_fast16_t i = 0; i < size; ++i) {
            if (compatibilityMatrix[i][unmatchedRunnerUp]) {
                options++;
                Matrix<bool> subMatrix;
                for (uint_fast16_t j = 0; j < size; ++j) {
                    if (j != i) {
                        std::vector<bool> row{};
                        for (uint_fast16_t k = 0; k < size; ++k) {
                            if (k != unmatchedRunnerUp) {
                                row.push_back(compatibilityMatrix[j][k]);
                            }
                        }
                        subMatrix.push_back(row);
                    }
                }

                auto deadEnd = computeProbabilities(subMatrix, conditionalProbabilities);
                if (deadEnd) {
                    options--;
                }
                else {
                    for (uint_fast16_t j = 0; j < size; ++j) {
                        for (uint_fast16_t k = 0; k < size; ++k) {
                            if (j < i) {
                                if (k < unmatchedRunnerUp) {
                                    probabilities[j][k] += conditionalProbabilities[j][k];
                                }
                                else if (k > unmatchedRunnerUp) {
                                    probabilities[j][k] += conditionalProbabilities[j][k-1];
                                }
                            }
                            else if (j > i) {
                                if (k < unmatchedRunnerUp) {
                                    probabilities[j][k] += conditionalProbabilities[j-1][k];
                                }
                                else if (k > unmatchedRunnerUp) {
                                    probabilities[j][k] += conditionalProbabilities[j-1][k-1];
                                }
                            }
                        }
                    }
                    probabilities[i][unmatchedRunnerUp] += 1;
                }
            }
        }
        if (options == 0) {
            probabilities.clear();
            output = true;
        }
    }
    if (options != 0) {
        for (uint_fast16_t i = 0; i < size; ++i) {
            for (uint_fast16_t j = 0; j < size; ++j) {
                probabilities[i][j] /= options;
            }
        }
    }
    if (unmatchedRunnerUp == -1) {
        saveProbabilities(id, probabilities, output);
    }

    return output;
}

void getProbabilities(const Matrix<std::string>& winners, const Matrix<std::string>& runnersUp) {
    const uint_fast16_t numTeams = static_cast<uint_fast16_t>(winners.size());
    Matrix<bool> fullCompatibilityMatrix(numTeams, std::vector<bool>(numTeams, false));
    for (size_t i = 0; i < numTeams; ++i) {
        for (size_t j = 0; j < numTeams; ++j) {
            bool temp = true;
            for (size_t k = 0; k < winners[0].size(); ++k) {
                if (winners[i][k] == runnersUp[j][k]) {
                    temp = false;
                }
            }
            fullCompatibilityMatrix[i][j] = temp;
        }
    }
    Matrix<double> probabilities(numTeams, std::vector<double>(numTeams, 0.0));
    auto start = std::chrono::high_resolution_clock::now();
    computeProbabilities(fullCompatibilityMatrix, probabilities);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Time taken to compute probabilities: " << duration.count() << " milliseconds." << std::endl;

    printMatrix(probabilities);
}

// int main()
// {
//     const Matrix<std::string> winners = {{"A","DE"}, {"B","EN"}, {"C","ES"}, {"D","ES"}, {"E","ES"}, {"F","DE"}, {"G","EN"}, {"H","ES"}};
//     const Matrix<std::string> runnersUp = {{"A","DEN"}, {"B","DEN"}, {"C","IT"}, {"D","IT"}, {"E","IT"}, {"F","FR"}, {"G","DE"}, {"H","PR"}};

//     getProbabilities(winners, runnersUp);

//     return 0;
// }
