//
//  main.c
//  CS136
//
//  Created by nha2 on 8/27/24.
// Test and demo program for netpbm. Reads a sample image and creates several output images.

// Mac
// #define PATH "/Users/michaelhuh/Documents/SJSU/24-25/FA_24/CS-136_comp-vision/CS136-Assignments/coding/project1/netpbm/"

// PC
#define PATH "/Users/mshuh/SJSU/FA24/CS136-CompVision/Assignments/coding/project1/netpbm/"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"

// added
#include <time.h>

// Function prototypes
Image toBlackWhite(Image img, int intensityThreshold);
Image expand(Image img);
Image shrink(Image img);
Image expand4(Image img);
Image shrink4(Image img);
Image noiseImage(Image img, float p);

int main(int argc, const char * argv[]) {
    //-------------------------------------------------------------------------------
       //create blackWhiteImage:
        // You need to change the path file: /Users/nha2/Downloads/Proj1_part1_2/Proj1_part1_2/ to your local directory

      //  Image inputImage = readImage(PATH "car.ppm");

       //-------------------------------------------------------------------------------
       // COMMENT THIS FUNCTION IF YOU DON'T WANT IT TO RUN EVEY TIME
   //  Image rotatedImage = createImage(inputImage.height, inputImage.width);
   //  Image invertedImage = createImage(inputImage.height, inputImage.width);
   //     for (int x = 0; x < inputImage.width; x++)
   //         for (int y = 0; y < inputImage.height; y++)
   //         {
   //             rotatedImage.map[y][x] = inputImage.map[inputImage.height - y - 1][inputImage.width - x - 1];

   //             invertedImage.map[y][x].r = 255 - inputImage.map[y][x].r;
   //             invertedImage.map[y][x].g = 255 - inputImage.map[y][x].g;
   //             invertedImage.map[y][x].b = 255 - inputImage.map[y][x].b;
   //             // Let's just ignore 'i' here; it's irrelevant if we want to save image as PPM.
   //         }

      //  writeImage(rotatedImage, PATH "rotated.pbm");
      //  writeImage(invertedImage, PATH "inverted.pbm");
      //  writeImage(inputImage, PATH "gray.pgm");
      //  writeImage(inputImage, PATH "black-white.pbm");

    //-------------------------------------------------------------------------------

      // inputImage (read from file)
      Image inputImage = readImage(PATH "text_image.ppm");

      // blackWhiteImage
      Image blackWhiteTextImage = toBlackWhite(inputImage, 128);
      writeImage(blackWhiteTextImage, PATH "black-white.pbm");

      // Denoise
      // S = shrink; E = Expand
      // SEES & ESSE
         // S & E 8-connectedness
         Image cleanedTextImageSEES = shrink(expand(expand(shrink(blackWhiteTextImage))));
         Image cleanedTextImageESSE = expand(shrink(shrink(expand(blackWhiteTextImage))));
         
         // S 4-connectedness; E 8-connectedness
         Image cleanedTextImageSEES_s4 = shrink4(expand(expand(shrink4(blackWhiteTextImage))));
         Image cleanedTextImageESSE_s4 = expand(shrink4(shrink4(expand(blackWhiteTextImage))));

         // S 8-connectedness; E 4-connectedness
         Image cleanedTextImageSEES_e4 = shrink(expand4(expand4(shrink(blackWhiteTextImage))));
         Image cleanedTextImageESSE_e4 = expand4(shrink(shrink(expand4(blackWhiteTextImage))));

         // S & E 4-connectedness
         Image cleanedTextImageSEES_b4 = shrink4(expand4(expand4(shrink4(blackWhiteTextImage))));
         Image cleanedTextImageESSE_b4 = expand4(shrink4(shrink4(expand4(blackWhiteTextImage))));

      // Write to files
      writeImage(cleanedTextImageSEES, PATH "image_cleaned_SEES.pbm");
      writeImage(cleanedTextImageESSE, PATH "image_cleaned_ESSE.pbm");
      writeImage(cleanedTextImageSEES_s4, PATH "image_cleaned_SEES_s4.pbm");
      writeImage(cleanedTextImageESSE_s4, PATH "image_cleaned_ESSE_s4.pbm");
      writeImage(cleanedTextImageSEES_e4, PATH "image_cleaned_SEES_e4.pbm");
      writeImage(cleanedTextImageESSE_e4, PATH "image_cleaned_ESSE_e4.pbm");
      writeImage(cleanedTextImageSEES_b4, PATH "image_cleaned_SEES_b4.pbm");
      writeImage(cleanedTextImageESSE_b4, PATH "image_cleaned_ESSE_b4.pbm");

      // best res: sees_b4
      writeImage(cleanedTextImageSEES_b4, PATH "image_cleaned.pbm");

      // Create noisedImage and write to files
      Image noisedImage = noiseImage(cleanedTextImageSEES_b4, 5.0f);
      writeImage(noisedImage, PATH "noise.pbm");

       //-------------------------------------------------------------------------------
       // Uncomment this after you finish your homework
       // Function that does threshold, noise and numbers of expanding and shrinking
       // COMMENT THIS FUNCTION IF YOU DON'T WANT IT TO RUN EVEY TIME
       //function_readImage();

        /* Delete back and white, noise, Expand and Shrink */
      //  deleteImage(inputImage);
      //  deleteImage(rotatedImage);
      //  deleteImage(invertedImage);
      deleteImage(inputImage);
      deleteImage(blackWhiteTextImage);
      deleteImage(cleanedTextImageSEES);
      deleteImage(cleanedTextImageESSE);
      deleteImage(cleanedTextImageSEES_s4);
      deleteImage(cleanedTextImageESSE_s4);
      deleteImage(cleanedTextImageSEES_e4);
      deleteImage(cleanedTextImageESSE_e4);
      deleteImage(cleanedTextImageSEES_b4);
      deleteImage(cleanedTextImageESSE_b4);
      deleteImage(noisedImage);

      printf("Program ends ... ");

      return 0;
}

//-------------------------------function_imageBlackWhite-------------------------------------------------
/* function that receives an Image structure and an intensity threshold
 to convert each pixel in the image to either black (intensity = 0)
 or white (intensity = 255). The function should return an Image structure
 containing the result. */

Image toBlackWhite(Image img, int intensityThreshold) {
   // create new img
   Image bwImg = createImage(img.height, img.width);

   // iterate over original img
   for (int i = 0; i < img.height; i++) {
      for (int j = 0; j < img.width; j++) {
         // if img <= intensity threshold --> obj (i = 0)
         if (img.map[i][j].i <= intensityThreshold) {
            bwImg.map[i][j].i = 0;
         }
         // otherwise --> background (i = 255)
         else {
            bwImg.map[i][j].i = 255;
         }
         // set r, g, b to match i
         bwImg.map[i][j].r = bwImg.map[i][j].g = bwImg.map[i][j].b = bwImg.map[i][j].i;
      }
   }
   return bwImg;
}

//--------------------------------Expand function-----------------------------------------------------------
/* Expand operation */

// 8-connectedness expand
Image expand(Image img) {
   // create new img
   Image expImg = createImage(img.height, img.width);

   // copy original img to new img
   for (int i = 0; i < img.height; i++) {
      for (int j = 0; j < img.width; j++) {
         expImg.map[i][j] = img.map[i][j];
      }
   }

   // iterate over original img
   for (int i = 0; i < img.height; i++) {
      for (int j = 0; j < img.width; j++) {
         // check if pixel is black/obj
         if (img.map[i][j].i == 0) {
            // make neighboring pixels black mapped to new img
            for (int x = -1; x < 2; x++) {
               for (int y = -1; y < 2; y++) {
                  //adjusted idx
                  int s = i + x;
                  int t = j + y;
                  // check if neighbor is in bounds
                  if(s >= 0 && s < img.height && t >= 0 && t < img.width) {
                     // set i to 0
                     expImg.map[s][t].i = 0;
                     // set r, g, b to match i
                     expImg.map[s][t].r = expImg.map[s][t].g = expImg.map[s][t].b = expImg.map[s][t].i;
                  }
               }
            }
         }
      }
   }
   return expImg;
}



//--------------------------------Shrink function-----------------------------------------------------------
/* Shrink operation */

// 8-connectedness shrink
Image shrink(Image img) {
   // create new img
   Image shrinkImg = createImage(img.height, img.width);

   // copy original img to new img
   for (int i = 0; i < img.height; i++) {
      for (int j = 0; j < img.width; j++) {
         shrinkImg.map[i][j] = img.map[i][j];
      }
   }

   // iterate over original img
   for (int i = 0; i < img.height; i++) {
      for (int j = 0; j < img.width; j++) {
         // check if pixel is white/background
         if (img.map[i][j].i == 255) {
            // make neighboring pixels white mapped to new img
            for (int x = -1; x < 2; x++) {
               for (int y = -1; y < 2; y++) {
                  //adjusted idx
                  int s = i + x;
                  int t = j + y;
                  // check if neighbor is in bounds
                  if(s >= 0 && s < img.height && t >= 0 && t < img.width) {
                     // set i to 255
                     shrinkImg.map[s][t].i = 255;
                     // set r, g, b to match i
                     shrinkImg.map[s][t].r = shrinkImg.map[s][t].g = shrinkImg.map[s][t].b = shrinkImg.map[s][t].i;
                  }
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
Image noiseImage(Image img, float p) {
    Image noiseImg = createImage(img.height, img.width);
    
    // Seed the random number generator
    srand((unsigned int)time(NULL));
    
    for (int i = 0; i < img.height; i++) {
        for (int j = 0; j < img.width; j++) {
            int intensity = img.map[i][j].i;
            
            // Generate a random float between 0 and 1
            float random = (float)rand() / RAND_MAX;
            
            if (random <= p / 100.0f) {  // Convert p to a probability between 0 and 1
                noiseImg.map[i][j].i = (intensity == 0) ? 255 : 0;
            } else {
                noiseImg.map[i][j].i = intensity;
            }
            
            // Set r, g, b to match i
            noiseImg.map[i][j].r = noiseImg.map[i][j].g = noiseImg.map[i][j].b = noiseImg.map[i][j].i;
        }
    }
    
    return noiseImg;
}



// Expand operation using 4-connectedness
Image expand4(Image img) {
   // Create a new image to store the result
   Image expImg = createImage(img.height, img.width);

   // Copy the original image to the new image
   for (int i = 0; i < img.height; i++) {
      for (int j = 0; j < img.width; j++) {
         expImg.map[i][j] = img.map[i][j];
      }
   }

   // Define the neighbor offsets for 4-connectedness
   int dx[] = {-1, 0, 1, 0};
   int dy[] = {0, 1, 0, -1};

   // Iterate over each pixel in the original image
   for (int i = 0; i < img.height; i++) {
      for (int j = 0; j < img.width; j++) {
         // Check if the current pixel is black (object)
         if (img.map[i][j].i == 0) {
            // Set the 4-connected neighbors to black in the new image
            for (int k = 0; k < 4; k++) {
               int s = i + dx[k];
               int t = j + dy[k];
               // Check if neighbor is within bounds
               if (s >= 0 && s < img.height && t >= 0 && t < img.width) {
                  expImg.map[s][t].i = 0;
                  // Set r, g, b to match the intensity
                  expImg.map[s][t].r = expImg.map[s][t].g = expImg.map[s][t].b = 0;
               }
            }
         }
      }
   }
   return expImg;
}

// Shrink operation using 4-connectedness
Image shrink4(Image img) {
   // Create a new image to store the result
   Image shrinkImg = createImage(img.height, img.width);

   // Initialize the new image to the original image
   for (int i = 0; i < img.height; i++) {
      for (int j = 0; j < img.width; j++) {
         shrinkImg.map[i][j] = img.map[i][j];
      }
   }

   // Define the neighbor offsets for 4-connectedness
   int dx[] = {-1, 0, 1, 0};
   int dy[] = {0, 1, 0, -1};

   // Iterate over each pixel in the original image
   for (int i = 0; i < img.height; i++) {
      for (int j = 0; j < img.width; j++) {
         // Check if the current pixel is white (background)
         if (img.map[i][j].i == 255) {
            // Set the 4-connected neighbors to white in the new image
            for (int k = 0; k < 4; k++) {
               int s = i + dx[k];
               int t = j + dy[k];
               // Check if neighbor is within bounds
               if (s >= 0 && s < img.height && t >= 0 && t < img.width) {
                  shrinkImg.map[s][t].i = 255;
                  // Set r, g, b to match the intensity
                  shrinkImg.map[s][t].r = shrinkImg.map[s][t].g = shrinkImg.map[s][t].b = 255;
               }
            }
         }
      }
   }
   return shrinkImg;
}
