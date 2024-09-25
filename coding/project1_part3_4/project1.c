// 
// project1.c
// CS136
// 
// project1_part3_4
// 
// created by Michael Huh 9/23/24
// last modified 9/23/24
// 
// referenced https://www.geeksforgeeks.org/generating-random-number-range-c/
// for generating random numbers in c
// 
// compile:
// gcc -o main project1.c netpbm.c
// 
// run:
// ./main
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"
#include <time.h>

#define MAX_EQUIVALENCES 1000000

// function prototypes
Image function_imageBlackWhite(Image img, int intensityThreshold);
Image expand(Image img);
Image shrink(Image img);
Image function_noiseImage(Image img, float p);
Matrix labelComponents(Image img);
Image colorAndCountComponents(Image img, Matrix labelMatrix, int *letterCount, int sizeThreshold);

int main(int argc, const char * argv[]) {
    // inputImage (read image file)
    Image inputImage = readImage("text_image.ppm");

    // to blackWhiteImage; intensity threshold = 128
    Image blackWhiteImage = function_imageBlackWhite(inputImage, 128);

    // denoise; shrink-expand-expand-shrink
    Image cleanedImage = shrink(expand(expand(shrink(blackWhiteImage))));

    // noise; probability = 5%
    Image noisedImage = function_noiseImage(cleanedImage, 5.0f);

    // label image components
    Matrix labeledImageMatrix = labelComponents(cleanedImage);

    Image matrixToImage = matrix2Image(labeledImageMatrix, 0, 1);
    writeImage(matrixToImage, "help.pgm");
    deleteImage(matrixToImage);

    // color and count components
    int count = 0;
    Image coloredImage = colorAndCountComponents(cleanedImage, labeledImageMatrix, &count, 100);
    printf("Number of Letters: %d\n", count);

    // write imgs to files
    writeImage(blackWhiteImage, "black-white.pbm");
    writeImage(cleanedImage, "image_cleaned.pbm");
    writeImage(noisedImage, "noise.pbm");
    writeImage(coloredImage, "text_colored.ppm");

    // free memory; delete imgs
    deleteImage(inputImage);
    deleteImage(blackWhiteImage);
    deleteImage(cleanedImage);
    deleteImage(noisedImage);
    deleteImage(coloredImage);

    printf("Program ends ...");
    return 0;
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

//--------------------------------Expand function-----------------------------------------------------------
/* Expand operation */

// 4-connectedness expand
Image expand(Image img) {
    // create new img
    Image expImg = createImage(img.height, img.width);

    // copy original img to new img
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            expImg.map[i][j] = img.map[i][j];
        }
    }

    // define neighbor offsets for 4-connectedness
    int dy[] = {0, 1, 0, -1};
    int dx[] = {-1, 0, 1, 0};

    // iterate over original img
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            // check if current pixel is black (obj)
            if (img.map[i][j].i == 0) {
                // iterate through 4-connected neighbors
                for (int k = 0; k < 4; k++) {
                    int s = i + dy[k];
                    int t = j + dx[k];

                    // check if within bounds
                    if (s >= 0 && s < img.height && t >= 0 && t < img.width) {
                        // set neighbor to black (obj)
                        expImg.map[s][t].i = 0;

                        // set r, g, b to match i
                        expImg.map[s][t].r = expImg.map[s][t].g = expImg.map[s][t].b = 0;
                    }
                }
            }
        }
    }
    return expImg;
}

//--------------------------------Shrink function-----------------------------------------------------------
/* Shrink operation */

// 4-connectedness shrink
Image shrink(Image img) {
    // create new img
    Image shrinkImg = createImage(img.height, img.width);

    // copy original img to new img
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            shrinkImg.map[i][j] = img.map[i][j];
        }
    }

    // define neighbor offsets for 4-connectedness
    int dy[] = {0, 1, 0, -1};
    int dx[] = {-1, 0, 1, 0};

    // iterate over original img
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            // check if current pixel is white (background)
            if (img.map[i][j].i == 255) {
                // iterate through 4-connected neighbors
                for (int k = 0; k < 4; k++) {
                    int s = i + dy[k];
                    int t = j + dx[k];

                    // check if within bounds
                    if (s >= 0 && s < img.height && t >= 0 && t < img.width) {
                        // set neighbor to white (background)
                        shrinkImg.map[s][t].i = 255;

                        // set r, g, b to match i
                        shrinkImg.map[s][t].r = shrinkImg.map[s][t].g = shrinkImg.map[s][t].b = 255;
                    }
                }
            }
        }
    }
    return shrinkImg;
}

//--------------------------------noise function-----------------------------------------------------------
/* function that adds binary noise to an image. This function receives an
 image and a floating point number p that indicates the probability
 (in percent) that each pixel in the image will be flipped, i.e.,
 turned from black to white or vice versa.
 */

Image function_noiseImage(Image img, float p) {
    //create new img
    Image noiseImg = createImage(img.height, img.width);
    
    // set seed for rand() function
    srand(time(0));
    
    // iterate over original img
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            // get pixel intensity
            int intensity = img.map[i][j].i;
            
            // generate random float between 0 and 1
            float random = (float)rand() / RAND_MAX;
            // if rand <= given prob, flip pixel
            if (random <= p / 100.0f) {  // converted p to float between 0 and 1
                noiseImg.map[i][j].i = (intensity == 0) ? 255 : 0;
            } else { // otherwise, pixel remains unchanges
                noiseImg.map[i][j].i = intensity;
            }
            
            // set r, g, b to match i
            noiseImg.map[i][j].r = noiseImg.map[i][j].g = noiseImg.map[i][j].b = noiseImg.map[i][j].i;
        }
    }
    return noiseImg;
}

//---------------------------4-neighbor region/component labeling-------------------------------------------

// stuct to hold equivalence pairs
typedef struct {
    int label1;
    int label2;
} EquivalencePair;

// struct to hold equivalence table
typedef struct {
    EquivalencePair *pairs;
    int count;
    int capacity;
} EquivalenceTable;

// create and initialize equivalence table
EquivalenceTable createEquivalenceTable() {
    EquivalenceTable et;
    et.pairs = (EquivalencePair *)malloc(MAX_EQUIVALENCES * sizeof(EquivalencePair));
    et.count = 0;
    et.capacity = MAX_EQUIVALENCES;
    return et;
}

// add equivalence pair to table
void addEquivalence(EquivalenceTable *et, int label1, int label2) {
    if (et->count < et->capacity) {
        et->pairs[et->count].label1 = label1;
        et->pairs[et->count].label2 = label2;
        et->count++;
    }
}

// 4-connectedness component labeling
Matrix labelComponents(Image img) {
    Matrix labelMatrix = createMatrix(img.height, img.width);   // initialize label matrix
    EquivalenceTable et = createEquivalenceTable();             // initialize equivalence table
    int currentLabel = 1;                                       // initialize lable counter

    // first pass 
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            // if is obj
            if (img.map[i][j].i == 0) {
                int up = (i > 0) ? (int)labelMatrix.map[i-1][j] : 0;        // get label of above pixel
                int left = (j > 0) ? (int)labelMatrix.map[i][j-1] : 0;      // get label of left pixel
                
                if (up == 0 && left == 0) {                     // if above and left both background
                    labelMatrix.map[i][j] = currentLabel++;     // add new label and increment
                }
                else if (up != 0 && left == 0) {                // if above labeled; left background
                    labelMatrix.map[i][j] = up;                 // set curr pixel label to same as above pixel
                }
                else if (left != 0 && up == 0) {                // if left labeled; above background
                    labelMatrix.map[i][j] = left;               // set curr pixel label to same as left pixel
                }
                else{                                           // if above and left both labeled
                    labelMatrix.map[i][j] = up;                 // set curr pixel label to same as above pixel ()
                    if (up != left) {                               // if above and left labels different
                        addEquivalence(&et, up, left);              // add to equivalence table
                    }
                }
            }
        }
    }

    // process equivalences; find root labels
        // create label map and load current labels
        int *labelMap = (int *)malloc(currentLabel * sizeof(int));
        for (int i = 0; i < currentLabel; i++) {
            labelMap[i] = i;
        }

        // path compression
        for (int i = 0; i < et.count; i++) {
            int label1 = et.pairs[i].label1;
            int label2 = et.pairs[i].label2;
            int root1 = label1;
            int root2 = label2;

            while (labelMap[root1] != root1) root1 = labelMap[root1];
            while (labelMap[root2] != root2) root2 = labelMap[root2];

            if (root1 < root2) {
                labelMap[root2] = root1;
            } else if (root2 < root1) {
                labelMap[root1] = root2;
            }
        }

    // second pass
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            if (labelMatrix.map[i][j] != 0) {
                int label = (int)labelMatrix.map[i][j];
                labelMatrix.map[i][j] = labelMap[label];    // relabel collisions
            }
        }
    }

    // deallocated memory
    free(et.pairs);
    free(labelMap);

    return labelMatrix;
}

//--------------------------------------------color and count components----------------------------------

// Function to generate a random color
Pixel generateRandomColor() {
    Pixel color;
    do {
        srand(time(0));
        color.r = rand() % 256;
        color.g = rand() % 256;
        color.b = rand() % 256;
    } while (color.r + color.g + color.b > 700 || color.r + color.g + color.b < 100);
    return color;
}

// Function to color components and count letters
Image colorAndCountComponents(Image img, Matrix labelMatrix, int *letterCount, int sizeThreshold) {
    int maxLabel = 0;
    for (int i = 0; i < labelMatrix.height; i++) {
        for (int j = 0; j < labelMatrix.width; j++) {
            if (labelMatrix.map[i][j] > maxLabel) {
                maxLabel = labelMatrix.map[i][j];
            }
        }
    }

    // Count the size of each component
    int *componentSizes = (int *)calloc(maxLabel + 1, sizeof(int));
    for (int i = 0; i < labelMatrix.height; i++) {
        for (int j = 0; j < labelMatrix.width; j++) {
            if (labelMatrix.map[i][j] != 0) {
                componentSizes[(int)labelMatrix.map[i][j]]++;
            }
        }
    }

    // Create a color map for components
    Pixel *colorMap = (Pixel *)malloc((maxLabel + 1) * sizeof(Pixel));
    for (int i = 1; i <= maxLabel; i++) {
        colorMap[i] = generateRandomColor();
    }

    // Create the output image
    Image coloredImage = createImage(img.height, img.width);
    int *countedLabels = (int *)calloc(maxLabel + 1, sizeof(int));

    // Color the components
    for (int i = 0; i < labelMatrix.height; i++) {
        for (int j = 0; j < labelMatrix.width; j++) {
            int label = (int)labelMatrix.map[i][j];
            if (label != 0 && componentSizes[label] >= sizeThreshold) {
                coloredImage.map[i][j] = colorMap[label];
                // Count each label only once
                if (countedLabels[label] == 0) {
                    countedLabels[label] = 1;
                    (*letterCount)++;
                }
            } else {
                coloredImage.map[i][j] = img.map[i][j];
            }
        }
    }

    free(componentSizes);
    free(colorMap);
    free(countedLabels);

    return coloredImage;
}