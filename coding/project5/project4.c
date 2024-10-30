// 
// project5.c
// CS136
// 
// project5
// 
// created by Michael Huh 10/30/24
// last modified 10/30/24
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

/*
first, derive local feature vectors
that describe the texture at given coordinates (x, y). These features could be obtained
through Law’s filters, co-occurrence matrices, any other techniques that you know or
develop, or combinations of them. The resulting data could be stored in an array of tables, or you could
introduce a new data structure to hold the data.
*/

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