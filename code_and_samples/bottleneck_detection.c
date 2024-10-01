#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BMP_WIDTH 950
#define BMP_HEIGHT 950

// Structure to store 2D points (for bottleneck points and boundary points)
typedef struct {
    int x;
    int y;
} Point;

// Function prototypes
void binarize(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], unsigned char bin_image[BMP_WIDTH][BMP_HEIGHT]);
void extractBoundary(unsigned char bin_image[BMP_WIDTH][BMP_HEIGHT], Point boundary[], int *boundary_size);
void calculateSlopeDifference(Point boundary[], int boundary_size, float slope_diff[]);
void detectBottleneckPoints(Point boundary[], float slope_diff[], int boundary_size, Point bottleneck_points[], int *bottleneck_count);
void separateCells(Point bottleneck_points[], int bottleneck_count, unsigned char bin_image[BMP_WIDTH][BMP_HEIGHT]);

// Helper functions for distance and slope calculation
float calculateSlope(Point p1, Point p2) {
    return (p2.y - p1.y) / (float)(p2.x - p1.x);
}

float calculateDistance(Point p1, Point p2) {
    return sqrtf((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

int main() {
    unsigned char input_image[BMP_WIDTH][BMP_HEIGHT];
    unsigned char bin_image[BMP_WIDTH][BMP_HEIGHT];

    // Step 1: Binarize the input image
    binarize(input_image, bin_image);

    // Step 2: Extract the boundary of the cells
    Point boundary[10000];
    int boundary_size = 0;
    extractBoundary(bin_image, boundary, &boundary_size);

    // Step 3: Calculate slope difference along the boundary
    float slope_diff[10000];
    calculateSlopeDifference(boundary, boundary_size, slope_diff);

    // Step 4: Detect bottleneck points from slope difference distribution
    Point bottleneck_points[100];
    int bottleneck_count = 0;
    detectBottleneckPoints(boundary, slope_diff, boundary_size, bottleneck_points, &bottleneck_count);

    // Step 5: Separate overlapping cells using bottleneck points
    separateCells(bottleneck_points, bottleneck_count, bin_image);

    // The image is now segmented, and the cells are separated
    printf("Cell separation complete.\n");

    return 0;
}

// Function to binarize the image (using a fixed threshold for simplicity)
void binarize(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT], unsigned char bin_image[BMP_WIDTH][BMP_HEIGHT]) {
    int threshold = 128;  // Simple fixed threshold (can be replaced with Otsu's method)
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGHT; y++) {
            bin_image[x][y] = (input_image[x][y] > threshold) ? 255 : 0;
        }
    }
}

// Function to extract the boundary of the binary image
void extractBoundary(unsigned char bin_image[BMP_WIDTH][BMP_HEIGHT], Point boundary[], int *boundary_size) {
    *boundary_size = 0;
    for (int x = 1; x < BMP_WIDTH - 1; x++) {
        for (int y = 1; y < BMP_HEIGHT - 1; y++) {
            if (bin_image[x][y] == 255) {
                // Check if this pixel has a black (0) neighbor, making it a boundary pixel
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

// Function to calculate slope difference distribution along the boundary
void calculateSlopeDifference(Point boundary[], int boundary_size, float slope_diff[]) {
    for (int i = 1; i < boundary_size - 1; i++) {
        // Slope between consecutive points
        float slope1 = calculateSlope(boundary[i-1], boundary[i]);
        float slope2 = calculateSlope(boundary[i], boundary[i+1]);
        slope_diff[i] = fabsf(slope2 - slope1);
    }
}

// Function to detect bottleneck points based on slope difference
void detectBottleneckPoints(Point boundary[], float slope_diff[], int boundary_size, Point bottleneck_points[], int *bottleneck_count) {
    *bottleneck_count = 0;
    for (int i = 1; i < boundary_size - 1; i++) {
        if (slope_diff[i] > slope_diff[i-1] && slope_diff[i] > slope_diff[i+1]) {
            // Local maxima in slope difference indicate bottleneck points
            bottleneck_points[*bottleneck_count] = boundary[i];
            (*bottleneck_count)++;
        }
    }
}

// Function to separate overlapping cells using bottleneck points
void separateCells(Point bottleneck_points[], int bottleneck_count, unsigned char bin_image[BMP_WIDTH][BMP_HEIGHT]) {
    if (bottleneck_count < 2) return;  // Not enough bottleneck points to separate cells

    // Find the closest pair of bottleneck points to split the cells
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

    // Draw a line between the two closest bottleneck points to separate the cells
    Point p1 = bottleneck_points[min_idx1];
    Point p2 = bottleneck_points[min_idx2];

    // Bresenham's line algorithm to draw a line between p1 and p2 in the binary image
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
