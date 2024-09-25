// 
// project1.c
// CS136
// 
// project1_part1
// 
// created by Michael Huh 9/17/24
// last modified 9/17/24
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

// function prototypes
Image function_imageBlackWhite(Image img, int intensityThreshold);
Image expand(Image img);
Image shrink(Image img);

int main(int argc, const char * argv[]) {
    // inputImage (read image file)
    Image inputImage = readImage("text_image.ppm");

    // to blackWhiteImage; intensity threshold = 128
    Image blackWhiteImage = function_imageBlackWhite(inputImage, 128);

    // denoise; shrink-expand-expand-shrink
    Image cleanedImage = shrink(expand(expand(shrink(blackWhiteImage))));

    // write imgs to files
    writeImage(blackWhiteImage, "black-white.pbm");
    writeImage(cleanedImage, "image_cleaned.pbm");

    // free memory; delete imgs
    deleteImage(inputImage);
    deleteImage(blackWhiteImage);
    deleteImage(cleanedImage);

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