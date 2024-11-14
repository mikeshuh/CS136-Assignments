// 
// project5.c
// CS136
// 
// project5
// 
// created by Michael Huh 10/30/24
// last modified 11/13/24
// 
// compile:
// gcc -o main project5.c netpbm.c
// 
// run:
// ./main
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"

int main(int argc, const char *argv[]) {
    printf("\nProgram ends...\n\n");
    return 0;
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

/*
first, derive local feature vectors
that describe the texture at given coordinates (x, y). These features could be obtained
through Law’s filters, co-occurrence matrices, any other techniques that you know or
develop, or combinations of them. The resulting data could be stored in an array of tables, or you could
introduce a new data structure to hold the data.

Law's filters:
    1. Filter the input image using texture filters.
    2. Compute texture energy by summing the
    absolute value of filtering results in local
    neighborhoods around each pixel.
    3. Combine features to achieve rotational
    invariance.

five filters:
• L5 = (1, 4, 6, 4, 1)
Level (Gaussian): gives a center-weighted local average
• E5 = (-1, -2, 0, 2, 1)
Edge (Gradient): responds to row or col step edges
• S5 = (-1, 0, 2, 0, -1)
Spot: detects spots
• R5 = (1, -4, 6, -4, 1)
Ripple: detects ripple
• W5 = (-1, 2, 0, -2, 1)
Wave: respond to an input pattern

Multiply any two vectors to get 25 convolution filters:
L5L5 E5L5 S5L5 W5L5 R5L5
L5E5 E5E5 S5E5 W5E5 R5E5
L5S5 E5S5 S5S5 W5S5 R5S5
L5W5 E5W5 S5W5 W5W5 R5W5
L5R5 E5R5 S5R5 W5R5 R5R5
*/

void lawsFiltering(char *inputFilename) {
    Image inputImg = readImage(inputFilename);
}

/*
your next step is to apply some form of cluster
analysis on these vectors in order to determine groups of similar vectors. This may work
better if you compute average vectors for small neighborhoods (maybe 8x8 or 16x16
pixels) instead of individual pixel locations. 
*/

/*
Finally, in the input image, mark the areas belonging to different groups in different colors,
for example, by setting either the R, G, or B values (or combinations of them) of all relevant
pixels to 0 or 255. Alternatively, you could simply overwrite the original gray values with
solid color.
*/

Image segmentTexture(Image inputImg, int segments) {
    
}