#include <stdio.h>

#include "matrix_util.h"

error expand_row(const pair* const compressed, const uint64_t length_compressed, float* const extended, const uint64_t length_extended){
	if(compressed[length_compressed-1].index >= length_extended){
		return DIMENSION_MISMATCH;
	}
	for(uint64_t i = 0; i < length_compressed; i++){
		extended[compressed[i].index] = compressed[i].value;
	}
	return OK;
}

void expand_matrix(const matrix_data* const compressed, float* const * const expanded){
	for(uint64_t col = 0; col <= compressed->no_cols; col++){
		for(uint64_t i = compressed->col_ptr[col]; i <= compressed->col_ptr[col+1]; i++){
			uint64_t row = compressed->row_indices[i];
			float value = compressed->values[i];
			expanded[row][col] = value;
		}
	}
	return OK;
}

error compress_matrix(const uint64_t no_rows, const uint64_t no_cols, const float expanded[no_rows][no_cols], matrix_data* const compressed){
	compressed->no_rows = no_rows;
	compressed->no_cols = no_cols;

	//TODO
		
	return OK;
}
