// 
// project2.c
// CS136
// 
// project2
// 
// created by Michael Huh 9/25/24
// last modified 9/28/24
// 
// compile:
// gcc -o main project2.c netpbm.c
// 
// run:
// ./main
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"

// smoothing filter
#define SMOOTHING_FILTER_HEIGHT 5
#define SMOOTHING_FILTER_WIDTH 5

// median filter
#define MEDIAN_FILTER_HEIGHT 5
#define MEDIAN_FILTER_WIDTH 5

// function prototypes
void printMatrix(const Matrix *);
Matrix smoothing_filter(Matrix, Matrix);
Matrix median_filter(Matrix, Matrix);

int main(int argc, const char *argv[]) {
    // input img
    Image inputImage = readImage("grayscaleCar.pgm");
    // input img -> matrix
    Matrix inputMatrix = image2Matrix(inputImage);

    // input img w noise
    Image inputNoiseImage = readImage("noiseGrayscaleCar.pgm");
    // input img w noise -> matrix
    Matrix inputNoiseMatrix = image2Matrix(inputNoiseImage);

    //---------------------------------------------------------------------smoothing filters-----------------------------------------------------------------------------

        // 5x5 discrete gaussian filter
        // double smoothingFilterArray[SMOOTHING_FILTER_HEIGHT][SMOOTHING_FILTER_WIDTH] = {{(double)1/273, (double)4/273, (double)7/273, (double)4/273, (double)1/273},
        //                                                                                 {(double)4/273, (double)16/273, (double)26/273, (double)16/273, (double)4/273},
        //                                                                                 {(double)7/273, (double)26/273, (double)41/273, (double)26/273, (double)7/273},
        //                                                                                 {(double)4/273, (double)16/273, (double)26/273, (double)16/273, (double)4/273},
        //                                                                                 {(double)1/273, (double)4/273, (double)7/273, (double)4/273, (double)1/273}};

        // 5x5 averaging filter
        double smoothingFilterArray[SMOOTHING_FILTER_HEIGHT][SMOOTHING_FILTER_WIDTH] = {{(double)1/25, (double)1/25, (double)1/25, (double)1/25, (double)1/25},
                                                                                        {(double)1/25, (double)1/25, (double)1/25, (double)1/25, (double)1/25},
                                                                                        {(double)1/25, (double)1/25, (double)1/25, (double)1/25, (double)1/25},
                                                                                        {(double)1/25, (double)1/25, (double)1/25, (double)1/25, (double)1/25},
                                                                                        {(double)1/25, (double)1/25, (double)1/25, (double)1/25, (double)1/25}};

        Matrix smoothingFilter = createMatrixFromArray(&smoothingFilterArray[0][0], SMOOTHING_FILTER_HEIGHT, SMOOTHING_FILTER_WIDTH);
        printf("Smoothing Filter: %d x %d\n", SMOOTHING_FILTER_HEIGHT, SMOOTHING_FILTER_WIDTH);
        printMatrix(&smoothingFilter);

    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------median filter---------------------------------------------------------------------------------

        Matrix medianFilter = createMatrix(MEDIAN_FILTER_HEIGHT, MEDIAN_FILTER_WIDTH);
        printf("Median Filter: %d x %d\n", MEDIAN_FILTER_HEIGHT, MEDIAN_FILTER_WIDTH);

    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------

    // apply smooth filter to input matrix
    Matrix smoothedMatrix = smoothing_filter(inputMatrix, smoothingFilter);
    // convert smoothed matrix to img
    Image smoothedImage = matrix2Image(smoothedMatrix, 0, 1);

    // apply median filter to input noise matrix
    Matrix medianFilteredMatrix = median_filter(inputNoiseMatrix, medianFilter);
    // convert median filtered matrix to img
    Image medianFilteredImage = matrix2Image(medianFilteredMatrix, 0, 1);

    // write smoothed and median filtered imgs to files
    writeImage(smoothedImage, "smoothed_image.pgm");
    writeImage(medianFilteredImage, "median_filtered_image.pgm");

    // free memory; delete imgs and matricies
    deleteImage(inputImage);
    deleteMatrix(inputMatrix);
    deleteImage(inputNoiseImage);
    deleteMatrix(inputNoiseMatrix);
    deleteMatrix(smoothingFilter);
    deleteMatrix(medianFilter);
    deleteMatrix(smoothedMatrix);
    deleteImage(smoothedImage);
    deleteMatrix(medianFilteredMatrix);
    deleteImage(medianFilteredImage);

    printf("\nProgram ends ...");
    return 0;
}

//---------------------------------------------print matrix----------------------------------------------------------

void printMatrix(const Matrix *m) {
    for (int i = 0; i < m->height; i++) {
        for (int j = 0; j < m->width; j++) {
            printf("%f ", m->map[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

//----------------------------------------------smoothing filter------------------------------------------------------

// rotate matrix helper function for smoothing convolution filter
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
Matrix smoothing_filter(Matrix m1, Matrix m2) {
    Matrix smoothedMatrix = createMatrix(m1.height, m1.width);      // result matrix
    Matrix rotatedFilter = createRotatedMatrix180(&m2);             // filter rotated

    // iterate over img matrix
    for (int i = 0; i < m1.height - m2.height + 1; i++) {
        for (int j = 0; j < m1.width - m2.width + 1; j++) {
            double sum = 0;

            // itereate over rotated filter; keep track of sum
            for (int k = 0; k < m2.height; k++) {
                for (int l = 0; l < m2.width; l++) {
                    sum += m1.map[i + k][j + l] * rotatedFilter.map[k][l];
                }
            }

            // store sum in result matrix
            smoothedMatrix.map[i + (m2.height - 1) / 2][j + (m2.width - 1) / 2] = sum;
        }
    }

    deleteMatrix(rotatedFilter);    // free memory

    return smoothedMatrix;
}

//-----------------------------------------------median filter--------------------------------------------------------

// comparison function for qsort
// referenced: https://www.tutorialspoint.com/c_standard_library/c_function_qsort.htm, https://stackoverflow.com/questions/20584499/why-qsort-from-stdlib-doesnt-work-with-double-values-c
int compare(const void *a, const void *b) {
    if (*(double *)a > *(double *)b)
        return 1;
    else if (*(double *)a < *(double *)b)
        return -1;
    else
        return 0; 
}

// sort array and get median helper function for median filter
double sortAndGetMedian(double *filterArr, int filterSize){
    // sort filter array
    qsort(filterArr, filterSize, sizeof(double), compare);

    if (filterSize % 2) {
        return filterArr[filterSize / 2];
    }
    return 0.5 * (filterArr[filterSize / 2 - 1] + filterArr[filterSize / 2]);
}

// m1 input image matrix; m2 filter matrix
Matrix median_filter(Matrix m1, Matrix m2) {
    Matrix medianFilteredImage = createMatrix(m1.height, m1.width);     // result matrix

    int filterSize = m2.height * m2.width;                                  // size of filter
    double *filterArr = (double *)malloc(filterSize * sizeof(double));      // filter array

    // iterate over img matrix
    for (int i = 0; i < m1.height - m2.height + 1; i++) {
        for (int j = 0; j < m1.width - m2.width + 1; j ++) {
            int idx = 0;

            // iterate over filter; put into filter array
            for (int k = 0; k < m2.height; k++) {
                for (int l = 0; l < m2.width; l++) {
                    filterArr[idx++] = m1.map[i + k][j + l];
                }
            }

            // get median and store in result matrix
            medianFilteredImage.map[i + (m2.height - 1) / 2][j + (m2.width - 1) / 2] = sortAndGetMedian(filterArr, filterSize);
        }
    }

    free(filterArr);    // free memory

    return medianFilteredImage;
}