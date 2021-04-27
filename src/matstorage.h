/****************************************************************************
 *
 * matstorage.h - Functions for converting between different internal storage
 * matrix representations
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
 * to the Compressed Sparse Rows. In addition it provides a way of representing
 * adjacency matrices with structures of arrays.
 *
 * The conversion function is based on the one found in the scipy source
 * code, specifically from
 * https://github.com/scipy/scipy/blob/f2ef65dc7f00672496d7de6154744fee55ef95e9/scipy/sparse/sparsetools/coo.h#L33
 *
 * TODO: Remove duplicated edges
 *
 ****************************************************************************/

#ifndef MATSTORAGE_H
#define MATSTORAGE_H

typedef struct matrix_coo_t {
    int nrows;  // = ncols since adj matrix is a square matrix
    int nnz;
    int *rows;  // row index for each non-zero value
    int *cols;  // column index for each non-zero value
} matrix_coo_t;

typedef struct matrix_csr_t {
    int nrows;
//    int nnz;  // found at row_offsets[nrows]
    int *row_offsets;    // offset in columns
    int *cols;           // column index for each non-zero value
} matrix_csr_t;

typedef struct gprops_t {
    bool is_directed;
    bool is_weighted;
    bool has_self_loops;
    bool is_connected;
} gprops_t;

#include "utils.h"

int all_matrix_coo_init(matrix_coo_t* matrix) {
    return  (matrix->cols != NULL) &&
            (matrix->rows != NULL) &&
            (matrix->nrows > -1) &&
            (matrix->nnz > -1);
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
 * As long as the average number of non-zeroes per row is > 1,
 * this format saves space relative to COO.
 *
 * @param nrows_dense
 * @param m_coo
 * @param m_csr structure representing the matrix in the new format.
 * Row_offsets and cols fields *must not* be preallocated.
 */
void coo_to_csr(matrix_coo_t *m_coo, matrix_csr_t *m_csr)
{

    if(!all_matrix_coo_init(m_coo)) {
        fprintf(stderr, "The matrix is not initialized");
        return;
    }

    int *row_offsets, *scan;
    int *rows = m_coo->rows;    // row indices of A
    int nnz = m_coo->nnz;       // number of nnz in A
    int nrows = m_coo->nrows;   // number of rows in A
    int *cols;

    row_offsets = (int *) malloc((nrows + 1) * sizeof(*row_offsets));
    cols = (int *) malloc(nnz * sizeof(*cols));
    assert(row_offsets);
    assert(cols);
    fill(row_offsets, (nrows + 1), 0);

    /*
     * Compute number of non-zero entries per column of A.
     */
    for (int n = 0; n < nnz; n++){
        row_offsets[rows[n]]++;
    }

    /*
     * Compute row offsets
     */
    for(int i = 0, psum = 0; i < nrows; i++){
        int temp = row_offsets[i];
        row_offsets[i] = psum;
        psum += temp;
    }
    row_offsets[nrows] = nnz;

    /*
     * Copy cols array of A in cols of B
     */
    for(int n = 0; n < nnz; n++) {
        int row = rows[n];
        int dest = row_offsets[row];

        cols[dest] = m_coo->cols[n];

        row_offsets[row]++;
    }

    for(int i = 0, last = 0; i <= nrows; i++){
        int temp = row_offsets[i];
        row_offsets[i] = last;
        last = temp;
    }

    m_csr->nrows = nrows;
    m_csr->cols = cols;
    m_csr->row_offsets = row_offsets;

}

void print_matrix_coo(matrix_coo_t* matrix) {

    if(!all_matrix_coo_init(matrix)) {
        fprintf(stderr, "The matrix is not initialized");
        return;
    }

    printf("nrows = %d\n", matrix->nrows);
    printf("nnz = %d\n", matrix->nnz);
    printf("rows = \n");
    print_array(matrix->rows, matrix->nnz - 1);
    printf("cols = \n");
    print_array(matrix->cols, matrix->nnz - 1);
}

void print_matrix_csr(matrix_csr_t* matrix) {

    if(matrix->row_offsets == NULL) {
        fprintf(stderr, "The matrix is not initialized");
        return;
    }

    int nnz = matrix->row_offsets[matrix->nrows];
    printf("nrows = %d\n", matrix->nrows);
    printf("offsets = \n");
    print_array(matrix->row_offsets, matrix->nrows);
    printf("cols = \n");
    print_array(matrix->cols, nnz - 1);
}

void free_matrix_coo(matrix_coo_t* matrix)
{
    free(matrix->rows);
    free(matrix->cols);
    matrix->rows = nullptr;
    matrix->cols = nullptr;
    matrix->nnz = -1;
    matrix->nrows = -1;
}

void free_matrix_csr(matrix_csr_t* matrix)
{
    free(matrix->row_offsets);
    free(matrix->cols);
    matrix->row_offsets = nullptr;
    matrix->cols = nullptr;
    matrix->nrows = -1;
}

void compute_degrees_undirected(matrix_coo_t* g, int *degree) {

    if(!all_matrix_coo_init(g)) {
        fprintf(stderr, "The graph is not initialized");
        return;
    }

    int *rows = g->rows;    // row indices of A
    int nnz = g->nnz;       // number of nnz in A
    int nrows = g->nrows;   // number of rows in A

    fill(degree, nrows, 0);

    /*
     * Compute number of non-zero entries per row of A.
     */
    for (int n = 0; n < nnz; n++){
        degree[rows[n]]++;
    }
}

void compute_degrees_directed(matrix_coo_t*  g, int *in_degree,
                              int *out_degree) {

    if(g->rows == NULL) {
        fprintf(stderr, "The graph is not initialized");
        return;
    }

    int *rows = g->rows;    // row indices of A
    int nnz = g->nnz;       // number of nnz in A
    int length = g->nrows;  // number of rows and columns in A
    int *cols = g->cols;    // column indices of A

//    offsets = (int *) malloc((length + 1) * sizeof(*offsets));
    fill(in_degree, length, 0);
    fill(out_degree, length, 0);

    /*
     * Compute number of non-zero entries per column of A.
     */
    for (int n = 0; n < nnz; n++){
        out_degree[rows[n]]++;
    }

    /*
     * Compute number of non-zero entries per row of A.
     */
    for (int n = 0; n < nnz; n++){
        in_degree[cols[n]]++;
    }
}

#endif // MATSTORAGE_H
