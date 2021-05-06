/****************************************************************************
 *
 * matstorage.cpp - Functions for converting between different internal storage
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
 ****************************************************************************/

#include "matstorage.h"
#include "utils.h"

void check_bc(matrix_pcsr_t g, const float *bc_cpu, const float *bc_gpu) {
    for (int i = 0; i < g.row_offsets[g.nrows]; i++) {
        assert(bc_cpu[i] == bc_gpu[i]);
    }
}

int check_matrix_pcoo_init(matrix_pcoo_t *matrix) {
    return (matrix->cols != nullptr) &&
           (matrix->rows != nullptr) &&
           (matrix->nrows > -1) &&
           (matrix->nnz > -1);
}

int check_matrix_rcoo_init(matrix_rcoo_t *matrix) {
    return (matrix->cols != nullptr) &&
           (matrix->rows != nullptr) &&
           (matrix->weights != nullptr) &&
           (matrix->nrows > -1) &&
           (matrix->nnz > -1);
}

void pcoo_to_pcsr(matrix_pcoo_t *m_coo, matrix_pcsr_t *m_csr) {

    if (!check_matrix_pcoo_init(m_coo)) {
        fprintf(stderr, "The matrix is not initialized");
        return;
    }

    int *row_offsets;
    int *rows = m_coo->rows; // row indices of A
    int nnz = m_coo->nnz;    // number of nnz in A
    int nrows = m_coo->nrows;// number of rows in A
    int *cols;

    row_offsets = (int *) malloc((nrows + 1) * sizeof(*row_offsets));
    cols = (int *) malloc(nnz * sizeof(*cols));
    assert(row_offsets);
    assert(cols);
    fill(row_offsets, (nrows + 1), 0);

    /*
     * Compute number of non-zero entries per column of A.
     */
    for (int n = 0; n < nnz; n++) {
        row_offsets[rows[n]]++;
    }

    /*
     * Compute row offsets
     */
    for (int i = 0, psum = 0; i < nrows; i++) {
        int temp = row_offsets[i];
        row_offsets[i] = psum;
        psum += temp;
    }
    row_offsets[nrows] = nnz;

    /*
     * Copy cols array of A in cols of B
     */
    for (int n = 0; n < nnz; n++) {
        int row = rows[n];
        int dest = row_offsets[row];
        cols[dest] = m_coo->cols[n];
        row_offsets[row]++;
    }

    for (int i = 0, last = 0; i <= nrows; i++) {
        int temp = row_offsets[i];
        row_offsets[i] = last;
        last = temp;
    }

    m_csr->nrows = nrows;
    m_csr->cols = cols;
    m_csr->row_offsets = row_offsets;
}

void rcoo_to_rcsr(matrix_rcoo_t *m_coo, matrix_rcsr_t *m_csr) {

    if (!check_matrix_rcoo_init(m_coo)) {
        fprintf(stderr, "The matrix is not initialized");
        return;
    }

    int *row_offsets;
    int *rows = m_coo->rows; // row indices of A
    int nnz = m_coo->nnz;    // number of nnz in A
    int nrows = m_coo->nrows;// number of rows in A
    int *cols, *weights;

    row_offsets = (int *) malloc((nrows + 1) * sizeof(*row_offsets));
    cols = (int *) malloc(nnz * sizeof(*cols));
    weights = (int *) malloc(nnz * sizeof(*weights));
    assert(row_offsets);
    assert(cols);
    fill(row_offsets, (nrows + 1), 0);

    /*
     * Compute number of non-zero entries per column of A.
     */
    for (int n = 0; n < nnz; n++) {
        row_offsets[rows[n]]++;
    }

    /*
     * Compute row offsets
     */
    for (int i = 0, psum = 0; i < nrows; i++) {
        int temp = row_offsets[i];
        row_offsets[i] = psum;
        psum += temp;
    }
    row_offsets[nrows] = nnz;

    /*
     * Copy cols array of A in cols of B
     */
    for (int n = 0; n < nnz; n++) {
        int row = rows[n];
        int dest = row_offsets[row];
        cols[dest] = m_coo->cols[n];
        weights[dest] = m_coo->weights[n];
        row_offsets[row]++;
    }

    for (int i = 0, last = 0; i <= nrows; i++) {
        int temp = row_offsets[i];
        row_offsets[i] = last;
        last = temp;
    }

    m_csr->nrows = nrows;
    m_csr->cols = cols;
    m_csr->row_offsets = row_offsets;
}

void print_matrix_coo(matrix_pcoo_t *matrix) {

    if (!check_matrix_pcoo_init(matrix)) {
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

void print_matrix_csr(matrix_pcsr_t *matrix) {

    if (matrix->row_offsets == nullptr) {
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

void free_matrix_pcoo(matrix_pcoo_t *matrix) {
    free(matrix->rows);
    free(matrix->cols);
    matrix->rows = nullptr;
    matrix->cols = nullptr;
    matrix->nnz = -1;
    matrix->nrows = -1;
}

void free_matrix_pcsr(matrix_pcsr_t *matrix) {
    free(matrix->row_offsets);
    free(matrix->cols);
    matrix->row_offsets = nullptr;
    matrix->cols = nullptr;
    matrix->nrows = -1;
}

void free_matrix_rcoo(matrix_rcoo_t *matrix) {
    free(matrix->rows);
    free(matrix->cols);
    free(matrix->weights);
    matrix->rows = nullptr;
    matrix->cols = nullptr;
    matrix->nnz = -1;
    matrix->nrows = -1;
}

void free_matrix_rcsr(matrix_rcsr_t *matrix) {
    free(matrix->row_offsets);
    free(matrix->cols);
    free(matrix->weights);
    matrix->row_offsets = nullptr;
    matrix->cols = nullptr;
    matrix->nrows = -1;
}