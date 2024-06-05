#ifndef SYMNMF_H
#define SYMNMF_H

int allocate_2D_array(double ***arr, int rows, int cols);

int free_2D_array(double ***arr, int rows);

int C_sym(double ***A, double ***X, const int rows, const int cols);

int C_ddg(double **D, double ***A, const int N);

int C_norm(double ***W, double **D, double ***A, const int N);

int C_symnmf(double ***H_out, double ***H_in, const int rows_H, const int cols_H,
             double ***W, const int N_W);

int parse_diag_to_matrix_form(double ***M, double **D, const int N);

#endif
