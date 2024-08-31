//
//  main.c
//  CS136
//
//  Created by nha2 on 8/27/24.
// Test and demo program for netpbm. Reads a sample image and creates several output images.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"
#include <stdio.h>




int main(int argc, const char * argv[]) {
    //-------------------------------------------------------------------------------
       //create blackWhiteImage:
        // You need to change the path file: /Users/nha2/Downloads/Proj1_part1_2/Proj1_part1_2/ to your local directory
       Image inputImage = readImage("/Users/mshuh/SJSU/FA24/CS136-CompVision/Assignments/project1/netpbm/car.ppm");

       //-------------------------------------------------------------------------------
       // COMMENT THIS FUNCTION IF YOU DON'T WANT IT TO RUN EVEY TIME
    Image rotatedImage = createImage(inputImage.height, inputImage.width);
    Image invertedImage = createImage(inputImage.height, inputImage.width);
       for (int x = 0; x < inputImage.width; x++)
           for (int y = 0; y < inputImage.height; y++)
           {
               rotatedImage.map[y][x] = inputImage.map[inputImage.height - y - 1][inputImage.width - x - 1];

               invertedImage.map[y][x].r = 255 - inputImage.map[y][x].r;
               invertedImage.map[y][x].g = 255 - inputImage.map[y][x].g;
               invertedImage.map[y][x].b = 255 - inputImage.map[y][x].b;
               // Let's just ignore 'i' here; it's irrelevant if we want to save image as PPM.
           }
       writeImage(rotatedImage, "/Users/mshuh/SJSU/FA24/CS136-CompVision/Assignments/project1/netpbm/rotated.pbm");
       writeImage(invertedImage, "/Users/mshuh/SJSU/FA24/CS136-CompVision/Assignments/project1/netpbm/inverted.pbm");
       writeImage(inputImage, "/Users/mshuh/SJSU/FA24/CS136-CompVision/Assignments/project1/netpbm/gray.pgm");
       writeImage(inputImage, "/Users/mshuh/SJSU/FA24/CS136-CompVision/Assignments/project1/netpbm/black-white.pbm");

    //-------------------------------------------------------------------------------
       //create blackWhiteImage:


       // create noiseImage:


       // create expandImage:


       // create shrinkImage:


       //-------------------------------------------------------------------------------
       // Uncomment this after you finish your homework
       // Function that does threshold, noise and numbers of spanding and shrinking
       // COMMENT THIS FUNCTION IF YOU DON'T WANT IT TO RUN EVEY TIME
       //function_readImage();

        /* Delete back and white, noise, Expand and Shrink */
       deleteImage(inputImage);
       deleteImage(rotatedImage);
       deleteImage(invertedImage);




       printf("Program ends ... ");


       return 0;
}



//-------------------------------function_imageBlackWhite-------------------------------------------------
/* function that receives an Image structure and an intensity threshold
 to convert each pixel in the image to either black (intensity = 0)
 or white (intensity = 255). The function should return an Image structure
 containing the result. */




//--------------------------------Expand function-----------------------------------------------------------
/* Expand operation */




//--------------------------------Shrink function-----------------------------------------------------------
/* Shrink operation */



//--------------------------------noise function-----------------------------------------------------------
/* function that adds binary noise to an image. This function receives an
 image and a floating point number p that indicates the probability
 (in percent) that each pixel in the image will be flipped, i.e.,
 turned from black to white or vice versa.
 */
