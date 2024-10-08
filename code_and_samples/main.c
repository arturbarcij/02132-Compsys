//To compile (linux/mac): gcc code_and_samples/cbmp.c code_and_samples/main.c -o main.out -std=c99
//To run (linux/mac): ./main.out code_and_samples/example.bmp code_and_samples/example_inv.bmp
//To run: .\main.out <input file path> <output file path>

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99 -DTIME_ANALYSIS
//To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "cbmp.h"
#include <time.h>

// Macros for time analysis
#ifdef TIME_ANALYSIS
    #define START_TIMER start = clock();
    #define STOP_TIMER(operation) \
        end = clock(); \
        cpu_time_used = ((double) (end - start)) * 1000.0 / CLOCKS_PER_SEC; \
        //printf("Time for %s: %f ms\n", operation, cpu_time_used);
#else
    #define START_TIMER
    #define STOP_TIMER(operation)
#endif



//Function to invert pixels of an image (negative)
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
      gray_image[x][y] = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2])*0.3333;
    }
  }
}

void Binarize(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH], int T){
  //Gamma mapping
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      //Binarizing
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

void erode(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH]) {
    printf("%s\n", "eroding");
    int arr_x[100000];
    int arr_y[100000];
    int i = 0;
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            if (output_image[x][y] == 255) {
                // Check for out of bonds hallojsa
                if ((x > 0 && output_image[x - 1][y] == 0) ||
                    (x < BMP_WIDTH - 1 && output_image[x + 1][y] == 0) ||
                    (y > 0 && output_image[x][y - 1] == 0) ||
                    (y < BMP_HEIGTH - 1 && output_image[x][y + 1] == 0)) 
                {
                    arr_x[i] = x;
                    arr_y[i] = y;
                    i++;
                    // tjek boundary på array
                    if (i >= 100000) {
                        fprintf(stderr, "du har en lille array brorz");
                        return;
                    }
                }
            }
        }
    }
    
    for (int t = 0; t < i; t++) {
        output_image[arr_x[t]][arr_y[t]] = 0;
    }
}

int Detection(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], int cell_detected, int x_coords[500], int y_coords[500]){
  printf("%s\n", "Detecting");
  int exclusion_flag = 0;
  int detect_flag = 0;
  int OOB_flag = 0;

  for (int x = 6; x < BMP_WIDTH; x++){
    for (int y = 6; y < BMP_HEIGTH; y++){ //Looping through the eroded image
    detect_flag = 0;
    OOB_flag = 0;
    
    if (input_image[x][y] == 255){ //If a white pixel is detected

      for (int i = 0; i < 8; i++){
        int white = 255;
        int size = 7;
          if ((input_image[x + size][y + i] == white || input_image[x - size][y - i] == white || input_image[x - size][y + i] == white || input_image[x + size][y - i] == white
          ||input_image[x + i][y + size] == white || input_image[x - i][y - size] == white || input_image[x - i][y + size] == white || input_image[x + i][y - size] == white ) & !exclusion_flag) {
            exclusion_flag = 1;
            break;
          }
          else {
            if (i == 7 && !detect_flag){
              detect_flag = 1;
              cell_detected += 1;
              x_coords[cell_detected] = x;
              y_coords[cell_detected] = y;

              printf("%d", x_coords);

              // Making the detected cell black
              for (int p = 0; p < 7; p++){
                for (int q = 0; q < 7; q++){

                    if ((x + p > BMP_WIDTH || y + q > BMP_HEIGTH || x - p < 0 || y - q < 0)){
                      fprintf(stderr,"Out of bounds (Detection)\n");
                      //Set the pixel which is out of bounds to black, whenever we try to access a pixel outside the image
                      //OOB_flag = !OOB_flag;
                      break;
                    }
                  else {
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
  }
 return cell_detected;
}

void DrawCrosses(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x_coords[500], int y_coords[500], int cell_detected){
  printf("%s\n", "Drawing crosses");
  //Make the crosses larger than 1-pixel wide
  for (int i = 0; i < cell_detected; i++){
    for (int x = 0; x < 7; x++){
      for (int w = -1; w <= 1; w++){
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

int Otsu3(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH]) {
    int histogram[256] = {0};

    for(int x = 0; x < BMP_WIDTH; x++) {
        for(int y = 0; y < BMP_HEIGTH; y++) {
            histogram[input_image[x][y]]++;
        }
    }

    double probability[256], mean[256];
    double max_between, between[256];
    int threshold;

    /*
    probability = class probability
    mean = class mean
    between = between class variance
    */

    for(int i = 0; i < 256; i++) {
        probability[i] = 0.0;
        mean[i] = 0.0;
        between[i] = 0.0;
    }

    probability[0] = histogram[0];

    for(int i = 1; i < 256; i++) {
        probability[i] = probability[i - 1] + histogram[i];
        mean[i] = mean[i - 1] + i * histogram[i];
    }

    threshold = 0;
    max_between = 0.0;

    for(int i = 0; i < 255; i++) {
        if(probability[i] != 0.0 && probability[i] != 1.0)
            between[i] = pow(mean[255] * probability[i] - mean[i], 2) / (probability[i] * (1.0 - probability[i]));
    else
        between[i] = 0.0;
        if(between[i] > max_between) {
            max_between = between[i];
            threshold = i;
        }
    }
    return threshold;
}


void Otsu2(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], int T) {
  int N = BMP_WIDTH*BMP_HEIGTH;
  int threshold = 0;
  double var_max = 0.0;
  double var = 0.0;
  long int sum = 0;
  long int sumB = 0;
  double q1, q2 = 0.0;
  double mu1, mu2 = 0;
  int max_I = 255;
  long int hist[256];
  float prob[256];

  for (int i = 0; i < 256; i++){
    hist[i] = 0;
    sum += i * hist[i];
  }

  for (int x = 0; x < BMP_WIDTH; x++){
    for (int y = 0; y < BMP_HEIGTH; y++){
      hist[input_image[x][y]] += 1;
    }
  }

  for (int t = 0; t < 256; t++){
    q1 += hist[t];

    if (q1 == 0){
      continue;
    }
    q2 = N - q1;
    sumB += t * hist[t];
    mu1 = sumB/q1;

    mu2 = (sumB - sum)/q2;

    var = q1 * q2 * (mu1 - mu2) * (mu1 - mu2);
    if (var > var_max){
      threshold = t;
      var_max = var;
      printf("Var max: %f\n", var_max);
      printf("Threshold: %d\n", threshold);
    }
  }
}


void Otsu(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], int T) {
// Maximaise the variance between the two classes (forebground (cells) and background (non-cells))
//resulting in dynamically finding a threshold for converting the image to binary
//Requires there to be a clear distinction bewtween the classes or two distinct modums in the histogram otherwise it will not work properly
//The threshold is the value that maximises the variance between the two classes

  float hist[256] = {0};
  float CDF[256] = {0};
  float p1; //first value of the probability in Otsu's algorithm with t = 1
	float q1; //first value of q qith t = 1
	float q1prev;//previos value of q1
	float q1next;//next value of q1 , aka q1(t+1)
	float mu1; //average of first group, before threshold
	float mu2;//average of second group, after threshold
	float mu1next;//next value of the mean for group 1(mu1(t+1))
	float mu2next;//next value of the mean for group 2(mu2(t+1))
	float mu;//the mean gray level value of the entire image before thresholding
	float betweenvariance=0;//between group variance
	float maxbetweenvariance=0;//max between group variance
	float sum=0;//sum of all histogram values to calculate the mean grey level value of the imagem values before threshholding
	float optimizedthresh=0;//optimized threshhold,
  float mu_t;   



  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      hist[input_image[x][y]]++; //Grey scale histogram
      sum += input_image[x][y];
      printf("Sum: %f\n", sum);
    }
  }

  //Compute the cumulative distribution function of the grey scale histogram
  for (int i = 0; i < 256; i++)
  {
    CDF[i] = hist[i]/BMP_WIDTH*BMP_HEIGTH;
    printf("Probability i'th pixel value: %f\n", CDF[i]);
  }
  p1 = CDF[0];
  q1 = p1;
  mu1 = 0;
  mu2 = 0;

  mu = sum/(BMP_WIDTH*BMP_HEIGTH);

  q1prev = q1;

  //Compute the between class variance for the treshhold range of 0 to 255
  for (int t = 0; t < 256; t++) 
  {
    q1next = q1prev+CDF[t+1];
    mu1next = (q1prev*mu1+(t+1)*(CDF[t+1]))/q1next;
    mu2next = (mu-q1next*mu1next)/(1-q1next);
    betweenvariance = q1prev*(1-q1prev)*((mu1-mu2)*(mu1-mu2));
    //printf("Between variance: %f\n", betweenvariance);
    if (betweenvariance > maxbetweenvariance) {
      maxbetweenvariance = betweenvariance;
      optimizedthresh = t;
      printf("Optimized threshold: %f\n", optimizedthresh);

    }
    q1prev = q1next;
    printf("q1prev: %f\n", q1prev);
    mu1 = mu1next;
    mu2 = mu2next;
    
    if (q1next == 0) {
      printf("%s","q1next is 0");
      mu1 = 0;
    }
  }
}

//Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char gray_image[BMP_WIDTH][BMP_HEIGTH];
unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH];
unsigned char eroded_image[BMP_WIDTH][BMP_HEIGTH];
unsigned char detect_image[BMP_WIDTH][BMP_HEIGTH];
int cell_detected = 0;
int x_coords[500];
int y_coords[500];
int T;


//Main function
int main(int argc, char** argv)
{
    clock_t start, end;       // Declare the variables once at the beginning
    double cpu_time_used;
    double er = 0;
    double det = 0;
  //argc counts how may arguments are passed
  //argv[0] is a string with the name of the program
  //argv[1] is the first command line argument (input image)
  //argv[2] is the second command line argument (output image)

  int countDetects = 0;

  //Checking that 2 arguments are passed
  if (argc != 3)
  {
      fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
      exit(1);
  }

  printf("Example program - 02132 - A1\n");

  //Load image from file
  read_bitmap(argv[1], input_image);

  //Run inversion
  //invert(input_image,output_image);
  rgb2gray(input_image, gray_image);
  T = Otsu3(gray_image);
  printf("Threshold: %d\n", T);
  Binarize(gray_image, bin_image, T);

  for (int i = 0; i < 3; i++){
    
    sleep(1);
    // Start timing before erosion
    START_TIMER;
    erode(bin_image);
    // End timing after erosion
    er = STOP_TIMER("erosion");

    START_TIMER;
    countDetects += Detection(bin_image, cell_detected, x_coords, y_coords);
    det = STOP_TIMER("detection");

    printf("Number of cells detected: %d\n", countDetects);

    // Calculate the time difference in milliseconds
    //cpu_time_used = ((double) (end - start)) * 1000.0 / CLOCKS_PER_SEC;
    //printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
    //printf("Time analysis %d: %f ms\n", i + 1, cpu_time_used);
    //printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");

    Convert23D(bin_image, output_image);
    DrawCrosses(input_image, x_coords, y_coords, countDetects);

    // Write the output image after each iteration
    write_bitmap(input_image, argv[2]);
  }
        

  printf("Done!\n");
  printf("Execution time analysis:\n");
  printf("Time for erosion: %d, detection: %d ms\n", er, det);
  printf("Time for both: %d", (er + det));
  return 0;
}

//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp
//To run: .\main.out <input file path> <output file path>

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp

//To run execution time analysis add flag
//To run (win): gcc cbmp.c main.c -o main.exe -std=c99 -DTIME_ANALYSIS
