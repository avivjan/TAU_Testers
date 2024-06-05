#define PY_SSIZE_T_CLEAN

#include "Python.h"
#include <stdlib.h>
#include "symnmf.h"
#include <stdio.h>

/*
 * Parsing PyObject list of lists '*PyArray_2D' to array '*CArray_2D'.
 * pre: '*CArray_2D' is NOT dynamically allocated.
 * Return 0 on success.
 *
 * 'PyArray_2D' - Address of PyObject that represents list of lists with dimensions '*rows' x '*cols'.
 */
int parse_PyObject_to_2D_array(PyObject **PyArray_2D, double ***CArray_2D, int *rows, int *cols) {
    int i, j;
    PyObject *PyArray_1D, *element;

    *rows = PyObject_Length(*PyArray_2D);
    if (*rows == -1 || *rows == 0)
        /* An error occurred while getting the length OR the array is empty */
        return 1;

    *cols = PyObject_Length(PyList_GetItem(*PyArray_2D, 0));
    /* Assuming the all rows maintain the same length */
    if (*cols == -1)
        return 1;

    /* Build CArray_2D from PyArray_2D */
    if (allocate_2D_array(CArray_2D, *rows, *cols) != 0)
        return 1;

    for (i = 0; i < *rows; i++) {

        PyArray_1D = PyList_GetItem(*PyArray_2D, i);
        if (*cols != PyObject_Length(PyArray_1D)) {
            /*PyArray_2D must contain equal row's length for all rows*/
            free_2D_array(CArray_2D, *rows);
            return 1;
        }

        for (j = 0; j < *cols; j++) {
            element = PyList_GetItem(PyArray_1D, j);
            (*CArray_2D)[i][j] = PyFloat_AsDouble(element);
        }
    }
    return 0;
}

/*
 * Parsing 2D array '*Carray_2D' to PyObject list of lists '*PyArray_2D'.
 * Return 0 on success.
 *
 * 'CArray_2D' - Address of 2D array with dimensions 'rows' x 'cols'.
 */
int parse_2D_array_to_PyObject(PyObject **PyArray_2D, double ***CArray_2D, const int rows, const int cols) {
    PyObject *PyArray_1D, *element;
    int i, j;

    *PyArray_2D = PyList_New(rows);
    if (*PyArray_2D == NULL)
        return 1;

    for (i = 0; i < rows; i++) {
        PyArray_1D = PyList_New(cols);
        if (PyArray_1D == NULL)
            return 1;

        for (j = 0; j < cols; j++) {
            element = PyFloat_FromDouble((*CArray_2D)[i][j]);
            if (element == NULL)
                return 1;

            PyList_SET_ITEM(PyArray_1D, j, element);
        }
        PyList_SET_ITEM(*PyArray_2D, i, PyArray_1D);
    }
    return 0;
}
/*
 * Returns the optimal H matrix based on the instructions or NULL on failure.
 */
static PyObject *symnmf(PyObject *self, PyObject *args) {
    PyObject *PyH_init, *PyH_out, *PyW;
    double **CH_init, **CH_out, **CW;
    int N_W, rows_H, cols_H;


    if (!PyArg_ParseTuple(args, "OO", &PyH_init, &PyW))
        return NULL;

    if (parse_PyObject_to_2D_array(&PyH_init, &CH_init, &rows_H, &cols_H) != 0)
        return NULL;
    if (parse_PyObject_to_2D_array(&PyW, &CW, &N_W, &N_W) != 0)
        return NULL;

    if (C_symnmf(&CH_out, &CH_init, rows_H, cols_H, &CW, N_W) != 0) {
        free_2D_array(&CH_init, rows_H);
        free_2D_array(&CH_out, rows_H);
        free_2D_array(&CW, N_W);
        return NULL;
    }

    if (parse_2D_array_to_PyObject(&PyH_out, &CH_out, rows_H, cols_H) != 0)
        return NULL;

    free_2D_array(&CH_init, rows_H);
    free_2D_array(&CH_out, rows_H);
    free_2D_array(&CW, N_W);

    return PyH_out;
}

/*
 * Returns the similarity matrix based on the instructions or NULL on failure.
 */
static PyObject *sym(PyObject *self, PyObject *args) {
    PyObject *PyX, *PyA;
    double **CX, **CA;
    int rows, cols, N;

    if (!PyArg_ParseTuple(args, "O", &PyX))
        return NULL;

    if (parse_PyObject_to_2D_array(&PyX, &CX, &rows, &cols) != 0)
        return NULL;

    if (C_sym(&CA, &CX, rows, cols) != 0)
        return NULL;

    N = rows;

    parse_2D_array_to_PyObject(&PyA, &CA, N, N);

    free_2D_array(&CX, rows);
    free_2D_array(&CA, N);
    return PyA;
}

/*
 * Returns the diagonal degree matrix based on the instructions or NULL on failure.
 */
static PyObject *ddg(PyObject *self, PyObject *args) {
    PyObject *PyX, *PyD;
    double **CX, **CA, *CD, **D_out;
    int rows, cols, N;

    if (!PyArg_ParseTuple(args, "O", &PyX))
        return NULL;

    if (parse_PyObject_to_2D_array(&PyX, &CX, &rows, &cols) != 0)
        return NULL;

    if (C_sym(&CA, &CX, rows, cols) != 0)
        return NULL;

    N = rows;

    if (C_ddg(&CD, &CA, N) != 0)
        return NULL;

    if (parse_diag_to_matrix_form(&D_out, &CD, N) != 0)
        return NULL;

    parse_2D_array_to_PyObject(&PyD, &D_out, N, N);

    free_2D_array(&CX, rows);
    free_2D_array(&CA, N);
    free(CD);
    free_2D_array(&D_out, N);
    return PyD;
}

/*
 * Returns the normalized similarity matrix based on the instructions or NULL on failure.
 */
static PyObject *norm(PyObject *self, PyObject *args) {
    PyObject *PyX, *PyW;
    double **CX, **CA, *CD, **CW;
    int rows, cols, N;

    if (!PyArg_ParseTuple(args, "O", &PyX))
        return NULL;

    if (parse_PyObject_to_2D_array(&PyX, &CX, &rows, &cols) != 0)
        return NULL;

    if (C_sym(&CA, &CX, rows, cols) != 0)
        return NULL;

    N = rows;

    if (C_ddg(&CD, &CA, N) != 0)
        return NULL;

    if (C_norm(&CW, &CD, &CA, N) != 0)
        return NULL;

    parse_2D_array_to_PyObject(&PyW, &CW, N, N);

    free_2D_array(&CX, rows);
    free_2D_array(&CA, N);
    free(CD);
    free_2D_array(&CW, N);
    return PyW;
}


/* ------------ CPython API ------------ */
static PyMethodDef symnmfMethods[] = {
        {"symnmf",
                (PyCFunction) symnmf,
                     METH_VARARGS,
                PyDoc_STR("Returns the final H matrix")},
        {"sym",
                (PyCFunction) sym,
                     METH_VARARGS,
                PyDoc_STR("Returns the similarity matrix")},
        {"ddg",
                (PyCFunction) ddg,
                     METH_VARARGS,
                PyDoc_STR("Returns the Diagonal Degree Matrix")},
        {"norm",
                (PyCFunction) norm,
                     METH_VARARGS,
                PyDoc_STR("Returns the normalized similarity matrix")},

        {NULL, NULL, 0, NULL}
};

static struct PyModuleDef symnmfmodule = {
        PyModuleDef_HEAD_INIT,
        "symnmfmodule",
        NULL,
        -1,
        symnmfMethods
};

PyMODINIT_FUNC PyInit_symnmfmodule(void) {
    return PyModule_Create(&symnmfmodule);
}

