// main.c
// Corrected version to output the result of the algorithm with crosses drawn over detected cells

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "cbmp.h"

// Define maximum sizes based on your image dimensions
#define MAX_BOUNDARY_POINTS 100000
#define MAX_BOTTLENECK_POINTS 1000
#define MAX_CELLS 500
#define PI 3.14159265358979323846

// Data structures
typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point points[MAX_BOUNDARY_POINTS];
    int count;
} Boundary;

typedef struct {
    double values[MAX_BOUNDARY_POINTS];
    int count;
} DoubleArray;

typedef struct {
    Point point;
    double value;
} BottleneckPoint;

typedef struct {
    Point centroid;
    Boundary boundary;
} Cell;

// Function prototypes
void rgb2gray(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char gray_image[BMP_WIDTH][BMP_HEIGTH]);
void binarize(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], int T);
int otsu_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH]);
void find_boundary(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], Boundary* boundary, int start_x, int start_y, bool visited[BMP_WIDTH][BMP_HEIGTH]);
void calculate_centroid(Boundary* boundary, double* centroid_x, double* centroid_y);
void calculate_distance_profile(Boundary* boundary, double centroid_x, double centroid_y, DoubleArray* distance_profile);
void calculate_slope_differences(DoubleArray* distance_profile, DoubleArray* slope_differences);
void find_bottleneck_points(DoubleArray* slope_differences, Boundary* boundary, BottleneckPoint* bottleneck_points, int* bottleneck_count);
void calculate_curvature(Boundary* boundary, DoubleArray* curvature);
void validate_bottleneck_points(BottleneckPoint* bottleneck_points, int* bottleneck_count, DoubleArray* curvature, Boundary* boundary);
void find_minimum_distance_pair(BottleneckPoint* bottleneck_points, int bottleneck_count, Point* point1, Point* point2);
void draw_line(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], Point p1, Point p2);
void process_image(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH]);
void label_cells(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], Cell cells[MAX_CELLS], int* cell_count);
void draw_crosses(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], Cell cells[MAX_CELLS], int cell_count);
void overlay_boundaries(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], Cell cells[MAX_CELLS], int cell_count);

// Main function
int main(int argc, char** argv)
{
    // Checking that 2 arguments are passed
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input file path> <output file path>\n", argv[0]);
        exit(1);
    }

    printf("Cell Segmentation using Bottleneck Detection Algorithm\n");

    // Load image from file
    unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
    read_bitmap(argv[1], input_image);

    // Convert RGB to grayscale
    unsigned char gray_image[BMP_WIDTH][BMP_HEIGTH];
    rgb2gray(input_image, gray_image);

    // Binarize the image using Otsu's method
    int T = otsu_threshold(gray_image);
    printf("Optimized threshold: %d\n", T);

    unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH];
    binarize(gray_image, bin_image, T);

    // Process the image to separate overlapping cells
    process_image(bin_image);

    // Label and collect cell data after segmentation
    Cell cells[MAX_CELLS];
    int cell_count = 0;
    label_cells(bin_image, cells, &cell_count);

    printf("Number of cells detected: %d\n", cell_count);

    // Prepare output image by copying the original image
    unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
    memcpy(output_image, input_image, sizeof(output_image));

    // Draw cell boundaries on the output image
    overlay_boundaries(output_image, cells, cell_count);

    // Draw crosses on the centroids of the detected cells
    draw_crosses(output_image, cells, cell_count);

    // Save the processed image with crosses and boundaries
    write_bitmap(output_image, argv[2]);

    printf("Processing completed. Output saved to %s\n", argv[2]);
    return 0;
}

// Function implementations

void rgb2gray(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char gray_image[BMP_WIDTH][BMP_HEIGTH]){
    for (int x = 0; x < BMP_WIDTH; x++)
    {
        for (int y = 0; y < BMP_HEIGTH; y++)
        {
            gray_image[x][y] = (unsigned char)((input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3);
        }
    }
}

void binarize(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], int T){
    for (int x = 0; x < BMP_WIDTH; x++)
    {
        for (int y = 0; y < BMP_HEIGTH; y++)
        {
            if (input_image[x][y] > T)
            {
                bin_image[x][y] = 255;
            }
            else
            {
                bin_image[x][y] = 0;
            }
        }
    }
}

int otsu_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH]) {
    // Otsu's method implementation
    int histogram[256] = {0};
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            histogram[input_image[x][y]]++;
        }
    }

    int total = BMP_WIDTH * BMP_HEIGTH;
    float sum = 0;
    for (int t = 0; t < 256; t++) sum += t * histogram[t];

    float sumB = 0;
    int wB = 0;
    int wF = 0;

    float varMax = 0;
    int threshold = 0;

    for (int t = 0; t < 256; t++) {
        wB += histogram[t];
        if (wB == 0) continue;
        wF = total - wB;
        if (wF == 0) break;

        sumB += (float)(t * histogram[t]);

        float mB = sumB / wB;
        float mF = (sum - sumB) / wF;

        float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);

        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = t;
        }
    }
    return threshold;
}

// Bottleneck Detection Algorithm Functions

void find_boundary(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], Boundary* boundary, int start_x, int start_y, bool visited[BMP_WIDTH][BMP_HEIGTH]) {
    int width = BMP_WIDTH;
    int height = BMP_HEIGTH;

    // Initialize boundary point list
    boundary->count = 0;

    int x = start_x;
    int y = start_y;
    int dir = 0; // Direction (0 to 7 for 8-connected)

    // Direction vectors for 8-connected neighborhood
    int dx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    int dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};

    do {
        // Add point to boundary
        if (boundary->count < MAX_BOUNDARY_POINTS) {
            boundary->points[boundary->count].x = x;
            boundary->points[boundary->count].y = y;
            boundary->count++;
            visited[x][y] = true;
        } else {
            printf("Boundary points exceed maximum limit.\n");
            break;
        }

        // Find next boundary point
        bool found_next = false;
        for (int i = 0; i < 8; i++) {
            int new_dir = (dir + i) % 8;
            int nx = x + dx[new_dir];
            int ny = y + dy[new_dir];

            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                if (bin_image[nx][ny] == 255 && !visited[nx][ny]) {
                    x = nx;
                    y = ny;
                    dir = (new_dir + 6) % 8; // Adjust direction
                    found_next = true;
                    break;
                }
            }
        }
        if (!found_next) {
            // No next point found, end of boundary
            break;
        }
    } while (x != start_x || y != start_y);
}

void calculate_centroid(Boundary* boundary, double* centroid_x, double* centroid_y) {
    double sum_x = 0;
    double sum_y = 0;
    for (int i = 0; i < boundary->count; i++) {
        sum_x += boundary->points[i].x;
        sum_y += boundary->points[i].y;
    }
    *centroid_x = sum_x / boundary->count;
    *centroid_y = sum_y / boundary->count;
}

void calculate_distance_profile(Boundary* boundary, double centroid_x, double centroid_y, DoubleArray* distance_profile) {
    distance_profile->count = boundary->count;

    for (int i = 0; i < boundary->count; i++) {
        int x = boundary->points[i].x;
        int y = boundary->points[i].y;
        double dx = x - centroid_x;
        double dy = y - centroid_y;
        distance_profile->values[i] = sqrt(dx * dx + dy * dy);
    }
}

void calculate_slope_differences(DoubleArray* distance_profile, DoubleArray* slope_differences) {
    // First, calculate the slopes
    int n = distance_profile->count;
    double slopes[MAX_BOUNDARY_POINTS];

    for (int i = 0; i < n; i++) {
        int next = (i + 1) % n;
        slopes[i] = distance_profile->values[next] - distance_profile->values[i];
    }

    // Now, calculate the slope differences
    slope_differences->count = n;

    for (int i = 0; i < n; i++) {
        int next = (i + 1) % n;
        slope_differences->values[i] = slopes[next] - slopes[i];
    }
}

void find_bottleneck_points(DoubleArray* slope_differences, Boundary* boundary, BottleneckPoint* bottleneck_points, int* bottleneck_count) {
    double threshold = 0.5; // Threshold for local maxima; adjust as needed
    int count = 0;

    for (int i = 1; i < slope_differences->count - 1; i++) {
        double prev = slope_differences->values[i - 1];
        double curr = slope_differences->values[i];
        double next = slope_differences->values[i + 1];

        if (curr > prev && curr > next && curr > threshold) {
            // Local maximum found
            bottleneck_points[count].point = boundary->points[i];
            bottleneck_points[count].value = curr;
            count++;
            if (count >= MAX_BOTTLENECK_POINTS) break; // Prevent overflow
        }
    }
    *bottleneck_count = count;
}

void calculate_curvature(Boundary* boundary, DoubleArray* curvature) {
    int n = boundary->count;
    curvature->count = n;

    for (int i = 0; i < n; i++) {
        int prev = (i - 1 + n) % n;
        int next = (i + 1) % n;

        int x0 = boundary->points[prev].x;
        int y0 = boundary->points[prev].y;
        int x1 = boundary->points[i].x;
        int y1 = boundary->points[i].y;
        int x2 = boundary->points[next].x;
        int y2 = boundary->points[next].y;

        double dx1 = x1 - x0;
        double dy1 = y1 - y0;
        double dx2 = x2 - x1;
        double dy2 = y2 - y1;

        double cross = dx1 * dy2 - dy1 * dx2;
        double dot = dx1 * dx2 + dy1 * dy2;
        double angle = atan2(cross, dot);
        curvature->values[i] = angle;
    }
}

void validate_bottleneck_points(BottleneckPoint* bottleneck_points, int* bottleneck_count, DoubleArray* curvature, Boundary* boundary) {
    int count = *bottleneck_count;
    int valid_count = 0;
    for (int i = 0; i < count; i++) {
        int index = -1;
        // Find index of bottleneck point in boundary
        for (int j = 0; j < boundary->count; j++) {
            if (boundary->points[j].x == bottleneck_points[i].point.x &&
                boundary->points[j].y == bottleneck_points[i].point.y) {
                index = j;
                break;
            }
        }
        if (index != -1) {
            double curv = curvature->values[index];
            if (curv < 0) {
                // Negative curvature (concave)
                bottleneck_points[valid_count] = bottleneck_points[i];
                valid_count++;
            }
        }
    }
    *bottleneck_count = valid_count;
}

void find_minimum_distance_pair(BottleneckPoint* bottleneck_points, int bottleneck_count, Point* point1, Point* point2) {
    double min_distance = INFINITY;
    for (int i = 0; i < bottleneck_count - 1; i++) {
        for (int j = i + 1; j < bottleneck_count; j++) {
            int x1 = bottleneck_points[i].point.x;
            int y1 = bottleneck_points[i].point.y;
            int x2 = bottleneck_points[j].point.x;
            int y2 = bottleneck_points[j].point.y;

            double dx = x1 - x2;
            double dy = y1 - y2;
            double distance = sqrt(dx * dx + dy * dy);

            if (distance < min_distance) {
                min_distance = distance;
                *point1 = bottleneck_points[i].point;
                *point2 = bottleneck_points[j].point;
            }
        }
    }
}

void draw_line(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], Point p1, Point p2) {
    int x0 = p1.x;
    int y0 = p1.y;
    int x1 = p2.x;
    int y1 = p2.y;

    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        // Set pixel to background
        if (x0 >= 0 && x0 < BMP_WIDTH && y0 >= 0 && y0 < BMP_HEIGTH) {
            bin_image[x0][y0] = 0;
        }

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void process_image(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH]) {
    bool changes_made = true;

    while (changes_made) {
        changes_made = false;

        bool visited[BMP_WIDTH][BMP_HEIGTH] = {false};

        for (int y = 0; y < BMP_HEIGTH; y++) {
            for (int x = 0; x < BMP_WIDTH; x++) {
                if (bin_image[x][y] == 255 && !visited[x][y]) {
                    // Start processing a new clump
                    Boundary boundary;
                    find_boundary(bin_image, &boundary, x, y, visited);

                    if (boundary.count == 0) {
                        continue;
                    }

                    // Process the clump
                    // Calculate the centroid of the boundary
                    double centroid_x, centroid_y;
                    calculate_centroid(&boundary, &centroid_x, &centroid_y);

                    // Compute the distance profile
                    DoubleArray distance_profile;
                    calculate_distance_profile(&boundary, centroid_x, centroid_y, &distance_profile);

                    // Calculate the slope differences
                    DoubleArray slope_differences;
                    calculate_slope_differences(&distance_profile, &slope_differences);

                    // Identify bottleneck points
                    BottleneckPoint bottleneck_points[MAX_BOTTLENECK_POINTS];
                    int bottleneck_count = 0;
                    find_bottleneck_points(&slope_differences, &boundary, bottleneck_points, &bottleneck_count);

                    // Calculate curvature
                    DoubleArray curvature;
                    calculate_curvature(&boundary, &curvature);

                    // Validate bottleneck points
                    validate_bottleneck_points(bottleneck_points, &bottleneck_count, &curvature, &boundary);

                    if (bottleneck_count >= 2) {
                        // Find minimum distance pair
                        Point p1, p2;
                        find_minimum_distance_pair(bottleneck_points, bottleneck_count, &p1, &p2);

                        // Draw line between bottleneck points to separate cells
                        draw_line(bin_image, p1, p2);

                        // Mark that changes were made
                        changes_made = true;
                    }
                }
            }
        }
    }
}

void label_cells(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], Cell cells[MAX_CELLS], int* cell_count) {
    bool visited[BMP_WIDTH][BMP_HEIGTH] = {false};
    int cell_idx = 0;

    for (int y = 0; y < BMP_HEIGTH; y++) {
        for (int x = 0; x < BMP_WIDTH; x++) {
            if (bin_image[x][y] == 255 && !visited[x][y]) {
                if (cell_idx >= MAX_CELLS) {
                    printf("Maximum number of cells exceeded.\n");
                    break;
                }

                // Find the boundary of the cell
                Boundary boundary;
                find_boundary(bin_image, &boundary, x, y, visited);

                if (boundary.count == 0) {
                    continue;
                }

                // Calculate centroid
                double centroid_x, centroid_y;
                calculate_centroid(&boundary, &centroid_x, &centroid_y);

                // Save cell data
                cells[cell_idx].boundary = boundary;
                cells[cell_idx].centroid.x = (int)centroid_x;
                cells[cell_idx].centroid.y = (int)centroid_y;

                cell_idx++;
            }
        }
    }
    *cell_count = cell_idx;
}

void draw_crosses(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], Cell cells[MAX_CELLS], int cell_count){
    for (int i = 0; i < cell_count; i++){
        int cx = cells[i].centroid.x;
        int cy = cells[i].centroid.y;

        // Draw a cross centered at (cx, cy)
        int size = 7; // Size of the cross
        for (int x = -size; x <= size; x++){
            if (cx + x >= 0 && cx + x < BMP_WIDTH){
                if (cy >= 0 && cy < BMP_HEIGTH){
                    // Horizontal line
                    input_image[cx + x][cy][0] = 255; // Red
                    input_image[cx + x][cy][1] = 0;
                    input_image[cx + x][cy][2] = 0;
                }
            }
        }
        for (int y = -size; y <= size; y++){
            if (cy + y >= 0 && cy + y < BMP_HEIGTH){
                if (cx >= 0 && cx < BMP_WIDTH){
                    // Vertical line
                    input_image[cx][cy + y][0] = 255; // Red
                    input_image[cx][cy + y][1] = 0;
                    input_image[cx][cy + y][2] = 0;
                }
            }
        }
    }
}

void overlay_boundaries(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], Cell cells[MAX_CELLS], int cell_count) {
    for (int i = 0; i < cell_count; i++) {
        Boundary boundary = cells[i].boundary;
        for (int j = 0; j < boundary.count; j++) {
            int x = boundary.points[j].x;
            int y = boundary.points[j].y;

            if (x >= 0 && x < BMP_WIDTH && y >= 0 && y < BMP_HEIGTH) {
                output_image[x][y][0] = 0;   // Blue
                output_image[x][y][1] = 0;
                output_image[x][y][2] = 255;
            }
        }
    }
}
