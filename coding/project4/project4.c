// 
// project4.c
// CS136
// 
// project3
// 
// created by Michael Huh 10/15/24
// last modified 10/15/24
// 
// compile:
// gcc -o main project4.c netpbm.c
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

// define accumulator
    #define MAX_Y 768
    #define MAX_X 1024
    #define MAX_R 150

    typedef struct
    {
        int y, x, r;
        int ***voteMap;
    } Accumulator;

// function prototypes
void edgeDetection(char *, char *, char *);
Accumulator initAccumulator(int, int, int);
void freeAccumulator(Accumulator *);
void houghTransformedLines(const Image *, Accumulator *);
Image visualizeHoughMaxima(const Accumulator *);

int main(int argc, const char *argv[]) {
    edgeDetection("circle.ppm", "circle_sobel.pbm", "circle_canny.pbm");

    Image inputImg = readImage("circle_sobel.pbm");

    Accumulator accum = initAccumulator(MAX_Y, MAX_X, MAX_R);

    houghTransformedLines(&inputImg, &accum);

    //Image houghMaximaImg = visualizeHoughMaxima(&accum);

    //writeImage(houghMaximaImg, "cirle_maxima.pgm");

    deleteImage(inputImg);
    freeAccumulator(&accum);
    //deleteImage(houghMaximaImg);

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

// struct for gradient magnitude and orientation
typedef struct {
    Matrix gradientMagnitudeMatrix;
    Matrix gradientOrientationMatrix;
} Gradient;

// calculate gradient magnitude and orientation; args: vertical and horizontal sobel filtered matrices
Gradient gradientMagnitudeAndOrientation(const Matrix *vertical, const Matrix *horizontal) {
    int height = vertical->height;
    int width = vertical->width;
    Matrix gradientMagnitudeMatrix = createMatrix(height, width);
    Matrix gradientOrientationMatrix = createMatrix(height, width);

    // compute gradient magnitude and orientation
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // gradient magnitude
            double gradientMagnitude = sqrt(pow(vertical->map[i][j], 2) + pow(horizontal->map[i][j], 2));

            // gradient angle
            double gradientAngle = atan2(horizontal->map[i][j], vertical->map[i][j]) * (180.0 / PI);
            if (gradientAngle < 0) {        // ensure angles from 0 to 360 deg                                              //     180
                gradientAngle += 360;                                                                                       //  270   90
            }                                                                                                               //      0
            gradientAngle = fmod(gradientAngle + 90, 360);      // adjust angles so 0 deg is oriented south like so ----------------^         

            // set gradient magnitude and orientation
            gradientMagnitudeMatrix.map[i][j] = gradientMagnitude;
            gradientOrientationMatrix.map[i][j] = gradientAngle;
        }
    }
    Gradient result = {gradientMagnitudeMatrix, gradientOrientationMatrix};

    return result;
}

//------------------------------------------------sobel filter-----------------------------------------------------

Image sobel(Image img) {
    // horizontal contour mask arr
    double iSobelArr[3][3] = {{1, 2, 1},
                                {0, 0, 0},
                                {-1, -2, -1}};
    // vertical contour mask arr
    double jSobelArr[3][3] = {{1, 0, -1},
                                {2, 0, -2},
                                {1, 0, -1}};
    // arr to matrix
    Matrix iSobelFilter = createMatrixFromArray(&iSobelArr[0][0], 3, 3);
    Matrix jSobelFilter = createMatrixFromArray(&jSobelArr[0][0], 3, 3);
    // input img to matrix
    Matrix inputImgMatrix = image2Matrix(img);

    // apply masks to input img
    Matrix iSobelFilteredMatrix = convolve(inputImgMatrix, iSobelFilter);
    Matrix jSobelFilteredMatrix = convolve(inputImgMatrix, jSobelFilter);

    // get gradient magnitude and orientation
    Gradient gradient = gradientMagnitudeAndOrientation(&jSobelFilteredMatrix, &iSobelFilteredMatrix);

    // gradient matrix to img
    Image sobelFilteredImg = matrix2Image(gradient.gradientMagnitudeMatrix, 1, 1);

    // threshold img
    Image thresholdedSobelFilteredImg = function_imageBlackWhite(sobelFilteredImg, 45);

    // free memory
    deleteMatrix(iSobelFilter);
    deleteMatrix(jSobelFilter);
    deleteMatrix(inputImgMatrix);
    deleteMatrix(iSobelFilteredMatrix);
    deleteMatrix(jSobelFilteredMatrix);
    deleteMatrix(gradient.gradientMagnitudeMatrix);
    deleteMatrix(gradient.gradientOrientationMatrix);
    deleteImage(sobelFilteredImg);

    return thresholdedSobelFilteredImg;
}

//--------------------------------------------------canny filter-------------------------------------------------------------

// assign sector helper function for nonmaxima suppression
Matrix assignSector(const Matrix *gradientMagnitude, const Matrix *gradientOrientation) {
    int height = gradientMagnitude->height;
    int width = gradientMagnitude->width;
    Matrix sectorMatrix = createMatrix(height, width);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            double angle = gradientOrientation->map[i][j];
            int sector;
            // sectors
                //  1   0   3
                //  2 [i,j] 2
                //  3   0   1
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

// nonmaxima suppression
Matrix nonmaximaSuppression(const Matrix *gradientMagnitudeMatrix, const Matrix *sectorMatrix) {
    int height = gradientMagnitudeMatrix->height;
    int width = gradientMagnitudeMatrix->width;
    Matrix suppressedMatrix = createMatrix(height, width);

    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int sector = sectorMatrix->map[i][j];
            int x1, x2, y1, y2;
            if (sector == 0) {
                x1 = x2 = j;            //    x
                y1 = i - 1;             //    x
                y2 = i + 1;             //    x
            } else if (sector == 1) {
                x1 = j - 1;             //  x
                x2 = j + 1;             //    x
                y1 = i - 1;             //      x
                y2 = i + 1;
            } else if (sector == 2) {
                x1 = j - 1;             //
                x2 = j + 1;             //  x x x
                y1 = y2 = i;            //
            } else {    // sector == 3
                x1 = j - 1;             //      x
                x2 = j + 1;             //    x
                y1 = i + 1;             //  x
                y2 = i - 1;
            }

            // compare gradient mags
            double gM0 = gradientMagnitudeMatrix->map[i][j];
            double gM1 = gradientMagnitudeMatrix->map[y1][x1];
            double gM2 = gradientMagnitudeMatrix->map[y2][x2];
            if (gM0 < gM1 || gM0 < gM2) {
                suppressedMatrix.map[i][j] = 0;
            } else {
                suppressedMatrix.map[i][j] = gM0;
            }
        }
    }

    return suppressedMatrix;
}

// follow candidate edge path and relabel if connected to edge; helper function for hysteresis thresholding
// modifies label matrix
void followPathAndRelabel(Matrix *labelMatrix, int currY, int currX) {
    int dY[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dX[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

    for (int i = 0; i < 8; i ++) {
        int resY = currY + dY[i];
        int resX = currX + dX[i];

        if (resY >= 0 && resY < labelMatrix->height && resX >= 0 && resX < labelMatrix->width) {
            if (labelMatrix->map[resY][resX] == 2) {
                labelMatrix->map[resY][resX] = 255;
                followPathAndRelabel(labelMatrix, resY, resX);
            }
        }
    }
}

// hysteresis thresholding; args: nonmaxima suppression matrix, lowthreshold, highthreshold
Matrix hysteresisThreshold(const Matrix *suppressedMatrix, double lowThreshold, double highThreshold) {
    int height = suppressedMatrix->height;
    int width = suppressedMatrix->width;
    Matrix labelMatrix = createMatrix(height, width);   // 0 = no edge; 255 = edge; 2 = candidate

    // first pass
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            double magnitude = suppressedMatrix->map[i][j];
            int label;

            // apply thresholds
            if (magnitude < lowThreshold) {
                label = 0;                              // no edge
            } else if (magnitude > highThreshold) {
                label = 255;                            // edge
            } else {
                label = 2;                              // candidate edge
            }
            labelMatrix.map[i][j] = label;
        }
    }

    // second pass
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (labelMatrix.map[i][j] == 255) {     // if edge
                followPathAndRelabel(&labelMatrix, i, j);   // check surrounding pixel for candidate, follow path, and relabel
            }
        }
    }

    return labelMatrix;
}

Image canny(Image img) {
    // horizontal contour mask arr
    double iSobelArr[3][3] = {{1, 2, 1},
                                {0, 0, 0},
                                {-1, -2, -1}};
    // vertical contour mask arr
    double jSobelArr[3][3] = {{1, 0, -1},
                                {2, 0, -2},
                                {1, 0, -1}};
    // arr to matrix
    Matrix iSobelFilter = createMatrixFromArray(&iSobelArr[0][0], 3, 3);
    Matrix jSobelFilter = createMatrixFromArray(&jSobelArr[0][0], 3, 3);
    // input img to matrix
    Matrix inputImgMatrix = image2Matrix(img);

    // apply masks to input img
    Matrix iSobelFilteredMatrix = convolve(inputImgMatrix, iSobelFilter);
    Matrix jSobelFilteredMatrix = convolve(inputImgMatrix, jSobelFilter);

    // get gradient magnitude and orientation
    Gradient gradient = gradientMagnitudeAndOrientation(&jSobelFilteredMatrix, &iSobelFilteredMatrix);

    // assign sectors for nonmaxima suppression
    Matrix sectorMatrix = assignSector(&gradient.gradientMagnitudeMatrix, &gradient.gradientOrientationMatrix);

    // perform nonmaxima suppression
    Matrix suppressedMatrix = nonmaximaSuppression(&gradient.gradientMagnitudeMatrix, &sectorMatrix);

    // adjust suppressed matrix to values 0 - 255
    Image tempImg = matrix2Image(suppressedMatrix, 1, 1);
    Matrix adjustedSuppressedMatrix = image2Matrix(tempImg);

    // perform hysteresis thresholding
    Matrix hysteresisThresholdedMatrix = hysteresisThreshold(&adjustedSuppressedMatrix, 40, 50);

    // matrix to img
    Image cannyFilteredImg = matrix2Image(hysteresisThresholdedMatrix, 1, 1);

    // free memory
    deleteMatrix(iSobelFilter);
    deleteMatrix(jSobelFilter);
    deleteMatrix(inputImgMatrix);
    deleteMatrix(iSobelFilteredMatrix);
    deleteMatrix(jSobelFilteredMatrix);
    deleteMatrix(gradient.gradientMagnitudeMatrix);
    deleteMatrix(gradient.gradientOrientationMatrix);
    deleteMatrix(sectorMatrix);
    deleteMatrix(suppressedMatrix);
    deleteImage(tempImg);
    deleteMatrix(adjustedSuppressedMatrix);
    deleteMatrix(hysteresisThresholdedMatrix);

    return cannyFilteredImg;
}

//-------------------------------------------------edge detection--------------------------------------

void edgeDetection(char *inputFilename, char *sobelFilename, char *cannyFilename) {
    // input img
    Image inputImage = readImage(inputFilename);
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

    // create buffer for smoothed image filename
    int len = strlen(inputFilename) + strlen("_smoothed_image.pgm") + 1;    // allocate memory for concatenated string
    char *smoothedFilename = (char *)malloc(len);
    
    // copy input filename and concatenate "_smoothed_image.pgm"
    strcpy(smoothedFilename, inputFilename);
    strcat(smoothedFilename, "_smoothed_image.pgm");

    // write smoothed image to file
    writeImage(smoothedImage, smoothedFilename);

    // sobel edge detection
    Image sobelFilteredImage = sobel(smoothedImage);
    writeImage(sobelFilteredImage, sobelFilename);

    // canny edge detection
    Image cannyFilteredImage = canny(smoothedImage);
    writeImage(cannyFilteredImage, cannyFilename);

    // free memory; delete imgs and matricies
    deleteImage(inputImage);
    deleteMatrix(inputMatrix);
    deleteMatrix(gaussianFilter);
    deleteMatrix(smoothedMatrix);
    deleteImage(smoothedImage);
    deleteImage(sobelFilteredImage);
    deleteImage(cannyFilteredImage);
}

//---------------------------------------houghTransformLines----------------------------------------------

Accumulator initAccumulator(int y, int x, int r) {
    Accumulator accum;
    accum.y = y;
    accum.x = x;
    accum.r = r;

    accum.voteMap = (int ***)calloc(y, sizeof(int **));
    for (int i = 0; i < y; i++) {
        accum.voteMap[i] = (int **)calloc(x, sizeof(int *));
        for (int j = 0; j < x; j++) {
            accum.voteMap[i][j] = (int *)calloc(r, sizeof(int));
        }
    }

    return accum;
}

void freeAccumulator(Accumulator *accum) {
    for (int i = 0; i < accum->y; i++) {
        for (int j = 0; j < accum->x; j++) {
            free(accum->voteMap[i][j]);
        }
        free(accum->voteMap[i]);
    }
    free(accum->voteMap);
}

void houghTransformedLines(const Image *edgeImg, Accumulator *accum) {
    for (int y = 0; y < edgeImg->height; y++) {
        for (int x = 0; x < edgeImg->width; x++) {
            if (edgeImg->map[y][x].i > 0) {
                for (int r = 0; r < accum->r; r++) {
                    for (int theta = -180; theta < 180; theta++) {
                        double radian = theta * PI / 180;
                        int y_center = (int)(y - r * sin(radian));
                        int x_center = (int)(x - r * cos(radian));
                        if (y_center >= 0 && y_center < accum->y && x_center >= 0 && x_center < accum->x) {
                            accum->voteMap[y_center][x_center][r]++;
                        }
                    }
                }
            }
        }
    }
}

Image visualizeHoughMaxima(const Accumulator *accum) {
    int height = accum->y;
    int width = accum->x;
    Matrix houghMaximaMatrix = createMatrix(height, width);

    for (int dY = 0; dY < height; dY++) {
        for (int dX = 0; dX < width; dX++) {
            for (int r = 0; r < accum->r; r++) {
                if (accum->voteMap[dY][dX][r] > 0){
                    for (int theta = -180; theta < 180; theta++) {
                        double radian = theta * PI / 180;
                        int y = (int)(dY - r * sin(radian));
                        int x = (int)(dX - r * cos(radian));
                        if (y >= 0 && y < accum->y && x >= 0 && x < accum->x) {
                            houghMaximaMatrix.map[y][x]++;
                        }
                    }
                }
            }
        }
    }

    Image houghMaximaImg = matrix2Image(houghMaximaMatrix, 1, 1);
    deleteMatrix(houghMaximaMatrix);

    return houghMaximaImg;
}

//---------------------------------------findHoughMaxima--------------------------------------------------