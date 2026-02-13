#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <string.h>

// dynamic 2D array for data points "all of them will be stored here"
double (*dataMatrix)[2];
// number of data points read from file "will help us with edge cases and loops"
int numDataPoints;


/*
    Function to ask the user if they want to assign initial centroid positions
    - if yes, it will read the positions into an array and return it
    - if no, it will return NULL
    - the function will allocate memory for the array if needed, and the caller is responsible for freeing it
    - the function will handle memory allocation errors
    - no input should be empty for a yes answer
    - the function will validate that the input positions are within the range of data points
    - if invalid input is detected, it will print an error message and exit the program
*/
int* askUserForIndexes(int numClusters) {
    int *userIndexes = NULL;
    char answer;
    int ch;  

    if (numClusters <= 0 || numDataPoints <= 0) {goto FAIL;}

    while (1) {
        printf("Do you want to choose initial centroid positions? (y/n): ");
        if (scanf(" %c", &answer) != 1) {
            while ((ch = getchar()) != '\n' && ch != EOF) {} // clear junk
            continue;
        }
        if (answer=='y' || answer=='Y' || answer=='n' || answer=='N') break;
        fprintf(stderr, "Please answer 'y' or 'n'.\n");
        while ((ch = getchar()) != '\n' && ch != EOF) {}
    }

    if (answer=='n' || answer=='N') {
        return NULL;
    }

    userIndexes = malloc(numClusters * sizeof *userIndexes);
    if (!userIndexes) goto FAIL;

    int i = 0;
    while (i < numClusters) {
        int indexValue;
        printf("Enter centroid index %d of %d (0..%d): ",
               i + 1, numClusters, numDataPoints - 1);

        if (scanf("%d", &indexValue) != 1) {
            fprintf(stderr, "Not an integer. Try again.\n");
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue; 
        }
        if (indexValue < 0 || indexValue >= numDataPoints) {
            fprintf(stderr, "Out of range (valid 0..%d). Try again.\n", numDataPoints - 1);
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue; 
        }

        userIndexes[i++] = indexValue;    
        while ((ch = getchar()) != '\n' && ch != EOF) {} 
    }

    return userIndexes;

FAIL:
    if (userIndexes) free(userIndexes);
    fprintf(stderr, "askUserCentroidIndices: input/allocation failed.\n");
    exit(1);
}

/*
    Function will clean up allocated memory for data points
*/
void cleanup() {
    free(dataMatrix);
    dataMatrix = NULL;
    numDataPoints = 0;
}

/* Function to read data points from a file
    - the file is expected to have two columns of floating point numbers representing x and y coordinates
    - the function will store them in a dynamically allocated 2D array "dataMatrix"
    - it will also update the global variable numDataPoints to reflect the number of points read after resetting them
    - it will handle memory allocation and resizing as needed, 64 initially allocated then doubled as needed
    - prints each data point as it is read and the total number of points read at the end "will be commented out later but useful for debugging"
    - handles file opening errors and memory allocation errors
    - cleans up allocated memory in case of errors
*/
void readData(char* filename) {
    FILE* dataFile = fopen(filename, "r");
    if (!dataFile) goto FAIL;
    // printf("[FILE OPENED SUCCESSFULLY !]\n");
    double x, y;
    int capacity = 64;

    numDataPoints = 0;
    free(dataMatrix);
    dataMatrix = NULL;

    dataMatrix = malloc(capacity * sizeof *dataMatrix);
    if (!dataMatrix) goto FAIL;

    while (fscanf(dataFile, "%lf %lf", &x, &y) == 2) {
        if (numDataPoints >= capacity) {
            capacity *= 2;
            double (*tmp)[2] = realloc(dataMatrix, capacity * sizeof *dataMatrix);
            if (!tmp) goto FAIL;
            dataMatrix = tmp;
        }

        dataMatrix[numDataPoints][0] = x;
        dataMatrix[numDataPoints][1] = y;

        // printf("%lf %lf\n", dataMatrix[numDataPoints][0], dataMatrix[numDataPoints][1]);
        numDataPoints++;
    }

    fclose(dataFile);
    // printf("Total data points read: %d\n", numDataPoints);
    return;

FAIL:
    if (dataFile) fclose(dataFile);
    if (dataMatrix) cleanup();
    fprintf(stderr, "Error reading data file (check for a typo in the filename): %s\n", filename);
    exit(1);
}

/* Function to initialize centroids for K-means clustering
    - takes the number of clusters (numClusters) and an array to store the indexes of chosen centroids "indexes are either a userinput or randomly chosen"
    - if the array is NULL, it will randomly select unique centroidIndexes from the data points
    - checks for edge cases where numClusters is greater than the number of data points or less than or equal to zero
    - if invalid, prints an error message and exits the program after cleaning up allocated memory
*/
int* initializeCentroids(int numClusters, int* centroidsIndexes) {

    int* finalCentroids = NULL;
    if (centroidsIndexes == NULL) {
        int count = 0;
        centroidsIndexes = malloc(numClusters * sizeof *centroidsIndexes);

        if (!centroidsIndexes) goto FAIL;

        srand(time(NULL));
        while (count < numClusters) {
            int randomIndex = rand() % numDataPoints;
            int duplicate = 0;
            for (int j = 0; j < count; j++) {
                if (randomIndex == centroidsIndexes[j]) {
                    duplicate = 1;
                    break;
                }
            }
            if (!duplicate) {
                centroidsIndexes[count] = randomIndex;
                count++;
                finalCentroids = centroidsIndexes;
            }
        }
        return finalCentroids;
    }

    finalCentroids = malloc(numClusters * sizeof *finalCentroids);
    if (!finalCentroids) goto FAIL;
    for (int i = 0; i < numClusters; i++) {
        finalCentroids[i] = centroidsIndexes[i];
    }
    return finalCentroids;

FAIL:
    fprintf(stderr, "invalid number of clusters: %d\n", numClusters);
    fprintf(stderr, "it should be > 0 and <= number of data points: %d\n", numDataPoints);
    if(dataMatrix) cleanup();
    exit(1);
}
/* Function to compute the Euclidean distance between a data point and a centroid 
    - takes the index of the data point and the coordinates of the centroid as parameters
    - returns the computed distance as a double
*/
double euclideanDistanceToCoords(int idx, double centroidCoords[]) {
    double x = dataMatrix[idx][0] - centroidCoords[0];
    double y = dataMatrix[idx][1] - centroidCoords[1];
    return sqrt(x*x + y*y);
}

/*
    Function to handle output of clustered data points to a file
    - takes a 2D array of data points with their assigned cluster labels
    - writes the data points and their cluster labels to a file named "kmeans-output.txt"
    - handles file opening errors and prints an error message if the file cannot be opened
*/
void outputHandler(double labledData[numDataPoints][3]) {

    FILE* fil = fopen("kmeans-output.txt", "w");
    if (!fil) {
        printf("Writing failed: could not open file !");
    }
    fprintf(fil, "X  Y   clusterLable\n");

    for (int i = 0; i < numDataPoints; i++) {
        fprintf(fil, "%.2lf %.2lf   %d\n", labledData[i][0], labledData[i][1], (int)labledData[i][2]);
    }

    fclose(fil);
}

/* K-means clustering implementation 
    - takes the name of the data file and the number of clusters as parameters
    - reads the data points from the file
    - initializes centroids either randomly or based on user input
    - iteratively assigns data points to the nearest centroid and updates centroid positions
    - stops when convergence is reached (no changes in cluster assignments) or after a maximum of 120 iterations
    - outputs the clustered data points to a file
    - handles edge cases such as invalid number of clusters and cleans up allocated memory
*/
void kmeansImplementation(char* dataFileName, int numClusters) {

    readData(dataFileName);

    if (numClusters <= 0 || numClusters > numDataPoints) {
        fprintf(stderr, "Nr of clusters is invalid\n");
        if (dataMatrix) cleanup();
        exit(1);
    }

    double labledData[numDataPoints][3];
    double centroids[numClusters][2];
    int* userIndexes = askUserForIndexes(numClusters);
    int* initialCentroidIndexes = initializeCentroids(numClusters, userIndexes);

    for (int i = 0; i < numClusters; i++) {
        centroids[i][0] = dataMatrix[initialCentroidIndexes[i]][0];
        centroids[i][1] = dataMatrix[initialCentroidIndexes[i]][1];
    }

    for (int i = 0; i < numDataPoints; i++) {
        labledData[i][0] = dataMatrix[i][0];
        labledData[i][1] = dataMatrix[i][1];
        labledData[i][2] = -1;
    }

    for (int i = 0; i < 120; i++) {
        int clusterSwitches = 0;

        for (int j = 0; j < numDataPoints; j++) {
            double minDistance = DBL_MAX;
            int newCluster = 0;

            for (int k = 0; k < numClusters; k++) {
                double dist = euclideanDistanceToCoords(j, centroids[k]);
                if(dist < minDistance) {
                    minDistance = dist;
                    newCluster = k;
                }
            }

            if ((int)labledData[j][2] != newCluster) {
                clusterSwitches++;
                labledData[j][2] = (double)newCluster;
            }
        }

        double clusterSums[numClusters][2]; memset(clusterSums, 0, sizeof(clusterSums));
        int clusterCounts[numClusters]; memset(clusterCounts, 0, sizeof(clusterCounts));

        for (int x = 0; x < numDataPoints; x++) {
            int clusterId = (int)labledData[x][2];
            clusterSums[clusterId][0] += labledData[x][0];
            clusterSums[clusterId][1] += labledData[x][1];
            clusterCounts[clusterId]++;
        }

        for (int x = 0; x < numClusters; x++) {
            if (clusterCounts[x] > 0) {
                centroids[x][0] = clusterSums[x][0] / clusterCounts[x];
                centroids[x][1] = clusterSums[x][1] / clusterCounts[x];
            }
        }

        if (clusterSwitches == 0) {
            printf("Num of iterations done: %d\n", i + 1);
            break;
        } if (i == 119) {
            printf("Max iterations reached: No full convergence.\n");
        }

    }

    outputHandler(labledData);

    free(initialCentroidIndexes);
    free(userIndexes);
    if (dataMatrix) cleanup();
    return;
}
