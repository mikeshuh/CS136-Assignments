// 
// project2.c
// CS136
// 
// project1_part1
// 
// created by Michael Huh 9/25/24
// last modified 9/25/24
// 
// compile:
// gcc -o main project2.c netpbm.c
// 
// run:
// ./main
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "netpbm.h"

int main(int argc, const char * argv[]) {
    // inputImage (read image file)
    Image inputImage = readImage("text_image.ppm");

    // free memory; delete imgs
    deleteImage(inputImage);

    printf("Program ends ...");
    return 0;
}

//----------------------------------------------smoothing filter------------------------------------------------------

// m1 input image matrix; m2 filter matrix
Matrix smoothing_filter(Matrix m1, Matrix m2) {

}

//-----------------------------------------------median filter--------------------------------------------------------

// m1 input image matrix; m2 filter matrix
Matrix median_filter(Matrix m1, Matrix m2) {

}