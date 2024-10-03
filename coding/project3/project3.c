// 
// project3.c
// CS136
// 
// project3
// 
// created by Michael Huh 9/30/24
// last modified 9/30/24
// 
// compile:
// gcc -o main project3.c netpbm.c
// 
// run:
// ./main
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"

// convolution filter
#define CONVOLUTION_FILTER_HEIGHT 5
#define CONVOLUTION_FILTER_WIDTH 5

// function prototypes
void printMatrix(const Matrix *);
Matrix convolve(Matrix, Matrix);
Image sobel(Image);

int main(int argc, const char *argv[]) {
    // input img
    Image inputImage = readImage("car_bw.pgm");
    // input img -> matrix
    Matrix inputMatrix = image2Matrix(inputImage);

    //---------------------------------------------------------------------convolution filter-----------------------------------------------------------------------------

    // 5x5 discrete gaussian filter
    double gaussianFilterArray[CONVOLUTION_FILTER_HEIGHT][CONVOLUTION_FILTER_WIDTH] = {{(double)1/273, (double)4/273, (double)7/273, (double)4/273, (double)1/273},
                                                                                        {(double)4/273, (double)16/273, (double)26/273, (double)16/273, (double)4/273},
                                                                                        {(double)7/273, (double)26/273, (double)41/273, (double)26/273, (double)7/273},
                                                                                        {(double)4/273, (double)16/273, (double)26/273, (double)16/273, (double)4/273},
                                                                                        {(double)1/273, (double)4/273, (double)7/273, (double)4/273, (double)1/273}};

    Matrix convolutionFilter = createMatrixFromArray(&gaussianFilterArray[0][0], CONVOLUTION_FILTER_HEIGHT, CONVOLUTION_FILTER_WIDTH);
    printf("Convolution Filter: %d x %d\n", CONVOLUTION_FILTER_HEIGHT, CONVOLUTION_FILTER_WIDTH);
    printMatrix(&convolutionFilter);

    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------

    // apply convolution filter to input matrix
    Matrix convolvedMatrix = convolve(inputMatrix, convolutionFilter);
    // convert convolved matrix to img
    Image convolvedImage = matrix2Image(convolvedMatrix, 1, 1);

    // write convolved img to files
    writeImage(convolvedImage, "convolved_image.pgm");

    Image sobelFilteredImage = sobel(convolvedImage);
    writeImage(sobelFilteredImage, "sobeltest.pgm");


    // free memory; delete imgs and matricies
    deleteImage(inputImage);
    deleteMatrix(inputMatrix);
    deleteMatrix(convolutionFilter);
    deleteMatrix(convolvedMatrix);
    deleteImage(convolvedImage);
    deleteImage(sobelFilteredImage);

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

//-------------------------------function_imageBlackWhite-------------------------------------------------
/* function that receives an Image structure and an intensity threshold
 to convert each pixel in the image to either black (intensity = 0)
 or white (intensity = 255). The function should return an Image structure
 containing the result. */

Image function_imageBlackWhite(Image img, int intensityThreshold) {
    // create new img
    Image bwImg = createImage(img.height, img.width);

    // iterate over original img
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            // if img pixel i <= intensity threshold, then pixel is obj (i = 0)
            // else, pixel is background (i = 255)
            bwImg.map[i][j].i = (img.map[i][j].i <= intensityThreshold) ? 0 : 255;

            // set r, g, b to match i
            bwImg.map[i][j].r = bwImg.map[i][j].g = bwImg.map[i][j].b = bwImg.map[i][j].i;
        }
    }
   return bwImg;
}

//----------------------------------------------convolution filter------------------------------------------------------

// rotate matrix helper function for convolution filter
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

//------------------------------------------------sobel filter-----------------------------------------------------

Image sobel(Image img) {
    double iSobelArr[3][3] = {{1, 2, 1},
                                {0, 0, 0},
                                {-1, -2, -1}};
    double jSobelArr[3][3] = {{1, 0, -1},
                                {2, 0, -2},
                                {1, 0, -1}};
    Matrix iSobelFilter = createMatrixFromArray(&iSobelArr[0][0], 3, 3);
    Matrix jSobelFilter = createMatrixFromArray(&jSobelArr[0][0], 3, 3);
    Matrix inputImgMatrix = image2Matrix(img);

    Matrix sobelFilteredMatrix = convolve(convolve(inputImgMatrix, iSobelFilter), jSobelFilter);
    Image sobelFilteredImg = matrix2Image(sobelFilteredMatrix, 1, 1);

    deleteMatrix(iSobelFilter);
    deleteMatrix(jSobelFilter);
    deleteMatrix(inputImgMatrix);
    deleteMatrix(sobelFilteredMatrix);

    return sobelFilteredImg;
}