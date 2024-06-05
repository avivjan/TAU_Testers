import sys
import numpy as np
import symnmfmodule as s  # C module

# global arguments
goal_list = ['symnmf', 'sym', 'ddg', 'norm']

# initializing the random function
np.random.seed(0)


def createDVectors(file):
    """
    Create the N Points from the given file
    :param file: opened input file
    :type file: TextIOWrapper
    :return: list of lists represents the vector in file
    :rtype: list of lists (size: N*d)
    """
    dVectors = []
    for line in file:
        vector = line.splitlines()[0].split(",")
        numVector = [eval(i) for i in vector]  # casting from string to number
        dVectors.append(numVector)
    return dVectors


# function to handle errors
def handleError():
    """
    function that prints error message and quit the program
    :return: void
    """
    print("An Error Has Occurred")
    exit()


def symnmf(X, k, N):
    """
    symnmf function that calls to the symnmf C function
    :param X: input vectors in a matrix form
    :type X: list of lists
    :param k: number of clusters
    :type k: int
    :param N: number of vectors in X
    :type N: int
    :return: symnmf matrix
    :rtype: list of lists (size: N*k)
    """
    W = norm(X)
    m = np.mean(np.array(W))
    initial_h = []
    #H = np.random.uniform(0, 2*np.sqrt(m/k), (N, k)).tolist()
    for i in range(N):
        initial_h.append([])
        for j in range(k):
            initial_h[i].append(2 * np.sqrt(m / k) * np.random.uniform())
    return s.symnmf(initial_h, W)


def sym(X):
    """
    sym function that calls to the sym C function
    :param X: input vectors in a matrix form
    :type X: list of lists
    :return: sym matrix
    :rtype: list of lists (size: N*N)
    """
    return s.sym(X)


def ddg(X):
    """
    ddg function that calls to the ddg C function
    :param X: input vectors in a matrix form
    :type X: list of lists
    :return: ddg matrix
    :rtype: list of lists (size: N*N)
    """
    return s.ddg(X)


def norm(X):
    """
    norm function that calls to the norm C function
    :param X: input vectors in a matrix form
    :type X: list of lists
    :return: norm matrix
    :rtype: list of lists (size: N*N)
    """
    return s.norm(X)


def printMat(mat):
    """
    prints the matrix in the specified form
    :param mat: matrix of floats to be printed
    :type mat: list of lists
    :return: void
    """
    for row in mat:
        formattedRow = ["%.4f" % num for num in row]
        print(*formattedRow, sep=",")


if __name__ == "__main__":
    """
    performs symNMF (symmetric Non-negative Matrix Factorization and prints the result
    """
    if len(sys.argv) != 4:  # missing argument
        handleError()

    # Initializing arguments
    try:
        K = int(sys.argv[1])
        goal = str(sys.argv[2])
        filename = str(sys.argv[3])
    except:
        handleError()

    if goal not in goal_list:  # check if goal is in the list of allowed values
        handleError()

    if not filename.endswith('.txt'):  # check filename extension
        handleError()

    with open(filename, 'r') as file:
        N = len(file.readlines())
        if 1 >= K or K >= N:
            handleError()

        file.seek(0)  # returning to the beginning of the file

        X = createDVectors(file)

    if goal == goal_list[0]:
        resMat = eval(goal)(X, K, N)
    else:
        resMat = eval(goal)(X)

    if resMat is None:
        handleError()

    printMat(resMat)
