import sys
import kmeans
import symnmf
import numpy as np
from sklearn.metrics import silhouette_score


def handleError():
    """
    function that prints error message and quit the program
    :return: void
    """
    print("An Error Has Occurred")
    exit()


def kmeans_clusters(k, x):
    """
    function to calculate the centroids using the kmeans program
    :param k: number of clusters
    :type k: int
    :param x: input vectors
    :type x: list of lists
    :return: list of index of the associating cluster of each vector in x
    :rtype: list
    """
    centroids = kmeans.calcCentroids(k, x)
    minDistances = [kmeans.findMin(vector, centroids) for vector in x]
    return minDistances


def symnmf_clusters(k, x):
    """
    function to calculate the centroids using the symnmf program
    :param k: number of clusters
    :type k: int
    :param x: input vectors
    :type x: list of lists
    :return: list of index of the associating cluster of each vector in x
    :rtype: list
    """
    H = np.array(symnmf.symnmf(x, k, len(x)))
    return H.argmax(axis=1)


if __name__ == "__main__":
    """
    performs analysis and compare between kmeans and symnmf 
    """
    if len(sys.argv) != 3:  # missing argument
        handleError()

    # Initializing arguments
    try:
        K = int(sys.argv[1])
        filename = str(sys.argv[2])
    except:
        handleError()

    if not filename.endswith('.txt'):  # check filename extension
        handleError()

    with open(filename, 'r') as file:
        N = len(file.readlines())
        if 1 >= K or K >= N:
            handleError()

        file.seek(0)  # returning to the beginning of the file

        X = symnmf.createDVectors(file)

    kmeans_cluster = kmeans_clusters(K, X)  # calculating the centroids using kmeans
    symnmf_cluster = symnmf_clusters(K, X)  # calculating the centroids using symnmf

    print("nmf: %.4f" % silhouette_score(X, symnmf_cluster))
    print("kmeans: %.4f" % silhouette_score(X, kmeans_cluster))




