// 
// project4.c
// CS136
// 
// project4
// 
// created by Michael Huh 10/15/24
// last modified 10/20/24
// 
// compile:
// gcc -o main project4.c netpbm.c
// 
// run:
// ./main
// 
// -----------NOTE-----------
//  ~15 second execution time on my machine
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"

// convolution filter
// #define CONVOLUTION_FILTER_HEIGHT 5
// #define CONVOLUTION_FILTER_WIDTH 5

// define accumulator; estimate radii range
    // soda
    #define MAX_R 75
    #define MIN_R 65

// define a maximum number of circles
#define MAX_CIRCLES 100
    
// accumulator struct
typedef struct
{
    int max_y, max_x, max_r, min_r;
    int ***voteMap;
} Accumulator;

// circle struct
typedef struct {
    int y, x, r;  // Coordinates of center and radius
} Circle;

// function prototypes
void edgeDetection(char *, char *, char *);
Accumulator initAccumulator(int, int, int, int);
void freeAccumulator(Accumulator *);
void houghTransformLines(const Image *, Accumulator *);
Image visualizeHoughMaxima(const Accumulator *);
Circle *findHoughMaxima(Accumulator *, int threshold, int *);
void drawCircle(Image *, int, int, int, int, int, int, int);

int main(int argc, const char *argv[]) {
    printf("\n");

    // original img
    Image originalImg = readImage("soda.ppm");

    // edge detection
    edgeDetection("soda.ppm", "soda_sobel.pbm", "soda_canny.pbm");
    
    // edge img
    Image inputImg = readImage("soda_sobel.pbm");

    // create accumulator
    Accumulator accum = initAccumulator(inputImg.height, inputImg.width, MAX_R, MIN_R);

    // hough transformation
    houghTransformLines(&inputImg, &accum);

    // create visualization of accumulator; create hough maxima image
    Image houghMaximaImg = visualizeHoughMaxima(&accum);
    writeImage(houghMaximaImg, "hough_maxima.pgm");     // write to files

    int numCircles;     // declare numCircles var
    // find maxima; store in Circle arr
    Circle *detectedCircles = findHoughMaxima(&accum, 170, &numCircles);    // adjust threshold value based on img experimentation

    printf("Num Circles/Soda Cans: %d\n", numCircles);    // print num circles detected
    // print circle location (x, y) and radius
    for (int i = 0; i < numCircles; i++) {
        printf("Circle/Can Detected: Center = (%d, %d), Radius = %d\n", detectedCircles[i].x, detectedCircles[i].y, detectedCircles[i].r);
    }

    // outline detected circles over original image in red
    int thickness = 2;      // adjust value to increase or decrease thickness of circle
    for (int i = 0; i < numCircles; i++) {
        drawCircle(&originalImg, detectedCircles[i].y, detectedCircles[i].x, detectedCircles[i].r, thickness, 255, 0, 0);   // draw red color circle
    }
    printf("\nDisplay Located Circles Complete...\n");

    // save modified image with circles drawn to files
    writeImage(originalImg, "hough_image.ppm");

    // free memory
    deleteImage(originalImg);
    deleteImage(inputImg);
    freeAccumulator(&accum);
    deleteImage(houghMaximaImg);
    free(detectedCircles);

    printf("\nProgram ends...\n\n");
    return 0;
}

//---------------------------------------------print matrix----------------------------------------------------------

// void printMatrix(const Matrix *m) {
//     for (int i = 0; i < m->height; i++) {
//         for (int j = 0; j < m->width; j++) {
//             printf("%f ", m->map[i][j]);
//         }
//         printf("\n");
//     }
//     printf("\n");
// }

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

    // remove smoothing
        // // input img -> matrix
        // Matrix inputMatrix = image2Matrix(inputImage);

        // //---------------------------------------------------------------------convolution filter-----------------------------------------------------------------------------

        // // 5x5 discrete gaussian filter
        // double gaussianFilterArray[CONVOLUTION_FILTER_HEIGHT][CONVOLUTION_FILTER_WIDTH] = {{(double)1/273, (double)4/273, (double)7/273, (double)4/273, (double)1/273},
        //                                                                                     {(double)4/273, (double)16/273, (double)26/273, (double)16/273, (double)4/273},
        //                                                                                     {(double)7/273, (double)26/273, (double)41/273, (double)26/273, (double)7/273},
        //                                                                                     {(double)4/273, (double)16/273, (double)26/273, (double)16/273, (double)4/273},
        //                                                                                     {(double)1/273, (double)4/273, (double)7/273, (double)4/273, (double)1/273}};

        // Matrix gaussianFilter = createMatrixFromArray(&gaussianFilterArray[0][0], CONVOLUTION_FILTER_HEIGHT, CONVOLUTION_FILTER_WIDTH);
        // printf("\nGaussian Filter: %d x %d\n", CONVOLUTION_FILTER_HEIGHT, CONVOLUTION_FILTER_WIDTH);
        // printMatrix(&gaussianFilter);

        // //-----------------------------------------------------------------------------------------------------------------------------------------------------------------

        // // gaussian smooth input matrix
        // Matrix smoothedMatrix = convolve(inputMatrix, gaussianFilter);
        // // convert smoothed matrix to img
        // Image smoothedImage = matrix2Image(smoothedMatrix, 1, 1);

        // // create buffer for smoothed image filename
        // int len = strlen(inputFilename) + strlen("_smoothed_image.pgm") + 1;    // allocate memory for concatenated string
        // char *smoothedFilename = (char *)malloc(len);
        
        // // copy input filename and concatenate "_smoothed_image.pgm"
        // strcpy(smoothedFilename, inputFilename);
        // strcat(smoothedFilename, "_smoothed_image.pgm");

        // // write smoothed image to file
        // writeImage(smoothedImage, smoothedFilename);

    // sobel edge detection
    Image sobelFilteredImage = sobel(inputImage);  // sobel(smoothedImage);
    writeImage(sobelFilteredImage, sobelFilename);

    // canny edge detection
    Image cannyFilteredImage = canny(inputImage);    //canny(smoothedImage);
    writeImage(cannyFilteredImage, cannyFilename);

    // free memory; delete imgs and matricies
    deleteImage(inputImage);
    // deleteMatrix(inputMatrix);
    // deleteMatrix(gaussianFilter);
    // deleteMatrix(smoothedMatrix);
    // deleteImage(smoothedImage);
    deleteImage(sobelFilteredImage);
    deleteImage(cannyFilteredImage);
}

//---------------------------------------houghTransformLines----------------------------------------------

// initialize accumulator
Accumulator initAccumulator(int y, int x, int max_r, int min_r) {
    Accumulator accum;
    accum.max_y = y;
    accum.max_x = x;
    accum.max_r = max_r;
    accum.min_r = min_r;

    // initialize voteMap; 3D array; all values 0
    accum.voteMap = (int ***)calloc(y, sizeof(int **));
    for (int i = 0; i < y; i++) {
        accum.voteMap[i] = (int **)calloc(x, sizeof(int *));
        for (int j = 0; j < x; j++) {
            accum.voteMap[i][j] = (int *)calloc(max_r, sizeof(int));
        }
    }

    return accum;
}

// free accumulator memory
void freeAccumulator(Accumulator *accum) {
    for (int i = 0; i < accum->max_y; i++) {
        for (int j = 0; j < accum->max_x; j++) {
            free(accum->voteMap[i][j]);
        }
        free(accum->voteMap[i]);
    }
    free(accum->voteMap);
}

// hough transform
void houghTransformLines(const Image *edgeImg, Accumulator *accum) {
    // iterate edge img
    for (int y = 0; y < edgeImg->height; y++) {
        for (int x = 0; x < edgeImg->width; x++) {
            // if pixel is edge
            if (edgeImg->map[y][x].i > 0) {
                // iterate from min radius to max radius
                for (int r = accum->min_r; r < accum->max_r; r++) {
                    // iterate from 0 to 360 degrees
                    for (int theta = 0; theta < 360; theta++) {
                        // convert to radian
                        double radian = theta * PI / 180;

                        // get y and x locations of potential center
                        int y_center = (int)(y - r * sin(radian));
                        int x_center = (int)(x - r * cos(radian));

                        // check bounds
                        if (y_center >= 0 && y_center < accum->max_y && x_center >= 0 && x_center < accum->max_x) {
                            // increment accumulator
                            accum->voteMap[y_center][x_center][r]++;
                        }
                    }
                }
            }
        }
    }
    printf("Hough Transformed Lines Complete...\n\n");
}

// create image of accumulator
Image visualizeHoughMaxima(const Accumulator *accum) {
    int height = accum->max_y;
    int width = accum->max_x;
    // create maxima matrix; 2D projection of the accumulator based on y and x; r is summed
    Matrix houghMaximaMatrix = createMatrix(height, width);

    // itereate accumulator y and x
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // sum r values
            int voteSum = 0;
            for (int r = accum->min_r; r < accum->max_r; r++) {
                voteSum += accum->voteMap[y][x][r];
            }

            // add to maxima matrix
            houghMaximaMatrix.map[y][x] = voteSum;
        }
    }

    // convert matrix to img with linear scaling, i.e., intensity vals 0-255
    Image houghMaximaImg = matrix2Image(houghMaximaMatrix, 1, 1);

    deleteMatrix(houghMaximaMatrix);    // free memory

    printf("Visualize Hough Maxima Complete...\n\n");

    return houghMaximaImg;
}

//---------------------------------------findHoughMaxima--------------------------------------------------

// define extended neighborhood; adjusted based on img
#define DY MIN_R
#define DX MIN_R
#define DR 7

// define duplicate threshold
#define DISTANCE_THRESHOLD 10
#define RADIUS_THRESHOLD 5

// find maxima; return array of found circles
Circle *findHoughMaxima(Accumulator *accum, int threshold, int *numCircles) {
    *numCircles = 0;    // circle count

    // allocate memory for circle arr
    Circle *circles = (Circle *)malloc(MAX_CIRCLES * sizeof(Circle));

    // traverse accumulator to find local maxima
    for (int y = 0; y < accum->max_y; y++) {
        for (int x = 0; x < accum->max_x; x++) {
            for (int r = accum->min_r; r < accum->max_r; r++) {
                int currentVotes = accum->voteMap[y][x][r];

                // apply threshold to avoid lower intensity maxima
                if (currentVotes < threshold) {
                    continue;
                }

                // check if currentVotes is a local maximum in an extended neighborhood
                int isLocalMaxima = 1;      // true
                for (int dy = -DY; dy <= DY; dy++) {
                    for (int dx = -DX; dx <= DX; dx++) {
                        for (int dr = -DR; dr <= DR; dr++) {
                            // skip current point itself
                            if (dy == 0 && dx == 0 && dr == 0) {
                                continue;
                            }

                            // calculate neighbor coordinates
                            int ny = y + dy;
                            int nx = x + dx;
                            int nr = r + dr;

                            // check bounds
                            if (ny >= 0 && ny < accum->max_y && nx >= 0 && nx < accum->max_x && nr >= accum->min_r && nr < accum->max_r) {
                                if (accum->voteMap[ny][nx][nr] > currentVotes) {
                                    isLocalMaxima = 0;      // false
                                    break;
                                }
                            }
                        }
                        if (!isLocalMaxima) {
                            break;
                        }
                    }
                    if (!isLocalMaxima) {
                        break;
                    }
                }

                // if local maxima, add to circles list after checking for duplicates
                if (isLocalMaxima && *numCircles < MAX_CIRCLES) {
                    // check for duplicates based on distance from already detected circles
                    int isDuplicate = 0;
                    for (int i = 0; i < *numCircles; i++) {
                        int dy = circles[i].y - y;
                        int dx = circles[i].x - x;
                        int dr = abs(circles[i].r - r);
                        double distance = sqrt(dx * dx + dy * dy);

                        // consider circles duplicate if centers are too close and radii are similar
                        if (distance < DISTANCE_THRESHOLD && dr < RADIUS_THRESHOLD) {
                            isDuplicate = 1;
                            break;
                        }
                    }

                    // add circle if not duplicate
                    if (!isDuplicate) {
                        circles[*numCircles].y = y;
                        circles[*numCircles].x = x;
                        circles[*numCircles].r = r;
                        (*numCircles)++;
                    }
                }
            }
        }
    }

    printf("Locate Hough Maxima Complete...\n\n");

    return circles;
}

// draw circles over img
void drawCircle(Image *img, int y, int x, int r, int thickness, int color_r, int color_g, int color_b) {
    for (int t = -thickness; t <= thickness; t++) {     // thickness
        int adjusted_radius = r + t;

        // iterate over circle using parametric equation of a circle
        for (int theta = 0; theta < 360; theta++) {
            double radian = theta * PI / 180.0;     // convert to radian
            // get y and x coords
            int y_circle = y + (int)(adjusted_radius * sin(radian));
            int x_circle = x + (int)(adjusted_radius * cos(radian));

            // ensure coords within bounds
            if (y_circle >= 0 && y_circle < img->height && x_circle >= 0 && x_circle < img->width) {
                // color img
                img->map[y_circle][x_circle].r = color_r;
                img->map[y_circle][x_circle].g = color_g;
                img->map[y_circle][x_circle].b = color_b;
            }
        }
    }
}
