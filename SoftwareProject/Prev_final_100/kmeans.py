import math
import sys

# Global Variables
epsilon = 0.001
K = 0
d = 0


def handleError(msg):
    print(msg)
    exit()


def createDVectors(file):
    dVectors = []
    for line in file:
        vector = line.splitlines()[0].split(",")
        numVector = [eval(i) for i in vector]  # casting from string to number
        dVectors.append(numVector)
    return dVectors


def initializeCentroids(dVectors):
    centroids = []
    for i in range(K):
        centroids.append(dVectors[i])
    return centroids


def findMin(vector, centroids):
    minDist = dDistance(vector, centroids[0])
    minCluster = 0
    for i in range(1, K):
        currDist = dDistance(vector, centroids[i])
        if currDist < minDist:
            minDist = currDist
            minCluster = i
    return minCluster


def dDistance(p, q):
    sum = 0
    for i in range(d):
        sum += math.pow((p[i] - q[i]), 2)
    return math.sqrt(sum)


def updateCentroids(currCentroids, prevCentroids, flagStop, clusters, dVectors):
    flagStop = True
    for i in range(K):
        currCentroids[i] = updateSingleCentroid(clusters[i], dVectors)

        if (not flagStop) or (dDistance(currCentroids[i], prevCentroids[i]) >= epsilon):
            flagStop = False

    return currCentroids, flagStop


def updateSingleCentroid(cluster, dVectors):
    newCentroid = []
    for i in range(d):
        sum = 0
        for vector in cluster:
            sum += dVectors[vector][i]
        newCentroid.append(sum / len(cluster))

    return newCentroid


def printCentroids(centroids):
    for centroid in centroids:
        formattedCentroid = ["%.4f" % num for num in centroid]
        print(*formattedCentroid, sep=",")


def calcCentroids(k, dVectors, iterations=300):
    global K, d
    K = k
    d = len(dVectors[0])
    currCentroids = initializeCentroids(dVectors)
    flagStop = False
    iterationsNum = 0

    # flagStop=False iff for every k, the distance between the updated centroid and the previous one is less than epsilon
    while (not flagStop) and iterationsNum < iterations:
        clusters = []
        for i in range(K):
            clusters.append([])

        prevCentroids = currCentroids.copy()

        for currVector in range(len(dVectors)):
            closestCentroid = findMin(dVectors[currVector], currCentroids)
            clusters[closestCentroid].append(currVector)

        currCentroids, flagStop = updateCentroids(currCentroids, prevCentroids, flagStop, clusters, dVectors)

        iterationsNum += 1

    return currCentroids


if __name__ == "__main__":
    if len(sys.argv) == 3:  # iter is not provided
        K = int(sys.argv[1])
        iterations = 200
        filename = str(sys.argv[2])
    elif len(sys.argv) == 4:  # iter is provided
        K = int(sys.argv[1])
        iterations = int(sys.argv[2])
        filename = str(sys.argv[3])
    else:
        print("An Error Has Occurred")
        exit()

    with open(filename, 'r') as file:

        N = len(file.readlines())
        if 1 >= K or K >= N:
            print("Invalid number of clusters!")
            exit()
        if 1 >= iterations or iterations >= 1000:
            print("Invalid maximum iteration!")
            exit()

        file.seek(0)  # returning to the beginning of the file

        dVectors = createDVectors(file)
        currCentroids = calcCentroids(K, dVectors, iterations)
        printCentroids(currCentroids)
