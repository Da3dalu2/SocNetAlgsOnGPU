/****************************************************************************
 *
 * mformats.h - Utility functions for converting between different internal
 * storage matrix representations
 *
 * Copyright 2021 (c) 2021 by Riccardo Battistini <riccardo.battistini2(at)studio.unibo.it>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * --------------------------------------------------------------------------
 *
 * This header file provides functions to convert from the COOrdinate format
 * to the Compressed Sparse Rows. In addition it provides
 * a way of representing these matrices with Structures of Arrays.
 *
 * The conversion functions are taken from the scipy source code, in particular
 * from https://github.com/scipy/scipy/blob/f2ef65dc7f00672496d7de6154744fee55ef95e9/scipy/sparse/sparsetools/coo.h#L33
 *
 * They have been adapted to accept structures representing the matrix formats.
 *
 ****************************************************************************/

#ifndef MATRIXFORMATS_H
#define MATRIXFORMATS_H

typedef struct matrix_coo_t {
    int nrows;  // = ncols since adj matrix is a square matrix
    int nnz;
    int *rows;  // row index for each non-zero value
    int *cols;  // column index for each non-zero value
} matrix_coo_t;

typedef struct matrix_csr_t {
    int nrows;
    int nnz;
    int *row_offsets;    // offset in columns
    int *cols;           // column index for each non-zero value
} matrix_csr_t;

/**
 * Initialize an array.
 *
 * @param arr pointer to the array to be initialized
 * @param n length of the array to be initialized
 * @param v value used to initialize the array
 */
void fill( int *arr, int n, int v) {
    for (int i = 0; i < n; i++)
        arr[i] = v;
}

//void printArray(int *array, int n) {
//    for(int i = 0; i < n; i++)
//        printf("[%d]: %d\n", i, array[i]);
//}

void fillPrefixSum(const int *arr, int n, int *prefixSum) {
    prefixSum[0] = arr[0];
    for (int i = 1; i < n; i++)
        prefixSum[i] = prefixSum[i - 1] + arr[i];
}

/**
 * Convert a matrix A, stored in COO format, to a matrix B, stored in the CSR
 * format.
 *
 * At the end (row_offsets, cols) forms a CSR representation,
 * with possible duplicates if they were present in the COO format.
 * This means that duplicate entries in the COO matrix are not merged.
 *
 * Row and column indices *are not* assumed to be ordered.
 *
 * The conversion algorithm has linear complexity.
 * Specifically O(nnz(A) + max(nrows, ncols)).
 *
 * @param nrows_dense
 * @param m_coo
 * @param m_csr structure representing the matrix in the new format.
 * Row_offsets and cols fields *must not* be preallocated.
 */
void coo2csr(matrix_coo_t *m_coo, matrix_csr_t *m_csr)
{
    int *row_offsets, *scan;
    int *rows = m_coo->rows; // row indices of A
    int nnz = m_coo->nnz;    // number of nnz in A
    int nrows = m_coo->nrows;    // number of rows in A
    int *cols;

    /*
     * Compute number of non-zero entries per row of A.
     */
    row_offsets = (int *) malloc((nrows + 1) * sizeof(*row_offsets));
    scan = (int *) malloc(nrows * sizeof(*scan));
    fill(row_offsets, (nrows + 1), 0);

    for (int n = 0; n < nnz; n++){
        row_offsets[rows[n]]++;
    }

    /*
     * Compute row offsets
     */
    fillPrefixSum(row_offsets, nrows, scan);
    row_offsets = scan;

    /*
     * Copy cols array of A in cols of B
     */
    cols = (int *) malloc(nnz * sizeof(*cols));

    for(int n = 0; n < nnz; n++) {
        cols[n] = m_coo->cols[n] + 1;
    }

    for(int i = 0, last = 0; i <= nrows; i++){
        int temp = row_offsets[i];
        row_offsets[i] = last;
        last = temp;
    }
    row_offsets[nrows] = nnz;

    m_csr->nrows = nrows;
    m_csr->nnz = nnz;
    m_csr->cols = cols;
    m_csr->row_offsets = row_offsets;
}

#endif //MATRIXFORMATS_H