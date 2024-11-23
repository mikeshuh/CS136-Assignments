// 
// project5.c
// CS136
// 
// project5
// 
// created by Michael Huh 10/30/24
// last modified 11/16/24
// 
// compile:
// gcc -o main project5.c netpbm.c
// 
// run:
// ./main
// 

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"

//-----k means------------
#define MAX_ITERATIONS 100
#define TOLERANCE 1e-4

//-----feature vector-----------------
#define SPACIAL_COORDINATE_WEIGHT 2.25

//-----texture energy-----------------
#define TEXTURE_ENERGY_NEIGHBORHOOD 16

//-----convolve----------------------------------------------------------------------------------------------------------------

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
    Matrix convolvedMatrix = createMatrix(m1.height, m1.width);      // result matrix to store the convolved values
    Matrix rotatedFilter = createRotatedMatrix180(&m2);             // rotate filter by 180 degrees before applying convolution

    // coordinates of filter anchor, used to determine the center of the filter
    int yFilterAnchor = (m2.height - 1) / 2;
    int xFilterAnchor = (m2.width - 1) / 2;

    // iterate over img matrix to apply the filter to each possible position
    for (int i = 0; i < m1.height - m2.height + 1; i++) {
        for (int j = 0; j < m1.width - m2.width + 1; j++) {
            double sum = 0;

            // iterate over rotated filter; calculate the sum of element-wise products between filter and image patch
            for (int k = 0; k < m2.height; k++) {
                for (int l = 0; l < m2.width; l++) {
                    sum += m1.map[i + k][j + l] * rotatedFilter.map[k][l];
                }
            }

            // store the calculated sum in the corresponding position in the result matrix
            convolvedMatrix.map[i + yFilterAnchor][j + xFilterAnchor] = sum;
        }
    }

    deleteMatrix(rotatedFilter);    // free memory allocated for rotated filter

    return convolvedMatrix;
}

//-----Law's filtering---------------------------------------------------------------------------------

// define 1D Law's filters
const double L5[5] = {1, 4, 6, 4, 1};
const double E5[5] = {-1, -2, 0, 2, 1};
const double S5[5] = {-1, 0, 2, 0, -1};
const double R5[5] = {1, -4, 6, -4, 1};
const double W5[5] = {-1, 2, 0, -2, 1};

// create a 5x5 Law's filter from outer product between two 1D vectors
Matrix createLawsFilter(const double *vector1, const double *vector2) {
    Matrix filter = createMatrix(5, 5);
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            filter.map[i][j] = vector1[i] * vector2[j];
        }
    }
    return filter;
}

// apply all 25 law's filters to input image to extract different texture features
void applyLawsFilters(Matrix inputMatrix, Matrix *filteredMatrices) {
    const double *filters[5] = {L5, E5, S5, R5, W5};
    int index = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            Matrix filter = createLawsFilter(filters[i], filters[j]); // create 5x5 filter by combining two 1d vectors
            filteredMatrices[index] = convolve(inputMatrix, filter);
            deleteMatrix(filter);
            index++;
        }
    }
}

// compute texture energy for each filtered image
Matrix computeTextureEnergy(Matrix filteredMatrix, int neighborhoodSize) {
    Matrix energyMatrix = createMatrix(filteredMatrix.height, filteredMatrix.width);
    int offset = neighborhoodSize / 2;
    double minVal = DBL_MAX, maxVal = -DBL_MAX;

    // compute sum of absolute values within neighborhood and find min and max values to normalize texture energy
    for (int i = offset; i < filteredMatrix.height - offset; i++) {
        for (int j = offset; j < filteredMatrix.width - offset; j++) {
            double sum = 0;
            for (int m = -offset; m <= offset; m++) {
                for (int n = -offset; n <= offset; n++) {
                    sum += fabs(filteredMatrix.map[i + m][j + n]); // fabs = absolute val
                }
            }
            energyMatrix.map[i][j] = sum;

            // get min and max vals for normalization
            if (sum < minVal) minVal = sum;
            if (sum > maxVal) maxVal = sum;
        }
    }

    // normalize energy matrix to range [0, 255]
    for (int i = 0; i < energyMatrix.height; i++) {
        for (int j = 0; j < energyMatrix.width; j++) {
            energyMatrix.map[i][j] = ((energyMatrix.map[i][j] - minVal) / (maxVal - minVal)) * 255.0;
        }
    }

    return energyMatrix;
}

//----------------------------------------k means-------------------------------------------------------------------

// compute euclidean distance between two feature vectors
double euclideanDistance(double *vec1, double *vec2, int size) {
    double sum = 0;
    for (int i = 0; i < size; i++) {
        sum += (vec1[i] - vec2[i]) * (vec1[i] - vec2[i]);
    }
    return sqrt(sum);
}

// initialize cluster centroids randomly
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
            // ensure centroids are distinct from previous centroids
            for (int m = 0; m < k; m++) {
                if (euclideanDistance(centroids[m], centroids[k], featureVectorSize) < TOLERANCE) {
                    distinct = 0;
                    break;
                }
            }
        }
    }
}

// k-means clustering; group pixels with similar texture features
void kMeansClustering(double ***featureVectors, int *labels, double **centroids, int height, int width, int segments, int featureVectorSize) {
    int changed;
    int iterations = 0;
    do {
        changed = 0;

        // assign each pixel to closest centroid by calculating distance to all centroids
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

        // update centroids by calculating mean of all feature vectors assigned to each cluster
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

        // replace old centroids with new centroids for next iteration of k-means
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

//-----segmentation----------------------------------------------------------------------------------------------------------

// segment image by texture
Image segmentTexture(Image inputImg, int segments) {
    // convert the input image to a matrix
    Matrix inputMatrix = image2Matrix(inputImg);

    // apply all 25 Law's filters to get 25 filtered matrices representing different texture features
    Matrix filteredMatrices[25];
    applyLawsFilters(inputMatrix, filteredMatrices);

    // compute texture energy for each filtered matrix
    Matrix energyMatrices[25];
    for (int i = 0; i < 25; i++) {
        energyMatrices[i] = computeTextureEnergy(filteredMatrices[i], TEXTURE_ENERGY_NEIGHBORHOOD); // using defined neighborhood size
        deleteMatrix(filteredMatrices[i]); // free filtered matrix memory
    }

    // combine texture energy matrices into feature vectors for each pixel to represent its texture properties
    int featureVectorSize = 27; // 25 from texture energy + 2 for spatial coordinates (also normalized to values [0, 255], then multiplied by spacial coord weight)
    double ***featureVectors = (double ***) malloc(inputMatrix.height * sizeof(double **));
    for (int i = 0; i < inputMatrix.height; i++) {
        featureVectors[i] = (double **) malloc(inputMatrix.width * sizeof(double *));
        for (int j = 0; j < inputMatrix.width; j++) {
            featureVectors[i][j] = (double *) malloc(featureVectorSize * sizeof(double));
            for (int k = 0; k < 25; k++) {
                featureVectors[i][j][k] = energyMatrices[k].map[i][j];
            }
            // add normalized spatial coordinates to feature vector
            featureVectors[i][j][25] = (double)i / (double)inputMatrix.height * 255 * SPACIAL_COORDINATE_WEIGHT; // normalized row coordinate * weight
            featureVectors[i][j][26] = (double)j / (double)inputMatrix.width * 255 *  SPACIAL_COORDINATE_WEIGHT;  // normalized column coordinate * weight
        }
    }

    // initialize centroids for k-means clustering using feature vectors
    double **centroids = (double **) malloc(segments * sizeof(double *));
    for (int k = 0; k < segments; k++) {
        centroids[k] = (double *) malloc(featureVectorSize * sizeof(double));
    }
    initializeCentroids(centroids, featureVectors, inputMatrix.height, inputMatrix.width, segments, featureVectorSize);

    // perform k-means clustering
    int *labels = (int *) malloc(inputMatrix.height * inputMatrix.width * sizeof(int));
    kMeansClustering(featureVectors, labels, centroids, inputMatrix.height, inputMatrix.width, segments, featureVectorSize);

    // create output segmented image by coloring each pixel based on its cluster assignment
    Image outputImg = createImage(inputImg.height, inputImg.width);
    for (int i = 0; i < inputImg.height; i++) {
        for (int j = 0; j < inputImg.width; j++) {
            int label = labels[i * inputImg.width + j];
            // assign unique color to each cluster to visualize segmentation results
            switch (label) {
                case 0: setPixel(outputImg, i, j, 255, 0, 0, NO_CHANGE); break; // red
                case 1: setPixel(outputImg, i, j, 0, 255, 0, NO_CHANGE); break; // green
                case 2: setPixel(outputImg, i, j, 0, 0, 255, NO_CHANGE); break; // blue
                case 3: setPixel(outputImg, i, j, 255, 255, 0, NO_CHANGE); break; // yellow
                case 4: setPixel(outputImg, i, j, 0, 255, 255, NO_CHANGE); break; // cyan
                case 5: setPixel(outputImg, i, j, 255, 0, 255, NO_CHANGE); break; // magenta
                case 6: setPixel(outputImg, i, j, 128, 0, 0, NO_CHANGE); break; // dark red
                case 7: setPixel(outputImg, i, j, 0, 128, 0, NO_CHANGE); break; // dark green
                case 8: setPixel(outputImg, i, j, 0, 0, 128, NO_CHANGE); break; // dark blue
                case 9: setPixel(outputImg, i, j, 128, 128, 0, NO_CHANGE); break; // olive
                case 10: setPixel(outputImg, i, j, 128, 0, 128, NO_CHANGE); break; // purple
                case 11: setPixel(outputImg, i, j, 0, 128, 128, NO_CHANGE); break; // teal
                default: setPixel(outputImg, i, j, 128, 128, 128, NO_CHANGE); break; // gray (fallback for out-of-range labels)
            }
        }
    }

    // free memory allocated for energy matrices, feature vectors, centroids, and labels
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

//----------------------------------------------------main---------------------------------------------------------------------

int main(int argc, const char *argv[]) {
    printf("\n");

                        // num segments
    int segments[21] = {0,// idx 0 placeholder      // img #
                        3, 4, 5, 6, 7,              // 1-5
                        8, 9, 10, 11, 12,           // 6-10
                        6, 6, 6, 6, 6,              // 11-15
                        6, 6, 6, 6, 6};             // 16-20

    // segment each image in textures folder; process all provided images
    for (int imgIndex = 1; imgIndex <= 20; imgIndex++) {
        char inputFilename[50];
        char outputFilename[50];

        // prepare input and output filenames
        snprintf(inputFilename, sizeof(inputFilename), "textures/%d.pgm", imgIndex);
        snprintf(outputFilename, sizeof(outputFilename), "segmented_textures/output_%d.ppm", imgIndex);

        // load input image from file
        Image inputImg = readImage(inputFilename);
        
        // segment input image with specified number of segments (clusters)
        Image outputImg = segmentTexture(inputImg, segments[imgIndex]);

        // save output segmented image to file
        writeImage(outputImg, outputFilename); // save as ppm

        // free memory
        deleteImage(inputImg);
        deleteImage(outputImg);

        printf("Processed %s -> %s\n", inputFilename, outputFilename);
    }

    printf("\nAll images processed. Program ends...\n\n");
    return 0;
}