// 
// project1.c
// CS136
// 
// project1_part3_4
// 
// created by Michael Huh 9/23/24
// last modified 9/25/24
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

// change based on image size
#define MAX_EQUIVALENCES 100000

// function prototypes
Image function_imageBlackWhite(Image img, int intensityThreshold);
Image expand(Image img);
Image shrink(Image img);
Image function_noiseImage(Image img, float p);
Matrix labelComponents(Image img);
Image colorAndCountComponents(Image img, Matrix labelMatrix, int sizeThreshold);

int main(int argc, const char * argv[]) {
    // seed rand function
    srand(time(0));

    // inputImage (read image file)
    Image inputImage = readImage("text_image.ppm");

    // to blackWhiteImage; intensity threshold = 128
    Image blackWhiteImage = function_imageBlackWhite(inputImage, 128);

    // denoise; shrink-expand-expand-shrink
    Image cleanedImage = shrink(expand(expand(shrink(blackWhiteImage))));

    // noise; probability = 5%
    Image noisedImage = function_noiseImage(cleanedImage, 5.0f);

    // label components
    Matrix labeledImageMatrix = labelComponents(cleanedImage);

    // color and count components; size threshold = 150
    Image coloredImage = colorAndCountComponents(cleanedImage, labeledImageMatrix, 150);

    // write imgs to files
    writeImage(blackWhiteImage, "black-white.pbm");
    writeImage(cleanedImage, "image_cleaned.pbm");
    writeImage(noisedImage, "noise.pbm");
    writeImage(coloredImage, "text_colored.ppm");

    // free memory; delete imgs and label matrix
    deleteImage(inputImage);
    deleteImage(blackWhiteImage);
    deleteImage(cleanedImage);
    deleteImage(noisedImage);
    deleteMatrix(labeledImageMatrix);
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

//-----------------------------component labeling------------------------------------------------------

// stuct to for equivalence pairs
typedef struct {
    int label1;
    int label2;
} EquivalencePair;

// struct to for equivalence table
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

// label components of image
Matrix labelComponents(Image img) {
    Matrix labelMatrix = createMatrix(img.height, img.width);   // initialize label matrix; 0 = unlabeled
    EquivalenceTable et = createEquivalenceTable();             // initialize equivalence table
    int currentLabel = 1;                                       // initialize lable counter

    // first pass; add labels and equivalences
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            // if pixel is obj
            if (img.map[i][j].i == 0) {
                int up = (i > 0) ? (int)labelMatrix.map[i-1][j] : 0;        // get label of above pixel
                int left = (j > 0) ? (int)labelMatrix.map[i][j-1] : 0;      // get label of left pixel
                
                if (up == 0 && left == 0) {                     // if above and left both unlabeled (0)
                    labelMatrix.map[i][j] = currentLabel++;         // add new label and increment
                } else if (up != 0 && left == 0) {              // if above labeled; left unlabeled
                    labelMatrix.map[i][j] = up;                     // set curr pixel label to above pixel label
                } else if (left != 0 && up == 0) {              // if left labeled; above unlabeled
                    labelMatrix.map[i][j] = left;                   // set curr pixel label to left pixel label
                } else{                                         // if above and left both labeled
                    labelMatrix.map[i][j] = up;                     // set curr pixel label to above pixel label
                    if (up != left) {                               // if above and left labels different
                        addEquivalence(&et, up, left);                  // add to equivalence table
                    }
                }
            }
        }
    }

    // allocate memory and create label map
    int *labelMap = (int *)malloc(currentLabel * sizeof(int));
    // initialize each label to map to itself
    for (int i = 0; i < currentLabel; i++) {
        labelMap[i] = i;
    }

    // process equivalences found in first pass
    for (int i = 0; i < et.count; i++) {
        int label1 = et.pairs[i].label1;
        int label2 = et.pairs[i].label2;
        int root1 = label1;
        int root2 = label2;
        
        // follow mapping chain to find root of label1 and label2
        while (labelMap[root1] != root1) {
            root1 = labelMap[root1];
        }
        while (labelMap[root2] != root2) {
            root2 = labelMap[root2];
        }
        
        // if roots are different, merge them
        if (root1 != root2) {
            // map larger root to smaller root
            if (root1 < root2) {
                labelMap[root2] = root1;
            } else {
                labelMap[root1] = root2;
            }
        }
    }

    // second pass; resolve labels
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            // if pixel is labeled (non-zero)
            if (labelMatrix.map[i][j] != 0) {
                // get initial label
                int label = (int)labelMatrix.map[i][j];

                // follow mapping chain to find root label
                while (labelMap[label] != label) {
                    label = labelMap[label];
                }

                // update label matrix with root label
                labelMatrix.map[i][j] = label;
            }
        }
    }

    // deallocated memory
    free(et.pairs);
    free(labelMap);

    return labelMatrix;
}

//-----------------------------------color and count components------------------------------------------

// generate random color
Pixel generateRandomColor() {
    Pixel color;
    do {
        color.r = rand() % 256;
        color.g = rand() % 256;
        color.b = rand() % 256;
    } while (color.r + color.g + color.b > 600 || color.r + color.g + color.b < 200);       // ensure color not too black or white
    return color;
}

// color and components of image
Image colorAndCountComponents(Image img, Matrix labelMatrix, int sizeThreshold) {
    // get max label / number of labels
    int maxLabel = 0;
    for (int i = 0; i < labelMatrix.height; i++) {
        for (int j = 0; j < labelMatrix.width; j++) {
            if (labelMatrix.map[i][j] > maxLabel) {
                maxLabel = labelMatrix.map[i][j];
            }
        }
    }

    // count size of each component
    int *componentSizes = (int *)calloc(maxLabel + 1, sizeof(int));
    for (int i = 0; i < labelMatrix.height; i++) {
        for (int j = 0; j < labelMatrix.width; j++) {
            if (labelMatrix.map[i][j] != 0) {
                componentSizes[(int)labelMatrix.map[i][j]]++;
            }
        }
    }

    // count components that meet size threshold
    // map random color to each component
    int count = 0;
    Pixel *colorMap = (Pixel *)malloc((maxLabel + 1) * sizeof(Pixel));
    for (int i = 1; i <= maxLabel; i++) {
        // if meets size threshold
        if (componentSizes[i] >= sizeThreshold) {
            colorMap[i] = generateRandomColor();
            count++;
        }
    }

    // create new img
    Image coloredImage = createImage(img.height, img.width);

    // color components
    for (int i = 0; i < labelMatrix.height; i++) {
        for (int j = 0; j < labelMatrix.width; j++) {
            // get label
            int label = (int)labelMatrix.map[i][j];

            // if labeled and component meets size threshold, then color pixel
            if (label != 0 && componentSizes[label] >= sizeThreshold) {
                coloredImage.map[i][j] = colorMap[label];
            } else {    // otherwise, copy pixel from original img
                coloredImage.map[i][j] = img.map[i][j];
            }
        }
    }

    // deallocated memory
    free(componentSizes);
    free(colorMap);

    // print number of components/letters
    printf("Number of Components: %d\n", count);

    return coloredImage;
}