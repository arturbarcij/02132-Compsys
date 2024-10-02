#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "cbmp.h"

#define MAX_BOUNDARY_POINTS 10000
#define MAX_BOTTLENECK_POINTS 100

typedef struct {
    int x;
    int y;
} Point;

// Function prototypes
void binarize(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], int T);
void extractBoundary(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], Point boundary[], int *boundary_size);
void calculateSlopeDifference(Point boundary[], int boundary_size, float slope_diff[]);
void detectBottleneckPoints(Point boundary[], float slope_diff[], int boundary_size, Point bottleneck_points[], int *bottleneck_count);
void separateCells(Point bottleneck_points[], int bottleneck_count, unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH]);
void Convert23D(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);
int Otsu(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH]);

// Helper functions for distance and slope calculation
float calculateSlope(Point p1, Point p2);
float calculateDistance(Point p1, Point p2);

int main(int argc, char** argv) {
    // Declaring image arrays
    unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
    unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH];
    unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
    
    Point boundary[MAX_BOUNDARY_POINTS];
    Point bottleneck_points[MAX_BOTTLENECK_POINTS];
    int boundary_size = 0, bottleneck_count = 0;
    float slope_diff[MAX_BOUNDARY_POINTS];
    int T;

    // Check that the correct number of arguments are passed
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file path> <output file path>\n", argv[0]);
        exit(1);
    }

    printf("Bottleneck Detection Algorithm - Image Segmentation\n");

    // Step 1: Load image from file
    read_bitmap(argv[1], input_image);

    // Step 2: Convert image to grayscale
    rgb2gray(input_image, bin_image);

    // Step 3: Binarize the image using Otsu's method
    T = Otsu(bin_image);
    printf("Optimal Threshold: %d\n", T);
    binarize(bin_image, bin_image, T);

    // Step 4: Extract boundary points from the binary image
    extractBoundary(bin_image, boundary, &boundary_size);
    printf("Boundary points extracted: %d\n", boundary_size);

    // Step 5: Calculate slope difference along the boundary
    calculateSlopeDifference(boundary, boundary_size, slope_diff);

    // Step 6: Detect bottleneck points from slope differences
    detectBottleneckPoints(boundary, slope_diff, boundary_size, bottleneck_points, &bottleneck_count);
    printf("Bottleneck points detected: %d\n", bottleneck_count);

    // Step 7: Separate cells using bottleneck points
    separateCells(bottleneck_points, bottleneck_count, bin_image);

    // Step 8: Convert the final binary image back to RGB for visualization
    Convert23D(bin_image, output_image);

    // Step 9: Write the output image to file
    write_bitmap(output_image, argv[2]);

    printf("Image segmentation complete. Output saved to: %s\n", argv[2]);
    return 0;
}

// Binarize function using the threshold
void binarize(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], int T) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            bin_image[x][y] = (input_image[x][y] > T) ? 255 : 0;
        }
    }
}

// Extract boundary points from the binary image
void extractBoundary(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], Point boundary[], int *boundary_size) {
    *boundary_size = 0;
    for (int x = 1; x < BMP_WIDTH - 1; x++) {
        for (int y = 1; y < BMP_HEIGTH - 1; y++) {
            if (bin_image[x][y] == 255) {
                if (bin_image[x-1][y] == 0 || bin_image[x+1][y] == 0 ||
                    bin_image[x][y-1] == 0 || bin_image[x][y+1] == 0) {
                    boundary[*boundary_size].x = x;
                    boundary[*boundary_size].y = y;
                    (*boundary_size)++;
                }
            }
        }
    }
}

// Calculate slope difference between consecutive boundary points
void calculateSlopeDifference(Point boundary[], int boundary_size, float slope_diff[]) {
    for (int i = 1; i < boundary_size - 1; i++) {
        float slope1 = calculateSlope(boundary[i-1], boundary[i]);
        float slope2 = calculateSlope(boundary[i], boundary[i+1]);
        slope_diff[i] = fabsf(slope2 - slope1);
    }
}

// Detect bottleneck points based on slope difference distribution
void detectBottleneckPoints(Point boundary[], float slope_diff[], int boundary_size, Point bottleneck_points[], int *bottleneck_count) {
    *bottleneck_count = 0;
    for (int i = 1; i < boundary_size - 1; i++) {
        if (slope_diff[i] > slope_diff[i-1] && slope_diff[i] > slope_diff[i+1]) {
            bottleneck_points[*bottleneck_count] = boundary[i];
            (*bottleneck_count)++;
        }
    }
}

// Separate cells by drawing a line between bottleneck points
void separateCells(Point bottleneck_points[], int bottleneck_count, unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH]) {
    if (bottleneck_count < 2) return;

    float min_distance = calculateDistance(bottleneck_points[0], bottleneck_points[1]);
    int min_idx1 = 0, min_idx2 = 1;

    for (int i = 0; i < bottleneck_count; i++) {
        for (int j = i + 1; j < bottleneck_count; j++) {
            float distance = calculateDistance(bottleneck_points[i], bottleneck_points[j]);
            if (distance < min_distance) {
                min_distance = distance;
                min_idx1 = i;
                min_idx2 = j;
            }
        }
    }

    // Draw a line between the closest bottleneck points
    Point p1 = bottleneck_points[min_idx1];
    Point p2 = bottleneck_points[min_idx2];

    int dx = abs(p2.x - p1.x), sx = p1.x < p2.x ? 1 : -1;
    int dy = abs(p2.y - p1.y), sy = p1.y < p2.y ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;

    while (1) {
        bin_image[p1.x][p1.y] = 0;  // Set the pixel on the separating line to black
        if (p1.x == p2.x && p1.y == p2.y) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; p1.x += sx; }
        if (e2 < dy) { err += dx; p1.y += sy; }
    }
}

// Convert the 2D binary image back to RGB for visualization
void Convert23D(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            for (int c = 0; c < BMP_CHANNELS; c++) {
                output_image[x][y][c] = input_image[x][y];
            }
        }
    }
}

// Calculate slope between two points
float calculateSlope(Point p1, Point p2) {
    return (p2.y - p1.y) / (float)(p2.x - p1.x);
}

// Calculate Euclidean distance between two points
float calculateDistance(Point p1, Point p2) {
    return sqrtf((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

// Otsu's method for thresholding
int Otsu(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH]) {
    int hist[256] = {0};
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            hist[input_image[x][y]]++;
        }
    }

    int total_pixels = BMP_WIDTH * BMP_HEIGTH;
    float sum = 0;
    for (int i = 0; i < 256; i++) sum += i * hist[i];

    float sumB = 0, wB = 0, wF = 0;
    float maxVar = 0, threshold = 0;
    for (int t = 0; t < 256; t++) {
        wB += hist[t];
        if (wB == 0) continue;
        wF = total_pixels - wB;
        if (wF == 0) break;

        sumB += t * hist[t];
        float mB = sumB / wB;
        float mF = (sum - sumB) / wF;

        // Between-class variance
        float betweenVar = wB * wF * (mB - mF) * (mB - mF);

        // Check if this is the maximum variance we've seen
        if (betweenVar > maxVar) {
            maxVar = betweenVar;
            threshold = t;
        }
    }

    return (int)threshold;  // Return the optimal threshold
}
