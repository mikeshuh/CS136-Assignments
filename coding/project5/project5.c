#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"

#define MAX_ITERATIONS 100
#define TOLERANCE 1e-4

//----------------------------------------------convolve------------------------------------------------------

// rotate matrix helper function for convolve
Matrix createRotatedMatrix180(const Matrix *m) {
    Matrix rotatedMatrix = createMatrix(m->height, m->width);
    for (int i = 0; i < m->height; i++) {
        for (int j = 0; j < m->width; j++) {
            rotatedMatrix.map[i][j] = m->map[m->height - i - 1][m->width - j - 1];
        }
    }
    return rotatedMatrix;
}

// m1 input img matrix; m2 filter matrix
Matrix convolve(Matrix m1, Matrix m2) {
    Matrix convolvedMatrix = createMatrix(m1.height, m1.width);      // result matrix
    Matrix rotatedFilter = createRotatedMatrix180(&m2);             // filter rotated

    // coordinates of filter anchor
    int yFilterAnchor = (m2.height - 1) / 2;
    int xFilterAnchor = (m2.width - 1) / 2;

    // iterate over img matrix
    for (int i = 0; i < m1.height - m2.height + 1; i++) {
        for (int j = 0; j < m1.width - m2.width + 1; j++) {
            double sum = 0;

            // itereate over rotated filter; keep track of sum of img & filter products
            for (int k = 0; k < m2.height; k++) {
                for (int l = 0; l < m2.width; l++) {
                    sum += m1.map[i + k][j + l] * rotatedFilter.map[k][l];
                }
            }

            // store sum in result matrix
            convolvedMatrix.map[i + yFilterAnchor][j + xFilterAnchor] = sum;
        }
    }

    deleteMatrix(rotatedFilter);    // free memory

    return convolvedMatrix;
}

// Define Law's 1D filters
const double L5[5] = {1, 4, 6, 4, 1};
const double E5[5] = {-1, -2, 0, 2, 1};
const double S5[5] = {-1, 0, 2, 0, -1};
const double R5[5] = {1, -4, 6, -4, 1};
const double W5[5] = {-1, 2, 0, -2, 1};

// Create a 5x5 Law's filter from two 1D vectors
Matrix createLawsFilter(const double *vector1, const double *vector2) {
    Matrix filter = createMatrix(5, 5);
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            filter.map[i][j] = vector1[i] * vector2[j];
        }
    }
    return filter;
}

// Apply all 25 Law's filters to the input image
void applyLawsFilters(Matrix inputMatrix, Matrix *filteredMatrices) {
    const double *filters[5] = {L5, E5, S5, R5, W5};
    int index = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            Matrix filter = createLawsFilter(filters[i], filters[j]);
            filteredMatrices[index] = convolve(inputMatrix, filter);
            deleteMatrix(filter);
            index++;
        }
    }
}

// Compute texture energy for each filtered image
Matrix computeTextureEnergy(Matrix filteredMatrix, int neighborhoodSize) {
    Matrix energyMatrix = createMatrix(filteredMatrix.height, filteredMatrix.width);
    int offset = neighborhoodSize / 2;
    double minVal = DBL_MAX, maxVal = -DBL_MAX;

    // Compute the sum of absolute values within the neighborhood and find min and max values
    for (int i = offset; i < filteredMatrix.height - offset; i++) {
        for (int j = offset; j < filteredMatrix.width - offset; j++) {
            double sum = 0;
            for (int m = -offset; m <= offset; m++) {
                for (int n = -offset; n <= offset; n++) {
                    sum += fabs(filteredMatrix.map[i + m][j + n]);
                }
            }
            energyMatrix.map[i][j] = sum;
            if (sum < minVal) minVal = sum;
            if (sum > maxVal) maxVal = sum;
        }
    }

    // Normalize the energy matrix to the range [0, 255]
    for (int i = 0; i < energyMatrix.height; i++) {
        for (int j = 0; j < energyMatrix.width; j++) {
            energyMatrix.map[i][j] = ((energyMatrix.map[i][j] - minVal) / (maxVal - minVal)) * 255.0;
        }
    }

    return energyMatrix;
}



// Function to compute Euclidean distance between two feature vectors
double euclideanDistance(double *vec1, double *vec2, int size) {
    double sum = 0;
    for (int i = 0; i < size; i++) {
        sum += (vec1[i] - vec2[i]) * (vec1[i] - vec2[i]);
    }
    return sqrt(sum);
}

// Function to initialize cluster centroids randomly but ensure they are distinct
void initializeCentroids(double **centroids, double ***featureVectors, int height, int width, int segments, int featureVectorSize) {
    for (int k = 0; k < segments; k++) {
        int distinct = 0;
        while (!distinct) {
            distinct = 1;
            int i = rand() % height;
            int j = rand() % width;
            for (int f = 0; f < featureVectorSize; f++) {
                centroids[k][f] = featureVectors[i][j][f];
            }
            // Ensure the centroids are distinct from previously initialized centroids
            for (int m = 0; m < k; m++) {
                if (euclideanDistance(centroids[m], centroids[k], featureVectorSize) < TOLERANCE) {
                    distinct = 0;
                    break;
                }
            }
        }
    }
}



// K-means clustering implementation
void kMeansClustering(double ***featureVectors, int *labels, double **centroids, int height, int width, int segments, int featureVectorSize) {
    int changed;
    int iterations = 0;
    do {
        changed = 0;

        // Assign each pixel to the closest centroid
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                double minDistance = euclideanDistance(featureVectors[i][j], centroids[0], featureVectorSize);
                int minIndex = 0;
                for (int k = 1; k < segments; k++) {
                    double distance = euclideanDistance(featureVectors[i][j], centroids[k], featureVectorSize);
                    if (distance < minDistance) {
                        minDistance = distance;
                        minIndex = k;
                    }
                }
                if (labels[i * width + j] != minIndex) {
                    labels[i * width + j] = minIndex;
                    changed = 1;
                }
            }
        }

        // Update centroids
        double **newCentroids = (double **) malloc(segments * sizeof(double *));
        int *count = (int *) calloc(segments, sizeof(int));
        for (int k = 0; k < segments; k++) {
            newCentroids[k] = (double *) calloc(featureVectorSize, sizeof(double));
        }

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int label = labels[i * width + j];
                for (int f = 0; f < featureVectorSize; f++) {
                    newCentroids[label][f] += featureVectors[i][j][f];
                }
                count[label]++;
            }
        }

        for (int k = 0; k < segments; k++) {
            if (count[k] > 0) {
                for (int f = 0; f < featureVectorSize; f++) {
                    newCentroids[k][f] /= count[k];
                }
            }
        }

        // Replace old centroids with new centroids
        for (int k = 0; k < segments; k++) {
            for (int f = 0; f < featureVectorSize; f++) {
                centroids[k][f] = newCentroids[k][f];
            }
            free(newCentroids[k]);
        }
        free(newCentroids);
        free(count);

        iterations++;
    } while (changed && iterations < MAX_ITERATIONS);
}

// Segment the texture using Law's filters and clustering
Image segmentTexture(Image inputImg, int segments) {
    // Convert image to grayscale matrix
    Matrix inputMatrix = image2Matrix(inputImg);

    // Apply Law's filters to get 25 filtered matrices
    Matrix filteredMatrices[25];
    applyLawsFilters(inputMatrix, filteredMatrices);

    // Compute texture energy for each filtered matrix
    Matrix energyMatrices[25];
    for (int i = 0; i < 25; i++) {
        energyMatrices[i] = computeTextureEnergy(filteredMatrices[i], 15); // Using 15x15 neighborhood
        deleteMatrix(filteredMatrices[i]); // Free filtered matrix memory
    }

    // Combine texture energy matrices into feature vectors
    int featureVectorSize = 25;
    double ***featureVectors = (double ***) malloc(inputMatrix.height * sizeof(double **));
    for (int i = 0; i < inputMatrix.height; i++) {
        featureVectors[i] = (double **) malloc(inputMatrix.width * sizeof(double *));
        for (int j = 0; j < inputMatrix.width; j++) {
            featureVectors[i][j] = (double *) malloc(featureVectorSize * sizeof(double));
            for (int k = 0; k < featureVectorSize; k++) {
                featureVectors[i][j][k] = energyMatrices[k].map[i][j];
            }
        }
    }

    // Initialize centroids for K-means
    double **centroids = (double **) malloc(segments * sizeof(double *));
    for (int k = 0; k < segments; k++) {
        centroids[k] = (double *) malloc(featureVectorSize * sizeof(double));
    }
    initializeCentroids(centroids, featureVectors, inputMatrix.height, inputMatrix.width, segments, featureVectorSize);

    // Perform K-means clustering
    int *labels = (int *) malloc(inputMatrix.height * inputMatrix.width * sizeof(int));
    kMeansClustering(featureVectors, labels, centroids, inputMatrix.height, inputMatrix.width, segments, featureVectorSize);

    // Create output segmented image
    Image outputImg = createImage(inputImg.height, inputImg.width);
    for (int i = 0; i < inputImg.height; i++) {
        for (int j = 0; j < inputImg.width; j++) {
            int label = labels[i * inputImg.width + j];
            // Assign color based on label
            switch (label) {
                case 0: setPixel(outputImg, i, j, 255, 0, 0, NO_CHANGE); break; // Red
                case 1: setPixel(outputImg, i, j, 0, 255, 0, NO_CHANGE); break; // Green
                case 2: setPixel(outputImg, i, j, 0, 0, 255, NO_CHANGE); break; // Blue
                case 3: setPixel(outputImg, i, j, 255, 255, 0, NO_CHANGE); break; // Yellow
                case 4: setPixel(outputImg, i, j, 0, 255, 255, NO_CHANGE); break; // Cyan
                default: setPixel(outputImg, i, j, 255, 0, 255, NO_CHANGE); break; // Magenta
            }
        }
    }

    // Free energy matrices, feature vectors, centroids, and labels
    for (int i = 0; i < 25; i++) {
        deleteMatrix(energyMatrices[i]);
    }
    for (int i = 0; i < inputMatrix.height; i++) {
        for (int j = 0; j < inputMatrix.width; j++) {
            free(featureVectors[i][j]);
        }
        free(featureVectors[i]);
    }
    free(featureVectors);
    for (int k = 0; k < segments; k++) {
        free(centroids[k]);
    }
    free(centroids);
    free(labels);
    deleteMatrix(inputMatrix);

    return outputImg;
}

int main(int argc, const char *argv[]) {
    // Segment each image in the "textures" folder
    for (int imgIndex = 1; imgIndex <= 20; imgIndex++) {
        char inputFilename[50];
        char outputFilename[50];

        // Prepare input and output filenames
        snprintf(inputFilename, sizeof(inputFilename), "textures/%d.pgm", imgIndex);
        snprintf(outputFilename, sizeof(outputFilename), "textures/output_%d.ppm", imgIndex);

        // Load input image
        Image inputImg = readImage(inputFilename);
        
        // Segment texture with specified number of segments
        Image outputImg = segmentTexture(inputImg, 5);

        // Save output image
        writeImage(outputImg, outputFilename); // Save as PPM for colored output

        // Clean up
        deleteImage(inputImg);
        deleteImage(outputImg);

        printf("Processed %s -> %s\n", inputFilename, outputFilename);
    }

    printf("\nAll images processed. Program ends...\n\n");
    return 0;
}
