Here are the constraints of the CSC format:

Matrices in the CSC format are stored in .txt file of the following format:
> no_rows, no_cols
> v0, v1, v2, ...
> i0, i1, i2, ...
> 0, c1, c2, ...

with the following semantics:
> no_rows and no_cols specify number of rows and number of columns of the matrix
> v0, v1, ... are values of the matrix
> i0, i1, ... is the row-index of the corresponding value
> 0, c1, .. is the accumulative number of non-zero values in the columns (values v0-vc1 are in the 1st column, values vc1-vc2 are in the 2nd column, etc.)

### Following syntactic constrains are to be imposed on the  format. All of the requirements have to be fulfilled for the matrix to be valid
> no_rows, no_cols (line 1)
    #1 length: 2
    #2 values-range: [1, UINT64_MAX]
> v0, v1, v2, ... (line 2)
    #3 length: [0, min{UINT64_MAX, no_rows*no_cols}]
    #4 values-range: float - {0.0f}
> i0, i1, i2, ... (line 3)
    #5 length: same as of the previous line
    #6 values-range: [0, no_rows-1]
> 0, c1, c2, ... (line 4)
    #7 length: no_cols+1
    #8 values-range: first value = 0, last value = length of line 2, rest in [0, min{UINT64_MAX, no_rows*no_cols}], (non-strictly) increasing
    #9 have to pass sanity check:
        for all index in [0, no_cols-1]: i[c[index]], ..., i[c[index+1]] are strictly increasing (implies: unique)
            where i[] and c[] are values from lines 3 and 4 respectively