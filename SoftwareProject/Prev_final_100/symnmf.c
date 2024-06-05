#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symnmf.h"

#define BETA 0.5
#define MAX_ITER 300
#define EPS 0.0001
#define DELIMITER ','
#define SYM "sym"
#define DDG "ddg"
#define NORM "norm"

const char *ERR_MSG = "An Error Has Occurred\n";

/*
 * Allocated new 2D array on 'arr' address.
 * Returns 0 on success.
 * */
int allocate_2D_array(double ***arr, int rows, int cols)
{
    int i;
    (*arr) = (double **)malloc(rows * sizeof(double *));
    if (*arr == NULL)
        return 1;

    for (i = 0; i < rows; i++)
    {
        (*arr)[i] = (double *)calloc(cols, sizeof(double));

        if ((*arr)[i] == NULL)
        {
            /* Memory allocation failed, free allocated memory of current allocation */
            free_2D_array(arr, i);
            return 1;
        }
    }
    return 0;
}

/*
 * Free 2D array 'arr' dynamic memory that contains 'rows' rows.
 * Returns 0 on success.
 *
 * 'arr' - address of 2D array with 'rows' rows
 */
int free_2D_array(double ***arr, int rows)
{
    int i;
    for (i = 0; i < rows; i++)
        free((*arr)[i]);
    free((*arr));
    return 0;
}

/*
 * Prints in stdout 2D array '*M'
 * Output example:
 * 0.0011, 0.0012
 * 0.0021, 0.0022
 * 0.0031, 0.0032
 *
 * Returns 0 on success.
 *
 * 'M' - Address of 2D matrix with dimension 'rows' x 'cols'
 */
int print_matrix(double ***M, const int rows, const int cols)
{
    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            printf("%.4f", (*M)[i][j]);

            if (j != cols - 1)
                printf("%c", DELIMITER);
        }
        printf("\n");
    }
    return 0;
}

/*
 * Returns the norm2 squared of 'vector'.
 * norm2_squared(v) = v[0]^2 + v[1]^2 + ... + v[len-1]^2
 *
 * 'vector' - Address of 1D array with length 'len'.
 */
double norm2_squared(double **vector, const int len)
{
    int i;
    double norm_squared = 0;
    for (i = 0; i < len; i++)
        norm_squared += (*vector)[i] * (*vector)[i];

    return norm_squared;
}

/*
 * Calculate the vector substitution ('V1' - 'V2') and places the result vector in 'result'.
 * pre: '*result' is dynamically allocated array with the length 'len'.
 * Returns 0 on success.
 *
 * 'result', 'V1', 'V2' - Address of 1D array with length 'len'.
 */
int vector_sub(double **result, double **V1, double **V2, const int len)
{
    int i;
    for (i = 0; i < len; i++)
        (*result)[i] = (*V1)[i] - (*V2)[i];
    return 0;
}

/*
 * Calculate the matrix substitution ('M1' - 'M2') and places the result matrix in '*result'.
 * pre: '*result' is dynamically allocated matrix with dimensions 'rows' x 'cols'.
 * Returns 0 on success.
 *
 * 'result', 'M1', 'M2' - Address of 2D array with dimensions 'rows' x 'cols'.
 */
int matrix_sub(double ***result, double ***M1, double ***M2, const int rows, const int cols)
{
    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            (*result)[i][j] = (*M1)[i][j] - (*M2)[i][j];
        }
    }
    return 0;
}

/*
 * Copy the matrix 'copyFrom' to the matrix 'copyTo'.
 * pre: '*copyTo' is dynamically allocated with the dimensions 'rows' x 'cols'
 * Returns 0 on success.
 *
 * 'copyTo', 'copyFrom' - Address of 2D array with dimensions 'rows' x 'cols'
 */
int copy_matrix(double ***copyTo, double ***copyFrom, const int rows, const int cols)
{
    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            (*copyTo)[i][j] = (*copyFrom)[i][j];
        }
    }
    return 0;
}

/*
 * Calculate (M,D) matrix inner product and place it in '*result'.
 * pre: '*result' is NOT dynamically allocated.
 * Returns 0 on success.
 *
 * 'M' - Address of 2D array with dimensions 'rows' x 'cols'.
 * 'D' - Address of 1D array that represents the diagonal of a diagonal matrix with dimensions 'N' x 'N'.
 * diag_on_right_flag - 1 for (M,D) inner product, 0 for (D,M) inner product.
 */
int mul_diag_matrix(double ***result, double ***M, double **D, const int N, const int diag_on_right_flag)
{
    int i, j;

    if (allocate_2D_array(result, N, N) != 0)
        return 1;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
        {

            if (diag_on_right_flag == 1)
                /*  (MD)_ij = m_ij * d_j  */
                (*result)[i][j] = (*M)[i][j] * (*D)[j];

            else if (diag_on_right_flag == 0)
                /*  (DM)_ij = d_i * m_ij  */
                (*result)[i][j] = (*M)[i][j] * (*D)[i];
        }
    }
    return 0;
}

/*
 * Calculate the squared matrix '*D' raised by power 'power' and place it in '*result'.
 * pre: '*result' is NOT dynamically allocated.
 * Returns 0 on success.
 *
 * 'D' - Address of 1D array that represents the diagonal of a diagonal matrix with dimensions 'N' x 'N'.
 */
int pow_diag_matrix(double **result, double **D, const int N, const double power)
{
    int i;
    *result = (double *)malloc(N * sizeof(double));
    if (*result == NULL)
        return 1;

    for (i = 0; i < N; i++)
        (*result)[i] = pow((*D)[i], power);

    return 0;
}

/*
 * Calculate (A,B) matrix inner product and place it in '*result'.
 * pre: '*result' is NOT dynamically allocated.
 * Returns 0 on success.
 *
 * C == (A,B) iff for every i,j in [0,N): c_ij == Σ a_i0*b_kj + a_i0*b_kj
 *
 * 'A' - Address of 2D array with dimensions 'rows_A' x 'cols_A'.
 * 'B' - Address of 2D array with dimensions 'rows_B' x 'cols_B'.
 */
int mul_matrix(double ***result, double ***A, const int rows_A, const int cols_A,
               double ***B, const int rows_B, const int cols_B)
{
    int i, j, k, N;
    if (cols_A != rows_B)
        return 1;
    N = cols_A;

    if (allocate_2D_array(result, rows_A, cols_B) != 0)
        return 1;

    for (i = 0; i < rows_A; i++)
    {
        for (j = 0; j < cols_B; j++)
        {
            for (k = 0; k < N; k++)
            {
                (*result)[i][j] += (*A)[i][k] * (*B)[k][j];
            }
        }
    }

    return 0;
}

/*
 * Calculate the transposed matrix of '*M' and place it in '*result'.
 * pre: '*result' is NOT dynamically allocated.
 * Returns 0 on success.
 *
 * T == M_T iff for every i,j in [0,N): t_ij == m_ji
 *
 * 'M' - Address of 2D array with dimensions 'rows' x 'cols'.
 */
int transpose(double ***result, double ***M, const int rows, const int cols)
{
    int i, j;
    if (allocate_2D_array(result, cols, rows) != 0)
        return 1;

    for (i = 0; i < cols; i++)
    {
        for (j = 0; j < rows; j++)
        {
            (*result)[i][j] = (*M)[j][i];
        }
    }
    return 0;
}

/*
 * Returns the squared frobenius norm of the matrix '*M'.
 *
 * F_norm_squared(A) == ΣΣ |a_ij|^2
 *
 * 'M' - Address of 2D array with dimensions 'rows' x 'cols'.
 */
double F_norm_squared(double ***M, const int rows, const int cols)
{
    int i, j;
    double norm_squared;
    norm_squared = 0;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            norm_squared += pow((*M)[i][j], 2);
        }
    }
    return norm_squared;
}

/*
 * Parsing a diagonal matrix '*D' represented by 1D array to presentation by 2D array and place it in '*M'.
 * pre: '*M' is NOT dynamically allocated.
 * Returns 0 on success.
 *
 * 'D' - Address of 1D array that represents the diagonal of a diagonal matrix with dimensions 'N' x 'N'.
 */
int parse_diag_to_matrix_form(double ***M, double **D, const int N)
{
    int i;
    if (allocate_2D_array(M, N, N) != 0)
        return 1;

    for (i = 0; i < N; i++)
        (*M)[i][i] = (*D)[i];
    return 0;
}

/*
 * Calculate the similarity matrix '*A' based on the instructions.
 * pre: '*A' is NOT dynamically allocated.
 * Returns 0 on success.
 *
 * 'X' - Address of 2D matrix that contains 'rows_X' vectors, each having a size of 'cols_X'.
 */
int C_sym(double ***A, double ***X, const int rows_X, const int cols_X)
{
    double a_ij, *temp_sub;
    int i, j, N, d;

    N = rows_X;
    d = cols_X;

    if (allocate_2D_array(A, N, N) != 0)
        return 1;

    temp_sub = (double *)malloc(d * sizeof(double));
    if (temp_sub == NULL)
        return 1;

    for (i = 0; i < N; i++)
    {
        for (j = i + 1; j < N; j++)
        {
            vector_sub(&temp_sub, &((*X)[i]), &((*X)[j]), d);
            a_ij = exp(-0.5 * norm2_squared(&temp_sub, d));

            (*A)[i][j] = a_ij;
            (*A)[j][i] = a_ij;
        }
    }
    free(temp_sub);

    return 0;
}

/*
 * Calculate the diagonal degree matrix '*D' based on the instructions.
 * pre: '*D' is NOT dynamically allocated.
 * Returns 0 on success.
 *
 * 'A' - Address of a similarity matrix with dimensions 'N' x 'N'.
 */
int C_ddg(double **D, double ***A, const int N)
{
    /* D is an address of 1D array that represents a diagonal matrix */
    int i, j;
    (*D) = (double *)calloc(N, sizeof(double));
    if (*D == NULL)
        return 1;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N; j++)
            (*D)[i] += (*A)[i][j];
    }
    return 0;
}

/*
 * Calculate the normalized similarity matrix '*W' based on the instructions.
 * pre: '*W' is NOT dynamically allocated.
 * Returns 0 on success.
 *
 * 'D' - Address of a diagonal degree matrix with dimensions 'N' x 'N'.
 * 'A' - Address of a similarity matrix with dimensions 'N' x 'N'.
 */

int C_norm(double ***W, double **D, double ***A, const int N)
{
    /* Let P = D^(-1/2) */
    double *P, **temp_calc;
    pow_diag_matrix(&P, D, N, -0.5);

    if (mul_diag_matrix(&temp_calc, A, &P, N, 1) != 0)
        return 1;

    if (mul_diag_matrix(W, &temp_calc, &P, N, 0) != 0)
        return 1;

    free(P);
    free_2D_array(&temp_calc, N);
    return 0;
}

/*
 * Calculate the updated 'H_out' matrix from 'H_in' based on the instructions.
 * pre: '*H_out' is dynamically allocated.
 * Returns 0 on success.
 *
 * 'H_out', 'H_in' - Address of a 2D array with dimensions 'rows_H' x 'cols_H'.
 * 'W' - Address of a 2D array with dimensions 'N_W' x 'N_W'.
 */
int update_H(double ***H_out, double ***H_in, const int rows_H, const int cols_H,
             double ***W, const int N_W)
{

    double **NUM, **DEN_temp, **DEN, **H_T;
    int i, j;

    if (transpose(&H_T, H_in, rows_H, cols_H) != 0)
        return 1;

    /* Note: rows_H == N_W */
    /* NUM dim: N_W x cols_H == rows_H x cols_H */
    if (mul_matrix(&NUM, W, N_W, N_W, H_in, rows_H, cols_H) != 0)
    {
        free_2D_array(&H_T, cols_H);
        free_2D_array(&NUM, rows_H);
        return 1;
    }

    /* DEN_temp dim: rows_H x rows_H */
    if (mul_matrix(&DEN_temp, H_in, rows_H, cols_H, &H_T, cols_H, rows_H) != 0)
    {
        free_2D_array(&H_T, cols_H);
        free_2D_array(&NUM, rows_H);
        free_2D_array(&DEN_temp, rows_H);
        return 1;
    }

    /* DEN dim: rows_H x cols_H */
    if (mul_matrix(&DEN, &DEN_temp, rows_H, rows_H, H_in, rows_H, cols_H) != 0)
    {
        free_2D_array(&H_T, cols_H);
        free_2D_array(&NUM, rows_H);
        free_2D_array(&DEN_temp, rows_H);
        free_2D_array(&DEN, rows_H);
        return 1;
    }

    /* Calculate H_out */
    /* H_out dim == DEN dim == NUM dim */
    for (i = 0; i < rows_H; i++)
    {
        for (j = 0; j < cols_H; j++)
        {

            if (DEN[i][j] == 0)
            {
                /* Division by zero */
                free_2D_array(&H_T, cols_H);
                free_2D_array(&NUM, rows_H);
                free_2D_array(&DEN_temp, rows_H);
                free_2D_array(&DEN, rows_H);
                return 1;
            }
            (*H_out)[i][j] = (*H_in)[i][j] * (1 - BETA + BETA * (NUM[i][j] / DEN[i][j]));
        }
    }

    free_2D_array(&NUM, rows_H);
    free_2D_array(&DEN_temp, rows_H);
    free_2D_array(&DEN, rows_H);
    free_2D_array(&H_T, cols_H);

    return 0;
}

/*
 * Calculate the optimal 'H_out' matrix from the initial 'H_in' based on the instructions.
 * pre: '*H_out' is NOT dynamically allocated.
 * Returns 0 on success.
 *
 * 'H_out', 'H_in' - Address of a 2D array with dimensions 'rows_H' x 'cols_H'.
 * 'W' - Address of a 2D array with dimensions 'N_W' x 'N_W'.
 */
int C_symnmf(double ***H_out, double ***H_in, const int rows_H, const int cols_H,
             double ***W, const int N_W)
{
    int i;
    double F_norm_s_val;
    double **SUB;

    if (allocate_2D_array(&SUB, rows_H, cols_H) != 0)
        return 1;
    if (allocate_2D_array(H_out, rows_H, cols_H) != 0)
    {
        free_2D_array(&SUB, rows_H);
        return 1;
    }

    F_norm_s_val = EPS + 1; /* Initial value */
    i = 0;

    while (i < MAX_ITER && F_norm_s_val >= EPS)
    {

        if (update_H(H_out, H_in, rows_H, cols_H, W, N_W) != 0)
        {
            /* Division by zero */
            free_2D_array(&SUB, N_W);
            free_2D_array(H_out, N_W);

            return 1;
        }
        matrix_sub(&SUB, H_out, H_in, rows_H, cols_H);
        F_norm_s_val = F_norm_squared(&SUB, rows_H, cols_H);
        copy_matrix(H_in, H_out, rows_H, cols_H);

        i++;
    }

    free_2D_array(&SUB, N_W);
    return 0;
}

/*
 * Finds the rows and columns of the data inside 'file_name' and place it on '*rows' and '*cols' respectively.
 * Returns 0 on success.
 *
 * 'file_name' - Address of 1D char array that represents the name of a file.
 * 'rows', 'cols' - Address for an integer variable.
 */
int get_file_rows_cols(char **file_name, int *rows, int *cols)
{
    FILE *file;
    int c;
    double fake_num;
    file = fopen(*file_name, "r");
    if (file == NULL)
        return 1;

    (*rows) = 0;
    (*cols) = 0;
    while (fscanf(file, "%lf", &fake_num) != EOF)
    {
        c = fgetc(file);

        if ((*rows) == 0)
            (*cols)++;

        if (c == '\n' || c == EOF)
            (*rows)++;
    }
    fclose(file);
    return 0;
}

/*
 * Read the data points from 'file_name' and place them into '*X'.
 * pre: '*X' is NOT dynamically allocated.
 * returns 0 on success.
 *
 * 'file_name' - Address of 1D char array that represents the name of a file.
 * 'rows', 'cols' - Address for an integer variable.
 */
int read_file(double ***X, char **file_name, int *rows, int *cols)
{
    FILE *file;
    int c, row, col;
    if (get_file_rows_cols(file_name, rows, cols) != 0)
        return 1;

    if (allocate_2D_array(X, *rows, *cols) != 0)
        return 1;

    file = fopen(*file_name, "r");
    if (file == NULL)
        return 1;

    row = 0;
    col = 0;
    /* read float number and then read ',' or '\n'*/
    while (fscanf(file, "%lf", &((*X)[row][col++])) != EOF)
    {
        c = fgetc(file);
        if (c == '\n')
        {
            row++;
            col = 0;
        }
        else if (c != DELIMITER && c != EOF)
        {
            free_2D_array(X, *rows);
            return 1;
        }
    }
    fclose(file);

    return 0;
}

/* Main program
 * Print the requested matrix by the 'goal'.
 * Expected argv: [{Program Name}, {'goal'}, {'file_name'}]
 * Returns 0 on success.
 *
 * 'goal' - "string" that equals to one of the following: ["sym", "ddg", "norm"]
 * 'file_name' - "string" of an existing file in the project folder that contains data point by the format:
 * 1.111111,2.2222222
 * 3.333333333,4.4444444
 * 5.0,6.6666
 */
int main(int argc, char *argv[])
{
    char *goal, *file_name;
    double **X, **A, *D, **D_out, **W;
    int N, d;

    if (argc != 3)
    {
        printf("%s", ERR_MSG);
        return 1;
    }

    goal = argv[1];
    file_name = argv[2];

    if (read_file(&X, &file_name, &N, &d) != 0)
    {
        printf("%s", ERR_MSG);
        return 1;
    }
    /* if goal in {'sym', 'ddg', 'norm'}, the method gets the suitable matrix and prints it. */
    if (strcmp(goal, SYM) == 0)
    {
        C_sym(&A, &X, N, d);

        print_matrix(&A, N, N);

        free_2D_array(&A, N);
    }
    else if (strcmp(goal, DDG) == 0)
    {
        C_sym(&A, &X, N, d);
        C_ddg(&D, &A, N);
        parse_diag_to_matrix_form(&D_out, &D, N);

        print_matrix(&D_out, N, N);

        free_2D_array(&A, N);
        free(D);
        free_2D_array(&D_out, N);
    }
    else if (strcmp(goal, NORM) == 0)
    {
        C_sym(&A, &X, N, d);
        C_ddg(&D, &A, N);
        C_norm(&W, &D, &A, N);

        print_matrix(&W, N, N);

        free_2D_array(&A, N);
        free(D);
        free_2D_array(&W, N);
    }
    else
    {
        printf("%s", ERR_MSG);
        free_2D_array(&X, N);
        return 1;
    }

    free_2D_array(&X, N);

    return 0;
}
