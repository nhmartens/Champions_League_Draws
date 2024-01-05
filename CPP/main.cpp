#include "CL_Draws.h"

int main() {
    const Matrix<std::string> winners = {{"A","DE"}, {"B","EN"}, {"C","ES"}, {"D","ES"}, {"E","ES"}, {"F","DE"}, {"G","EN"}, {"H","ES"}};
    const Matrix<std::string> runnersUp = {{"A","DEN"}, {"B","DEN"}, {"C","IT"}, {"D","IT"}, {"E","IT"}, {"F","FR"}, {"G","DE"}, {"H","PR"}};

    getProbabilities(winners, runnersUp);
    return 0;
}
