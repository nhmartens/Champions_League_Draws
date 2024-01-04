#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <vector>
#include <array>
#include <set>
#include <sstream>
#include <chrono>


constexpr static char numTeams = 8;
double counter = 0.0;
template <typename T>
using Matrix = std::vector<std::vector<T>>;
uint count = 0;

const static std::array<std::array<std::string, 2>, numTeams> winners = {{{"A","DE"}, {"B","EN"}, {"C","ES"}, {"D","ES"}, {"E","ES"}, {"F","DE"}, {"G","EN"}, {"H","ES"}}};
const static std::array<std::array<std::string, 2>, numTeams> runnersUp = {{{"A","DEN"}, {"B","DEN"}, {"C","IT"}, {"D","IT"}, {"E","IT"}, {"F","FR"}, {"G","DE"}, {"H","PR"}}};


static std::unordered_map<std::bitset<128>, Matrix<double>> computedProbabilities;
static std::unordered_map<std::bitset<128>, bool> computedDeadEnds;

static Matrix<bool> fullCompatibilityMatrix(numTeams, std::vector<bool>(numTeams, false));
//static std::string seasonId = "0";
static std::bitset<128> seasonId;
static std::unordered_map<std::bitset<128>, std::set<std::string>> seasonLog;

struct LoadedValue {
    bool SavedValue = false;
    bool DeadEnd = false;
};

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
                bool inverse = false) noexcept
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
                        Matrix<uint_fast16_t>& id) noexcept {

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
        id[i][0] = temp;
        id[i][1] = order[i];
    }
}

void generateSortedId(const Matrix<bool>& compatibilityMatrix,
                      std::vector<uint_fast16_t>& key,
                      std::vector<uint_fast16_t>& rowOrder,
                      std::vector<uint_fast16_t>& columnOrder) noexcept
{
    bool row = true;
    bool sorted[2] = {false, false};
    Matrix<uint_fast16_t> id(compatibilityMatrix.size(), std::vector<uint_fast16_t>(2, 0));
    Matrix<uint_fast16_t> subId(compatibilityMatrix.size(), std::vector<uint_fast16_t>(2, 0));
    auto size = static_cast<uint_fast16_t>(subId.size());
    auto matrix2 = compatibilityMatrix;
    if(key.empty()) {
        for (uint_fast16_t i = 0; i < size; ++i) {
            rowOrder.push_back(i);
            columnOrder.push_back(i);
            key.push_back(0);
        }
    }
    uint_fast16_t maximum;

    while (true) {
        std::vector<uint_fast16_t>& order = row ? rowOrder : columnOrder;
        Matrix<uint_fast16_t>& id_ref = row ? id : subId;

        generateUnsortedId(matrix2, order, row, id_ref);
        sorted[row ? 0 : 1] = true;
        maximum = 0;


        for (uint_fast16_t i = 0; i < size; ++i) {
            if (id_ref[i][0] < maximum) {
                sorted[row ? 0 : 1] = false;
                break;
            } else {
                maximum = id_ref[i][0];
            }
        }
        if (!sorted[row ? 0 : 1]) {
            std::sort(id_ref.begin(), id_ref.end(), [](const auto& a, const auto& b) {
                return a[0] < b[0];
            });

            for (uint_fast16_t i = 0; i < size; ++i) {
                order[i] = id_ref[i][1];
            }
        }

        // if (row) {
        //     id = subId;
        // }

        if (sorted[0] && sorted[1]) {
            break;
        }

        sortMatrix(compatibilityMatrix, rowOrder, columnOrder, matrix2);
        row = !row;
    }

    for (uint_fast16_t i = 0; i < size; ++i) {
        key[i] = id[i][0];
    }

}

// std::string idToString(const std::vector<uint_fast16_t>& id) {
//     std::string str;
//     str.reserve(4*id.size());
//     std::stringstream ss(str);
//     auto size = static_cast<uint_fast16_t>(id.size());

//     for (uint_fast16_t i = 0; i < size; ++i) {
//         if (id[i] < 16) {
//             ss << std::setfill('0') << std::setw(4) << std::hex << id[i];
//         } else if (id[i] < 256) {
//             ss << std::setfill('0') << std::setw(3) << std::hex << id[i];
//         } else if (id[i] < 4096) {
//             ss << std::setfill('0') << std::setw(2) << std::hex << id[i];
//         } else {
//             ss << std::hex << id[i];
//         }
//     }
//     return ss.str();
// }


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

LoadedValue loadProbabilities (const std::vector<uint_fast16_t>& key,
                        const std::vector<uint_fast16_t>& rowOrder,
                        const std::vector<uint_fast16_t>& columnOrder,
                        Matrix<double>& probabilities) noexcept
{
    LoadedValue output;
    const std::bitset<128> s = idGenerate(key);
    //const std::string s = idToString(key);
    //seasonLog[seasonId].insert(s);
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
    sortMatrix(it->second, rowOrder, columnOrder, probabilities, true);
    output.SavedValue = true;
    return output;
}

void saveProbabilities(const std::vector<uint_fast16_t>& key,
                       const std::vector<uint_fast16_t>& rowOrder,
                       const std::vector<uint_fast16_t>& columnOrder,
                       Matrix<double>& probabilities,
                       const bool deadEnd) noexcept
{
    const std::bitset<128> s = idGenerate(key);
    //const std::string s = idToString(key);
    computedProbabilities[s] = probabilities;
    computedDeadEnds[s] = deadEnd;
    if (!probabilities.empty()) {
        sortMatrix(probabilities, rowOrder, columnOrder, computedProbabilities[s]);
    }
}

bool computeProbabilities(const Matrix<bool>& compatibilityMatrix,
                          Matrix<double>& probabilities,
                          int unmatchedRunnerUp=-1)
{
    auto size = static_cast<uint_fast16_t>(compatibilityMatrix.size());
    probabilities.clear();

    std::vector<uint_fast16_t> key;
    std::vector<uint_fast16_t> rowOrder;
    std::vector<uint_fast16_t> columnOrder;
    Matrix<double> conditionalProbabilities;
    bool output = false;

    if (unmatchedRunnerUp == -1) {
        generateSortedId(compatibilityMatrix, key, rowOrder, columnOrder);
        //Matrix<double> cachedProbabilities{};
        auto check = loadProbabilities(key, rowOrder, columnOrder, probabilities);
        if (check.SavedValue){
            //probabilities = cachedProbabilities;
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
        saveProbabilities(key, rowOrder, columnOrder, probabilities, output);
    }

    return output;
}


void initialize() {
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
    std::vector<uint_fast16_t> rowOrder;
    std::vector<uint_fast16_t> columnOrder;
    std::vector<uint_fast16_t> key;
    for (uint_fast16_t i = 0; i < fullCompatibilityMatrix.size(); ++i) {
        rowOrder.push_back(i);
        columnOrder.push_back(i);
        key.push_back(0);
    }
    generateSortedId(fullCompatibilityMatrix, key, rowOrder, columnOrder);
    seasonId = idGenerate(key);
    //seasonId = idToString(key);
    seasonLog.insert({seasonId, std::set<std::string>()});

};


int main()
{
    initialize();
    /*
    for (size_t i = 0; i < numTeams; ++i) {
        for (size_t j = 0; j < numTeams; ++j) {
            std::cout << fullCompatibilityMatrix[i][j] << " ";
        }
        std::cout << "\n";
    }
    */
    std::vector<uint_fast16_t> rowOrder;
    std::vector<uint_fast16_t> columnOrder;
    std::vector<uint_fast16_t> key;
    for (uint_fast16_t i = 0; i < fullCompatibilityMatrix.size(); ++i) {
        rowOrder.push_back(i);
        columnOrder.push_back(i);
        key.push_back(0);
    }
    generateSortedId(fullCompatibilityMatrix, key, rowOrder, columnOrder);
    //for (size_t i = 0; i < fullCompatibilityMatrix.size(); ++i) {
    //        std::cout << key[i] << " ";
    //    }
    //    std::cout << "\n";


    Matrix<double> probabilities(fullCompatibilityMatrix.size(), std::vector<double>(fullCompatibilityMatrix.size(), 0.0));

    auto start = std::chrono::high_resolution_clock::now();
    computeProbabilities(fullCompatibilityMatrix, probabilities);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Time taken by function: " << duration.count() << " milliseconds." << std::endl;

    printMatrix(probabilities);
    std::size_t numberOfElements = computedProbabilities.size();
    std::cout << "Number of elements in the map: " << numberOfElements << std::endl;




    // if (probabilities.empty()){
    //     std::cout << "Probabilities is empty";
    // }
    // std::cout << "\n";
    // std::cout << "\n";
    // for (uint i = 0; i < probabilities.size(); ++i) {
    //     for (uint j = 0; j < probabilities.size(); ++j) {
    //         std::cout << probabilities[i][j] <<", ";
    //     }
    //     std::cout << "\n";
    // }

    // Matrix<bool> CM{
    //     {false, true},
    //     {true, false}
    // };
    // int unmatched = 1;



    // computeProbabilities(CM, probabilities, unmatched);
    // std::cout << "\n";
    // printMatrix(probabilities);

    return 0;
}
