#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "cbmp.h"
#include <math.h>
#include <time.h>

#define BitBMP (BMP_WIDTH / 8 + 1)
#define BMP_HEIGHT 950
unsigned char output_2dbit[BitBMP][BMP_HEIGHT] = {0};
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_2d[BMP_WIDTH][BMP_HEIGTH];
unsigned char output_3d[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

// Function to invert pixels of an image (negative)
void invert(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]) {
    for (int x = 0; x < BMP_WIDTH; x++) 
    {
        for (int y = 0; y < BMP_HEIGHT; y++) 
        {
            for (int c = 0; c < BMP_CHANNELS; c++) 
            {
                output_image[x][y][c] = 255 - input_image[x][y][c];
            }
        }
    }
}

void rgb2gray(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char gray_image[BMP_WIDTH][BMP_HEIGHT]) {
    for (int y = 0; y < BMP_HEIGHT; y++) 
    {
        for (int x = 0; x < BMP_WIDTH; x++) 
        {
            gray_image[x][y] = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) * 0.3333;
        }
    }
}

void Binarize(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], unsigned char bin_image[BitBMP][BMP_HEIGHT], int T) {
    if (T < 0 || T > 255) 
    {
        printf("Setting threshold to default (90)");
        T = 90;
    }
    for (int y = 0; y < BMP_HEIGHT; y++) 
    {
        for (int x = 0; x < BMP_WIDTH; x++) 
        {
            int t = floor(x / 8);
            if (input_image[x][y] > T) 
            {
                bin_image[t][y] |= (1 << (7 - (x % 8)));  // Set bit
            } 
            else 
            {
                bin_image[t][y] &= ~(1 << (7 - (x % 8)));  // Clear bit
            }
        }
    }
}

void Convert23D(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]) {
    for (int x = 0; x < BMP_WIDTH; x++) 
    {
        for (int y = 0; y < BMP_HEIGHT; y++) 
        {
            for (int c = 0; c < BMP_CHANNELS; c++) 
            {
                output_image[x][y][c] = input_image[x][y];
            }
        }
    }
}

// takes position and returns whether it is 1 or o
int checkPlaceValue(unsigned char output_image[BitBMP][BMP_HEIGHT], int placeX, int placeY) {
    int pos = placeX / 8;
    int tempVal = output_image[pos][placeY];
    int bitPosition = 7 - (placeX % 8);  
    return (tempVal & (1 << bitPosition)) != 0 ? 1 : 0;  
}

//makes pixel dead, makes the bit from 1 to 0
void makeDead(unsigned char output_image[BitBMP][BMP_HEIGHT], int placeX, int placeY) {
    int pos = placeX / 8;
    int bitPosition = 7 - (placeX % 8);  // Position of the bit in the byte
    output_image[pos][placeY] &= ~(1 << bitPosition);  // Set bit to 0 
}

void erode(unsigned char output_2dbit[BitBMP][BMP_HEIGHT]) {
    int arr_x[100000];
    int arr_y[100000];
    int i = 0;
    for (int x = 0; x < BMP_WIDTH; x++) 
    {
        for (int y = 0; y < BMP_HEIGHT; y++) 
        {
            if (checkPlaceValue(output_2dbit, x, y) == 1) 
            {
                // Cross-erosion
                if ((x > 0 && checkPlaceValue(output_2dbit, x - 1, y) == 0) ||
                    (x < BMP_WIDTH - 1 && checkPlaceValue(output_2dbit, x + 1, y) == 0) ||
                    (y > 0 && checkPlaceValue(output_2dbit, x, y - 1) == 0) ||
                    (y < BMP_HEIGHT - 1 && checkPlaceValue(output_2dbit, x, y + 1) == 0)) {
                    arr_x[i] = x;
                    arr_y[i] = y;
                    i++;
                    if (i >= 100000) 
                    {
                        printf("Exceeded boundary in erosion\n");
                        return;
                    }
                }
            }
        }
    }

    for (int t = 0; t < i; t++) 
    {
        makeDead(output_2dbit, arr_x[t], arr_y[t]);
    }
}

int Otsu(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT]) {
    // Otsu's thresholding method to find the optimal threshold
    float hist[256] = {0};
    float q1 = 0, mu1 = 0, mu2 = 0, mu = 0, betweenvariance = 0;
    float maxbetweenvariance = 0;
    float optimizedthresh = 0;
    double sum = 0;

    // Create a histogram of the grayscale image
    for (int x = 0; x < BMP_WIDTH; x++) 
    {
        for (int y = 0; y < BMP_HEIGHT; y++) 
        {
            hist[input_image[x][y]]++;
            sum += input_image[x][y];
        }
    }

    // Normalize the histogram
    for (int i = 0; i < 256; i++) 
    {
        hist[i] = hist[i] / (BMP_WIDTH * BMP_HEIGHT);
    }

    // Calculate the overall mean gray level
    mu = sum / (BMP_WIDTH * BMP_HEIGHT);

    // Iterate over all possible threshold values (0-255)
    for (int t = 0; t < 256; t++) 
    {
        float q1next = q1 + hist[t];
        if (q1next == 0 || q1next == 1) continue;

        float mu1next = (q1 * mu1 + t * hist[t]) / q1next;
        float mu2next = (mu - q1next * mu1next) / (1 - q1next);

        betweenvariance = q1 * (1 - q1) * (mu1 - mu2) * (mu1 - mu2);

        if (betweenvariance > maxbetweenvariance) 
        {
            maxbetweenvariance = betweenvariance;
            optimizedthresh = t;
        }

        q1 = q1next;
        mu1 = mu1next;
        mu2 = mu2next;
    }

    return (int)optimizedthresh;
}
int Detection(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], int x_coords[500], int y_coords[500], int cell_detected) {
    int detect_flag = 0;

    for (int x = 6; x < BMP_WIDTH - 6; x++) 
    {
        for (int y = 6; y < BMP_HEIGHT - 6; y++) 
        { // looping through the eroded image
            detect_flag = 0;

            if (input_image[x][y] == 255) { // if a white pixel is detected
                for (int i = 0; i < 8; i++) 
                {
                    if ((input_image[x + 7][y + i] == 255 || input_image[x - 7][y - i] == 255 ||
                         input_image[x - 7][y + i] == 255 || input_image[x + 7][y - i] == 255 ||
                         input_image[x + i][y + 7] == 255 || input_image[x - i][y - 7] == 255 ||
                         input_image[x - i][y + 7] == 255 || input_image[x + i][y - 7] == 255)) 
                        
                    {
                        break;
                    } 
                    else 
                    {
                        if (i == 7 && !detect_flag) 
                        {
                            detect_flag = 1;
                            cell_detected += 1;
                            x_coords[cell_detected] = x;
                            y_coords[cell_detected] = y;

                            for (int p = 0; p < 7; p++) 
                            {
                                for (int q = 0; q < 7; q++) 
                                {
                                    if ((x + p >= BMP_WIDTH) || (y + q >= BMP_HEIGHT) || (x - p < 0) || (y - q < 0)) 
                                    {
                                        fprintf(stderr, "Out of bounds\n");
                                        break;
                                    }
                                    input_image[x + p][y + q] = 0;
                                    input_image[x + p][y - q] = 0;
                                    input_image[x - p][y + q] = 0;
                                    input_image[x - p][y - q] = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return cell_detected;
}

void DrawCrosses(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x_coords[500], int y_coords[500], int cell_detected) {
    for (int i = 0; i < cell_detected; i++) 
    {
        if ((x_coords[i] != 0) && (y_coords[i] != 0)) 
        {
            for (int x = 0; x < 7; x++) 
            {
                for (int w = -1; w <= 1; w++) 
                {
                    input_image[x_coords[i] + x][y_coords[i] + w][0] = 255;
                    input_image[x_coords[i] + x][y_coords[i] + w][1] = 0;
                    input_image[x_coords[i] + x][y_coords[i] + w][2] = 0;

                    input_image[x_coords[i] - x][y_coords[i] + w][0] = 255;
                    input_image[x_coords[i] - x][y_coords[i] + w][1] = 0;
                    input_image[x_coords[i] - x][y_coords[i] + w][2] = 0;

                    input_image[x_coords[i] + w][y_coords[i] + x][0] = 255;
                    input_image[x_coords[i] + w][y_coords[i] + x][1] = 0;
                    input_image[x_coords[i] + w][y_coords[i] + x][2] = 0;

                    input_image[x_coords[i] + w][y_coords[i] - x][0] = 255;
                    input_image[x_coords[i] + w][y_coords[i] - x][1] = 0;
                    input_image[x_coords[i] + w][y_coords[i] - x][2] = 0;
                }
            }
        }
    }
}


int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file path> <output file path>\n", argv[0]);
        exit(1);
    }

    //load imag
    read_bitmap(argv[1], input_image);

    // make grayscale
    rgb2gray(input_image, output_2d);

    // use otsu
    int x_coords[500];
    int y_coords[500];
    int countDetects = 0;
    int T;
    int count = 0;
    int detects_old = 0;
    int detecs_old_old = 0;
    T = Otsu(output_2d);

    // binarize
    Binarize(output_2d, output_2dbit, T);

    // erode
    while (1) {
        erode(output_2dbit);
        countDetects = Detection(output_2d, x_coords, y_coords, countDetects);
        detects_old = countDetects;
        count++;
        if (detects_old == countDetects && count > 10) {
            break;
        }
    }

    // print number of cells
    printf("Number of cells detected: %d\n", countDetects);

    // convert array back to 3dim array
    Convert23D(output_2d, output_3d);

    // draw the crosses
    DrawCrosses(input_image, x_coords, y_coords, countDetects);

    // print out the image again
    write_bitmap(input_image, argv[2]);

    printf("Done!\n");
    return 0;
}
