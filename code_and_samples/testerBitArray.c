#include <stdio.h>


// Example dimensions (change according to your actual size)
#define PLACE_VALUE_X 2
#define BMP_HEIGTH 2

// CheckPlaceValue function
int checkPlaceValue(unsigned char output_image[PLACE_VALUE_X][BMP_HEIGTH], int placeX, int placeY)
{
    int pos = placeX / 8;
    int tempVal = output_image[pos][placeY];
    int bitPosition = 7 - (placeX % 8); //pos of the bit in the int
    
    // check if the bit at 1 or not 0
    if (tempVal & (1 << bitPosition)) {
        return 1; 
    }
    
    return 0; 
}

// MakeDead function
void makeDead(unsigned char output_image[PLACE_VALUE_X][BMP_HEIGTH], int placeX, int placeY)
{
    int pos = placeX / 8;
    int bitPosition = 7 - (placeX % 8); // Position of the bit in the byte (most significant is 7)
    
    // Set the bit at `bitPosition` to 0 (make it black)
    output_image[pos][placeY] &= ~(1 << bitPosition);
}

// Test the functions
int main(int argc, char** argv) {
    // Test image (integers in place of binary values)
    unsigned char output_image[PLACE_VALUE_X][BMP_HEIGTH] = {
        {123, 50}, // Row 0 (122 = 0b01111011, 50 = 0b00110010)
        {77, 88}   // Row 1 (77 = 0b01001101, 88 = 0b01011000)
    };
    
    // Test position: Check pixel at (placeX=3, placeY=0) which is the 4th pixel of the first byte (122)
    for(int i=0; i<16; i++)
    {
        int placeX = 12;
        int placeY = 0;

        // Check initial value of the pixel
        printf("%d", checkPlaceValue(output_image, placeX, placeY));
        
        // Make the pixel dead (set it to black)
        //makeDead(output_image, placeX, placeY);
        
        // Check the value again after making it dead
        //printf("Pixel value at (7, 0) after makeDead: %d\n", checkPlaceValue(output_image, placeX, placeY));
        //printf("Initial pixel value at (7, 0): %d\n", checkPlaceValue(output_image, placeX, placeY));
        
    }


    return 0;
}
