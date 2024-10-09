// 
// project3.c
// CS136
// 
// project3
// 
// created by Michael Huh 9/30/24
// last modified 10/9/24
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
Image canny(Image);

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

    Matrix gaussianFilter = createMatrixFromArray(&gaussianFilterArray[0][0], CONVOLUTION_FILTER_HEIGHT, CONVOLUTION_FILTER_WIDTH);
    printf("\nGaussian Filter: %d x %d\n", CONVOLUTION_FILTER_HEIGHT, CONVOLUTION_FILTER_WIDTH);
    printMatrix(&gaussianFilter);

    //-----------------------------------------------------------------------------------------------------------------------------------------------------------------

    // gaussian smooth input matrix
    Matrix smoothedMatrix = convolve(inputMatrix, gaussianFilter);
    // convert smoothed matrix to img
    Image smoothedImage = matrix2Image(smoothedMatrix, 1, 1);

    // write smoothed img to files
    writeImage(smoothedImage, "smoothed_image.pgm");

    Image sobelFilteredImage = sobel(smoothedImage);
    writeImage(sobelFilteredImage, "sobeltest.pgm");

    Image cannyFilteredImage = canny(smoothedImage);
    writeImage(cannyFilteredImage, "cannytest.pgm");

    // free memory; delete imgs and matricies
    deleteImage(inputImage);
    deleteMatrix(inputMatrix);
    deleteMatrix(gaussianFilter);
    deleteMatrix(smoothedMatrix);
    deleteImage(smoothedImage);
    deleteImage(sobelFilteredImage);
    deleteImage(cannyFilteredImage);

    printf("Program ends ...\n");
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

Image function_imageBlackWhite(Image img, double intensityThreshold) {
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

//----------------------------------------gradient mag and orientation--------------------------------------------

typedef struct {
    Matrix gradientMagnitudeMatrix;
    Matrix gradientOreintationMatrix;
} Gradient;

Gradient gradientMagnitudeAndOrientation(Matrix *vertical, Matrix *horizontal) {
    int height = vertical->height;
    int width = vertical->width;
    Matrix gradientMagnitudeMatrix = createMatrix(height, width);
    Matrix gradientOrientationMatrix = createMatrix(height, width);

    // Compute gradient magnitude and orientation
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            double gradientMagnitude = sqrt(pow(vertical->map[i][j], 2) + pow(horizontal->map[i][j], 2));
            double gradientAngle = atan2(horizontal->map[i][j], vertical->map[i][j]) * (180.0 / PI);
            if (gradientAngle < 0) {
                gradientAngle += 360;
            }
            gradientAngle = fmod(gradientAngle + 90, 360);
            gradientMagnitudeMatrix.map[i][j] = gradientMagnitude;
            gradientOrientationMatrix.map[i][j] = gradientAngle;
        }
    }
    Gradient result = {gradientMagnitudeMatrix, gradientOrientationMatrix};

    return result;
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

    Matrix iSobelFilteredMatrix = convolve(inputImgMatrix, iSobelFilter);
    Matrix jSobelFilteredMatrix = convolve(inputImgMatrix, jSobelFilter);

    Gradient gradient = gradientMagnitudeAndOrientation(&iSobelFilteredMatrix, &jSobelFilteredMatrix);
    Matrix gradientMagnitude = gradient.gradientMagnitudeMatrix;

    Image sobelFilteredImg = matrix2Image(gradientMagnitude, 1, 1);

    Image thresholdedSobelFilteredImg = function_imageBlackWhite(sobelFilteredImg, 60);

    deleteMatrix(iSobelFilter);
    deleteMatrix(jSobelFilter);
    deleteMatrix(iSobelFilteredMatrix);
    deleteMatrix(jSobelFilteredMatrix);
    deleteMatrix(inputImgMatrix);
    deleteMatrix(gradientMagnitude);
    deleteImage(sobelFilteredImg);

    return thresholdedSobelFilteredImg;
}

//--------------------------------------------------canny filter-------------------------------------------------------------

Matrix assignSector(const Matrix *gradientMagnitude, const Matrix *gradientOrientation) {
    int height = gradientMagnitude->height;
    int width = gradientMagnitude->width;
    Matrix sectorMatrix = createMatrix(height, width);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            double angle = gradientOrientation->map[i][j];
            int sector;
            if ((angle >= 337.5 && angle < 22.5) || (angle >= 157.5 && angle < 202.5)) {
                sector = 0;
            } else if ((angle >= 22.5 && angle < 67.5) || (angle >= 202.5 && angle < 247.5)) {
                sector = 1;
            } else if ((angle >= 67.5 && angle < 112.5) || (angle >= 247.5 && angle < 292.5)) {
                sector = 2;
            } else {    // ((angle >= 112.5 && angle < 157.5 || (angle >= 292.5 && angle < 337.5))
                sector = 3;
            }
            sectorMatrix.map[i][j] = sector;
        }
    }
    
    return sectorMatrix;
}

Matrix nonmaximaSuppression(Matrix *gradientMagnitude, Matrix *sector) {
    
}

Image canny(Image img) {
    double pCannyArr[2][2] = {{0.5, 0.5},
                                {-0.5, -0.5}};
    double qCannyArr[2][2] = {{0.5, -0.5},
                                {0.5, -0.5}};
    Matrix pCannyFilter = createMatrixFromArray(&pCannyArr[0][0], 2, 2);
    Matrix qCannyFilter = createMatrixFromArray(&qCannyArr[0][0], 2, 2);
    Matrix inputImgMatrix = image2Matrix(img);

    Matrix pCannyFilteredMatrix = convolve(inputImgMatrix, pCannyFilter);
    Matrix qCannyFitleredMatrix = convolve(inputImgMatrix, qCannyFilter);

    Gradient gradient = gradientMagnitudeAndOrientation(&pCannyFilteredMatrix, &qCannyFitleredMatrix);
    Matrix gradientMagnitude = gradient.gradientMagnitudeMatrix;
    Matrix gradientOrientation = gradient.gradientOreintationMatrix;

    Matrix sectorMatrix = assignSector(&gradientMagnitude, &gradientOrientation);

    Image cannyFilteredImg = matrix2Image(gradientMagnitude, 1, 1);

    deleteMatrix(pCannyFilter);
    deleteMatrix(qCannyFilter);
    deleteMatrix(inputImgMatrix);
    deleteMatrix(pCannyFilteredMatrix);
    deleteMatrix(qCannyFitleredMatrix);
    deleteMatrix(gradientMagnitude);
    deleteMatrix(gradientOrientation);

    return cannyFilteredImg;
}