#ifndef MUL_GUSTAFSON_H
#define MUL_GUSTAFSON_H

#include <string.h> // memset
#include "../util/matrix_util.h"

/**
 * This function is the true key implementation of our project. There are a few
 * reasons for that.
 *  @c + It operates on vectors in the @c CSC format, so it should be much
 * more space efficient than any algorithm that expands any vector to 
 * its' true size in its' operation. This corresponds well with the benefits of
 * the CSC/RSC format itself.
 *  @c + Secondly it maximizes the count of the so-called @c concordant traversals, as per [1]
 * and [2]. In the case of CSC format and under the constraints of our
 * matrix_data struct (See matrix.h), that would be when when we try to
 * explicitely load a single *col* of a matrix,as opposed to a row. Therefore we extract
 * a col from B (efficient fetch) and all the cols from A (efficient fetch)
 * for which the value in col from B is not zero. After multiplying and Summing
 * up the results, this will yield a full column of the result matrix.
 * 
 * @c caveat Some cols from A will possibly have to be loaded multiple times. This gets worse
 * for less sparse matrices and could be optimized for such, by loading all the columns
 * from A in advance. This would in turn limit the low-storage-requirements of
 * this algorithm, which is its' key value. Therefore we decided on repeateadly
 * loading the columns for each column of B.
 *
 * [1] https://people.csail.mit.edu/sanchez/papers/2021.gamma.asplos.pdf 
 * [2] Vivienne Sze, Yu-Hsin Chen, Tien-Ju Yang, and Joel S Emer. Efficient processing
of deep neural networks. Synthesis Lectures on Computer Architecture, 15(2), 2020
 * [3] https://www.researchgate.net/figure/Gustavson-algorithm-Sparse-matrix-matrix-multiplication-is-performed-in-a-row-wise_fig4_327077990#:~:text=Gustavson%20algorithm%3A%20Sparse%20matrix%E2%80%93matrix%20multiplication%20is%20performed%20in%20a,based%20on%20their%20column%20indices.
 */
__attribute__((hot)) void matrix_mul_gustavson(const void*  a, const void*  b, void*  c);

#endif //MUL_GUSTAFSON_H
