// main.c
// Corrected and debugged code with expected output

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

// Function prototypes
void invert(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);
void rgb2gray(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char gray_image[BMP_WIDTH][BMP_HEIGTH]);
void Binarize(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], int T);
void Convert23D(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]);
void DrawCrosses(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x_coords[MAX_CELLS], int y_coords[MAX_CELLS], int cell_detected);
int Otsu(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH]);

// Bottleneck detection functions
void process_image(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH]);
void find_boundary(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], Boundary* boundary, int start_x, int start_y, bool visited[BMP_WIDTH][BMP_HEIGTH]);
void calculate_centroid(Boundary* boundary, double* centroid_x, double* centroid_y);
void calculate_distance_profile(Boundary* boundary, double centroid_x, double centroid_y, DoubleArray* distance_profile);
void calculate_slope_differences(DoubleArray* distance_profile, DoubleArray* slope_differences);
void find_bottleneck_points(DoubleArray* slope_differences, Boundary* boundary, BottleneckPoint* bottleneck_points, int* bottleneck_count);
void calculate_curvature(Boundary* boundary, DoubleArray* curvature);
void validate_bottleneck_points(BottleneckPoint* bottleneck_points, int* bottleneck_count, DoubleArray* curvature, Boundary* boundary);
void find_minimum_distance_pair(BottleneckPoint* bottleneck_points, int bottleneck_count, Point* point1, Point* point2);
void draw_line(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], Point p1, Point p2);

// Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_2d[BMP_WIDTH][BMP_HEIGTH];
unsigned char output_3d[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

int x_coords[MAX_CELLS];
int y_coords[MAX_CELLS];
int countDetects = 0;
int T;

// Main function
int main(int argc, char** argv)
{
    // Checking that 2 arguments are passed
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input file path> <output file path>\n", argv[0]);
        exit(1);
    }

    printf("Example program - 02132 - A1\n");

    // Load image from file
    read_bitmap(argv[1], input_image);
    printf("Image loaded successfully.\n");

    // Convert RGB to grayscale
    rgb2gray(input_image, output_2d);
    printf("Image converted to grayscale.\n");

    // Binarize the image using Otsu's method
    T = Otsu(output_2d);
    printf("Optimized threshold: %d\n", T);
    Binarize(output_2d, output_2d, T);
    printf("Image binarized.\n");

    // Process the image to separate overlapping cells
    printf("Starting image processing...\n");
    process_image(output_2d);
    printf("Image processing completed.\n");

    // Detect cells and store their centroids
    bool visited[BMP_WIDTH][BMP_HEIGTH] = {false};
    countDetects = 0;
    printf("Starting cell detection...\n");

    for (int y = 0; y < BMP_HEIGTH; y++) {
        for (int x = 0; x < BMP_WIDTH; x++) {
            if (output_2d[x][y] == 255 && !visited[x][y]) {
                Boundary boundary;
                find_boundary(output_2d, &boundary, x, y, visited);

                if (boundary.count == 0) {
                    continue;
                }

                // Calculate centroid
                double centroid_x, centroid_y;
                calculate_centroid(&boundary, &centroid_x, &centroid_y);

                if (countDetects < MAX_CELLS) {
                    x_coords[countDetects] = (int)centroid_x;
                    y_coords[countDetects] = (int)centroid_y;
                    countDetects++;
                } else {
                    printf("Maximum number of cells exceeded.\n");
                    break;
                }
            }
        }
    }

    printf("Cell detection completed.\n");
    printf("Number of cells detected: %d\n", countDetects);

    // Draw crosses on the original input image at the centroids of the detected cells
    DrawCrosses(input_image, x_coords, y_coords, countDetects);
    printf("Crosses drawn on image.\n");

    // Save the processed image
    write_bitmap(input_image, argv[2]);
    printf("Processed image saved to %s\n", argv[2]);

    printf("Done!\n");
    return 0;
}

// Function implementations

void invert(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]){
    for (int x = 0; x < BMP_WIDTH; x++)
    {
        for (int y = 0; y < BMP_HEIGTH; y++)
        {
            for (int c = 0; c < BMP_CHANNELS; c++)
            {
                output_image[x][y][c] = 255 - input_image[x][y][c];
            }
        }
    }
}

void rgb2gray(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char gray_image[BMP_WIDTH][BMP_HEIGTH]){
    for (int x = 0; x < BMP_WIDTH; x++)
    {
        for (int y = 0; y < BMP_HEIGTH; y++)
        {
            gray_image[x][y] = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3;
        }
    }
}

void Binarize(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], int T){
    if (T < 0 || T > 255){
        printf("Setting threshold to default (90)\n");
        T = 90;
    }
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

void Convert23D(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]){
    for (int x = 0; x < BMP_WIDTH; x++)
    {
        for (int y = 0; y < BMP_HEIGTH; y++)
        {
            for (int c = 0; c < BMP_CHANNELS; c++)
            {
                output_image[x][y][c] = input_image[x][y];
            }
        }
    }
}

void DrawCrosses(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x_coords[MAX_CELLS], int y_coords[MAX_CELLS], int cell_detected){
    for (int i = 0; i < cell_detected; i++){
        if ((x_coords[i] != 0) && (y_coords[i] != 0)) {
            for (int x = -7; x <= 7; x++){
                if (x_coords[i] + x >= 0 && x_coords[i] + x < BMP_WIDTH){
                    input_image[x_coords[i] + x][y_coords[i]][0] = 255; // Red
                    input_image[x_coords[i] + x][y_coords[i]][1] = 0;
                    input_image[x_coords[i] + x][y_coords[i]][2] = 0;
                }
            }
            for (int y = -7; y <= 7; y++){
                if (y_coords[i] + y >= 0 && y_coords[i] + y < BMP_HEIGTH){
                    input_image[x_coords[i]][y_coords[i] + y][0] = 255; // Red
                    input_image[x_coords[i]][y_coords[i] + y][1] = 0;
                    input_image[x_coords[i]][y_coords[i] + y][2] = 0;
                }
            }
        }
    }
}

int Otsu(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH]) {
    // Otsu's method implementation
    float hist[256] = {0};
    double sum = 0;
    float total = BMP_WIDTH * BMP_HEIGTH;

    for (int x = 0; x < BMP_WIDTH; x++)
    {
        for (int y = 0; y < BMP_HEIGTH; y++)
        {
            hist[input_image[x][y]]++; // Grey scale histogram
            sum += input_image[x][y];
        }
    }

    // Normalize histogram
    for (int i = 0; i < 256; i++)
    {
        hist[i] = hist[i] / total;
    }

    float q1 = 0.0;
    float mu1 = 0.0;
    float mu = sum / total;
    float max_between_variance = 0.0;
    int optimized_thresh = 0;

    for (int t = 0; t < 256; t++)
    {
        q1 += hist[t];
        if (q1 == 0)
            continue;
        if (q1 == 1)
            break;
        mu1 += t * hist[t];

        float mu2 = (mu - mu1) / (1 - q1);
        float between_variance = q1 * (1 - q1) * (mu1 / q1 - mu2) * (mu1 / q1 - mu2);

        if (between_variance > max_between_variance)
        {
            max_between_variance = between_variance;
            optimized_thresh = t;
        }
    }

    return optimized_thresh;
}

// Bottleneck Detection Algorithm Functions

void process_image(unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH]) {
    bool changes_made = true;
    int iteration_count = 0;
    int max_iterations = 10; // Set a reasonable limit

    bool visited[BMP_WIDTH][BMP_HEIGTH];

    while (changes_made) {
        changes_made = false;
        iteration_count++;
        if (iteration_count > max_iterations) {
            printf("Maximum iterations reached in process_image.\n");
            break;
        }
        printf("Iteration %d in process_image.\n", iteration_count);

        // Reset visited array
        memset(visited, false, sizeof(visited));

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
                    distance_profile.count = boundary.count;
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
    printf("Exiting process_image after %d iterations.\n", iteration_count);
}


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
    int n = distance_profile->count;

    // Use slope_differences->values as temporary storage for slopes
    double* slopes = slope_differences->values;

    // Calculate slopes
    for (int i = 0; i < n; i++) {
        int next = (i + 1) % n;
        slopes[i] = distance_profile->values[next] - distance_profile->values[i];
    }

    // Calculate slope differences
    for (int i = 0; i < n; i++) {
        int next = (i + 1) % n;
        slope_differences->values[i] = slopes[next] - slopes[i];
    }

    slope_differences->count = n;
}


void find_bottleneck_points(DoubleArray* slope_differences, Boundary* boundary, BottleneckPoint* bottleneck_points, int* bottleneck_count) {
    double threshold = 0.1; // Threshold for local maxima; adjust as needed
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
    // Bresenham's Line Algorithm
    int x0 = p1.x;
    int y0 = p1.y;
    int x1 = p2.x;
    int y1 = p2.y;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (true) {
        if (x0 >= 0 && x0 < BMP_WIDTH && y0 >= 0 && y0 < BMP_HEIGTH) {
            // Instead of setting to 0, set to a mid-gray value to distinguish from background
            bin_image[x0][y0] = 128;
        }

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}
