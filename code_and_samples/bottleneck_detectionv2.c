#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define MAX_BOUNDARY_POINTS 10000
#define PI 3.14159265358979323846

typedef struct {
    int width;
    int height;
    unsigned char* data; // Binary image data (0 or 255)
} Image;

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point points[MAX_BOUNDARY_POINTS];
    int count;
} Boundary;

typedef struct {
    double* values;
    int count;
} DoubleArray;

typedef struct {
    Point point;
    double value;
} BottleneckPoint;

// Function prototypes
void find_boundary(Image* image, Boundary* boundary);
void calculate_centroid(Boundary* boundary, double* centroid_x, double* centroid_y);
void calculate_distance_profile(Boundary* boundary, double centroid_x, double centroid_y, DoubleArray* distance_profile);
void calculate_slope_differences(DoubleArray* distance_profile, DoubleArray* slope_differences);
void find_bottleneck_points(DoubleArray* slope_differences, Boundary* boundary, BottleneckPoint* bottleneck_points, int* bottleneck_count);
void calculate_curvature(Boundary* boundary, DoubleArray* curvature);
void validate_bottleneck_points(BottleneckPoint* bottleneck_points, int* bottleneck_count, DoubleArray* curvature, Boundary* boundary);
void find_minimum_distance_pair(BottleneckPoint* bottleneck_points, int bottleneck_count, Point* point1, Point* point2);
void draw_line(Image* image, Point p1, Point p2);
void process_image(Image* image);

// Main function
int main() {
    // Load your binary image into the Image struct
    Image image;
    // Initialize image.width, image.height, and image.data
    // For this example, we assume the image is already loaded

    // Process the image to separate overlapping cells
    process_image(&image);

    // Save or display the processed image
    // Implement image saving function as needed

    // Free allocated memory
    free(image.data);

    return 0;
}

/**
 * Finds the boundary of a binary object in the image.
 * This function uses a simple boundary tracing algorithm.
 */
void find_boundary(Image* image, Boundary* boundary) {
    int width = image->width;
    int height = image->height;
    unsigned char* data = image->data;

    bool found_start = false;
    int start_x = 0, start_y = 0;

    // Find a starting point (first foreground pixel)
    for (int y = 0; y < height && !found_start; y++) {
        for (int x = 0; x < width && !found_start; x++) {
            if (data[y * width + x] == 255) {
                start_x = x;
                start_y = y;
                found_start = true;
            }
        }
    }

    if (!found_start) {
        boundary->count = 0;
        return;
    }

    // Initialize boundary point list
    boundary->count = 0;

    int x = start_x;
    int y = start_y;
    int dir = 0; // Direction (0 to 3 for 4-connected)

    int dx[4] = {1, 0, -1, 0}; // Right, down, left, up
    int dy[4] = {0, 1, 0, -1};

    do {
        // Add point to boundary
        if (boundary->count < MAX_BOUNDARY_POINTS) {
            boundary->points[boundary->count].x = x;
            boundary->points[boundary->count].y = y;
            boundary->count++;
        } else {
            printf("Boundary points exceed maximum limit.\n");
            break;
        }

        // Find next boundary point
        bool found_next = false;
        for (int i = 0; i < 4; i++) {
            int new_dir = (dir + i) % 4;
            int nx = x + dx[new_dir];
            int ny = y + dy[new_dir];

            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                if (data[ny * width + nx] == 255) {
                    x = nx;
                    y = ny;
                    dir = (new_dir + 3) % 4; // Turn left
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

/**
 * Calculates the centroid (center of mass) of the boundary points.
 */
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

/**
 * Calculates the distance from the centroid to each boundary point.
 * This forms the 1D distance profile.
 */
void calculate_distance_profile(Boundary* boundary, double centroid_x, double centroid_y, DoubleArray* distance_profile) {
    distance_profile->count = boundary->count;
    distance_profile->values = (double*)malloc(sizeof(double) * boundary->count);

    for (int i = 0; i < boundary->count; i++) {
        int x = boundary->points[i].x;
        int y = boundary->points[i].y;
        double dx = x - centroid_x;
        double dy = y - centroid_y;
        distance_profile->values[i] = sqrt(dx * dx + dy * dy);
    }
}

/**
 * Calculates the slope differences along the distance profile.
 * This helps in identifying bottleneck points.
 */
void calculate_slope_differences(DoubleArray* distance_profile, DoubleArray* slope_differences) {
    // First, calculate the slopes
    int n = distance_profile->count;
    double* slopes = (double*)malloc(sizeof(double) * n);

    for (int i = 0; i < n; i++) {
        int next = (i + 1) % n;
        slopes[i] = distance_profile->values[next] - distance_profile->values[i];
    }

    // Now, calculate the slope differences
    slope_differences->count = n;
    slope_differences->values = (double*)malloc(sizeof(double) * n);

    for (int i = 0; i < n; i++) {
        int next = (i + 1) % n;
        slope_differences->values[i] = slopes[next] - slopes[i];
    }

    free(slopes);
}

/**
 * Finds local maxima in the slope differences, which are potential bottleneck points.
 */
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
        }
    }
    *bottleneck_count = count;
}

/**
 * Calculates the curvature at each boundary point to identify concave regions.
 */
void calculate_curvature(Boundary* boundary, DoubleArray* curvature) {
    int n = boundary->count;
    curvature->count = n;
    curvature->values = (double*)malloc(sizeof(double) * n);

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

/**
 * Validates bottleneck points by ensuring they are located within concave regions.
 */
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

/**
 * Finds the pair of bottleneck points with the minimum Euclidean distance.
 */
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

/**
 * Draws a line between two points on the image to separate the cells.
 * This uses Bresenham's line drawing algorithm.
 */
void draw_line(Image* image, Point p1, Point p2) {
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
        if (x0 >= 0 && x0 < image->width && y0 >= 0 && y0 < image->height) {
            image->data[y0 * image->width + x0] = 0;
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

/**
 * Processes the image to separate overlapping cells using the bottleneck detection algorithm.
 */
void process_image(Image* image) {
    bool finished = false;

    while (!finished) {
        // Find the boundary of the clump
        Boundary boundary;
        find_boundary(image, &boundary);

        if (boundary.count == 0) {
            finished = true;
            break;
        }

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
        BottleneckPoint bottleneck_points[MAX_BOUNDARY_POINTS];
        int bottleneck_count = 0;
        find_bottleneck_points(&slope_differences, &boundary, bottleneck_points, &bottleneck_count);

        // Calculate curvature
        DoubleArray curvature;
        calculate_curvature(&boundary, &curvature);

        // Validate bottleneck points
        validate_bottleneck_points(bottleneck_points, &bottleneck_count, &curvature, &boundary);

        if (bottleneck_count < 2) {
            // No bottleneck points found; segmentation finished
            finished = true;
        } else {
            // Find minimum distance pair
            Point p1, p2;
            find_minimum_distance_pair(bottleneck_points, bottleneck_count, &p1, &p2);

            // Draw line between bottleneck points to separate cells
            draw_line(image, p1, p2);
        }

        // Free allocated memory
        free(distance_profile.values);
        free(slope_differences.values);
        free(curvature.values);
    }
}
