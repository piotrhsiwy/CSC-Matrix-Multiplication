import numpy as np


def print_vect(vect):
    first = True
    for val in vect:
        if first:
            print("%.6f" % val, end='')
            first = False
        else:
            print("," + ("%.6f" % val), end='')

    print("")


def print_vect_normal(vect):
    first = True
    for val in vect:
        if first:
            print(val, end='')
            first = False
        else:
            print("," + str(val), end='')
    print("")


def print_numpy_matrix_as_compressed(arr):
    values = []
    row_indeces = []
    col_ptr = [0]

    column_i = 0
    row_i = 0
    for column in arr.T:
        for elem in column:
            if elem == 0:
                row_i += 1
                continue
            values.append(elem)
            row_indeces.append(row_i)
            row_i += 1
        row_i = 0
        column_i += 1
        col_ptr.append(len(values))

    print(str(len(arr)) + "," + str(len(arr[0])))
    print_vect(values)
    print_vect_normal(row_indeces)
    print_vect_normal(col_ptr)


arr1 = np.array([[1, 2], [4, 5]])
print(arr1)
arr2 = np.array([[3, 4], [7, 8]])
print(arr2)

# print A.txt and B.txt
print("A.txt:")
print_numpy_matrix_as_compressed(arr1)
print("B.txt:")
print_numpy_matrix_as_compressed(arr2)

# expected.txt
arr3 = np.matmul(arr1, arr2)
print(arr3)
print("expected.txt:")
print_numpy_matrix_as_compressed(arr3)
