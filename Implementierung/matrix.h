//
// Created by lars on 6/17/24.
//

#ifndef GRA24CAPSPROJEKT_T064_MATRIX_H
#define GRA24CAPSPROJEKT_T064_MATRIX_H


struct matrix_data {
    uint64_t noRows;
    uint64_t noCols;
    size_t length_values;
    float* values;
    uint64_t* rowIndex;
    // len (*colPtr) = noRows + 1
    uint64_t* colPtr;
};



#endif //GRA24CAPSPROJEKT_T064_MATRIX_H
