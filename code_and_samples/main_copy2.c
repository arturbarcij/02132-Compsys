//To compile (linux/mac): gcc code_and_samples/cbmp.c code_and_samples/main.c -o main.out -std=c99
//To run (linux/mac): ./main.out code_and_samples/example.bmp code_and_samples/example_inv.bmp
//To run: .\main.out <input file path> <output file path>

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "cbmp.h"
#include <math.h>
#include <time.h>

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
  if (T < 0 || T > 255){
    printf("Setting threshold to default (90)");
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

void erode(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH]) {
    int arr_x[100000];
    int arr_y[100000];
    int i = 0;
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            if (output_image[x][y] == 255) {
                // Check for out of bonds hallojsa
                // Cross-erosion
                // 0 1 0
                // 1 1 1
                // 0 1 0
                if ((x > 0 && output_image[x - 1][y] == 0) ||
                    (x < BMP_WIDTH - 1 && output_image[x + 1][y] == 0) ||
                    (y > 0 && output_image[x][y - 1] == 0) ||
                    (y < BMP_HEIGTH - 1 && output_image[x][y + 1] == 0))

                // Diagonal-erosion
                // 1 0 1
                // 0 1 0
                // 1 0 1
                // if (x > 0 && output_image[x - 1][y - 1] == 0 ||
                //     x < BMP_WIDTH - 1 && output_image[x + 1][y + 1] == 0 ||
                //     y >  0 && output_image[x - 1][y + 1] == 0 ||
                //     y < BMP_HEIGTH - 1 && output_image[x + 1][y - 1] == 0) 

                //Diamond erosion (5x5 erosion zone)
                // 0 0 1 0 0 
                // 0 1 1 1 0
                // 1 1 1 1 1
                // 0 1 1 1 0
                // 0 0 1 0 0
        
                // if (x > 2 && output_image[x - 2][y] == 0 ||
                //     x > 1 && output_image[x - 1][y] == 0 ||
                //     x < BMP_WIDTH - 1 && output_image[x + 1][y] == 0 ||
                //     x < BMP_WIDTH - 2 && output_image[x + 2][y] == 0 ||
                //     y > 2 && output_image[x][y - 2] == 0 ||
                //     y > 1 && output_image[x][y - 1] == 0 ||
                //     y < BMP_HEIGTH - 1 && output_image[x][y + 1] == 0 ||
                //     y < BMP_HEIGTH - 2 && output_image[x][y + 2] == 0 ||
                //     x > 1 && y > 1 && output_image[x - 1][y - 1] == 0 ||
                //     x < BMP_WIDTH - 1 && y < BMP_HEIGTH - 1 && output_image[x + 1][y + 1] == 0 ||
                //     x > 1 && y < BMP_HEIGTH - 1 && output_image[x - 1][y + 1] == 0 ||
                //     x < BMP_WIDTH - 1 && y > 1 && output_image[x + 1][y - 1] == 0)

                // Box
                // 1 1 1
                // 1 1 1
                // 1 1 1
                // if ((x > 0 && output_image[x - 1][y] == 0) ||
                //     (x < BMP_WIDTH - 1 && output_image[x + 1][y] == 0) ||
                //     (y > 0 && output_image[x][y - 1] == 0) ||
                //     (y < BMP_HEIGTH - 1 && output_image[x][y + 1] == 0) ||
                //     (x > 0 && output_image[x - 1][y - 1] == 0 ||
                //     (x < BMP_WIDTH - 1 && output_image[x + 1][y + 1] == 0) ||
                //     (y >  0 && output_image[x - 1][y + 1] == 0) ||
                //     (y < BMP_HEIGTH - 1 && output_image[x + 1][y - 1]) == 0))

                // Line
                // 1 1 1
                // if ((x > 0 && output_image[x - 1][y] == 0) ||
                //     (x < BMP_WIDTH - 1 && output_image[x + 1][y] == 0))

                // Diagonal line
                // 1 0 0
                // 0 1 0
                // 0 0 1
                // if ((x > 0 && y < BMP_HEIGTH - 1 && output_image[x - 1][y + 1] == 0) ||
                //     (x < BMP_WIDTH - 1 && y > 0 && output_image[x + 1][y - 1] == 0))
                {
                    arr_x[i] = x;
                    arr_y[i] = y;
                    i++;
                    // tjek boundary på array
                    if (i >= 100000) {
                        fprintf(stderr, "test boundaries");
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

int Detection(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], int x_coords[500], int y_coords[500], int cell_detected){
  int detect_flag = 0;

  for (int x = 6; x < BMP_WIDTH - 6; x++){
    for (int y = 6; y < BMP_HEIGTH - 6; y++){ //Looping through the eroded image

    detect_flag = 0;


    if (input_image[x][y] == 255){ //If a white pixel is detected
      for (int i = 0; i < 8; i++){
          if ((input_image[x + 7][y + i] == 255 || input_image[x - 7][y - i] == 255 || input_image[x - 7][y + i] == 255 || input_image[x + 7][y - i] == 255
          ||input_image[x + i][y + 7] == 255 || input_image[x - i][y - 7] == 255 || input_image[x - i][y + 7] == 255 || input_image[x + i][y - 7] == 255 )) {
            break;
          }

          else {
            if (i == 7 && !detect_flag){
              detect_flag = 1;
              cell_detected += 1;
              x_coords[cell_detected] = x;
              y_coords[cell_detected] = y;

              for (int p = 0; p < 7; p++){
                for (int q = 0; q < 7; q++){
                    if ((x + p > BMP_WIDTH) || (y + q > BMP_HEIGTH) || (x - p < 0) || (y - q < 0)){
                      fprintf(stderr,"Out of bounds\n");
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


void DrawCrosses(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], int x_coords[500], int y_coords[500], int cell_detected){
  for (int i = 0; i < cell_detected; i++){
    if ((x_coords[i] != 0) && (y_coords[i] != 0)) {
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
}

int Otsu(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH]) {
// Maximaise the variance between the two classes (forebground (cells) and background (non-cells))
//resulting in dynamically finding a threshold for converting the image to binary
//Requires there to be a clear distinction bewtween the classes or two distinct modums in the histogram otherwise it will not work properly
//The threshold is the value that maximises the variance between the two classes

  float hist[256] = {0};
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
	double   sum=0;//sum of all histogram values to calculate the mean grey level value of the imagem values before threshholding
	float optimizedthresh=0;//optimized threshhold,   



  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      hist[input_image[x][y]]++; //Grey scale histogram
      sum += input_image[x][y];
    }
  }

  //Compute the cumulative distribution function of the grey scale histogram
  for (int i = 0; i < 256; i++)
  {
    hist[i] = hist[i]/(BMP_WIDTH*BMP_HEIGTH);
  }
  q1 = hist[0];
  mu1 = 0;
  mu2 = 0;

  mu = sum/(BMP_WIDTH*BMP_HEIGTH);

  q1prev = q1;

  //Compute the between class variance for the treshhold range of 0 to 255
  for (int t = 0; t < 256; t++) 
  {
    q1next = q1prev+hist[t+1];
    mu1next = (q1prev*mu1+(t+1)*(hist[t+1]))/q1next;
    mu2next = (mu-q1next*mu1next)/(1-q1next);
    betweenvariance = q1prev*(1-q1prev)*((mu1-mu2)*(mu1-mu2));

    if (betweenvariance > maxbetweenvariance) {
      maxbetweenvariance = betweenvariance;
      optimizedthresh = t;

    }
    q1prev = q1next;
    mu1 = mu1next;
    mu2 = mu2next;
    
    if (q1next == 0) {
      mu1 = 0;
    }
  }
  return optimizedthresh;
}

void ApplyPrewittFilter(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH]) {
    int gx, gy;
    float sum;

    int prewitt_V[3][3] = {
    {-1, 0, 1},
    {-1, 0, 1},
    {-1, 0, 1}
};

int prewitt_H[3][3] = {
    {-1, -1, -1},
    {0, 0, 0},
    {1, 1, 1}
};

    for (int x = 1; x < BMP_WIDTH - 1; x++) {
        for (int y = 1; y < BMP_HEIGTH - 1; y++) {
            gx = 0;
            gy = 0;

            // Apply the Prewitt kernels
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    gx += input_image[x + i][y + j] * prewitt_V[i + 1][j + 1];
                    gy += input_image[x + i][y + j] * prewitt_H[i + 1][j + 1];
                }
            }

            // Calculate the gradient magnitude
            sum = pow((pow(abs(gx),2) + pow(abs(gy),2)),0.5);

            // Clamp the result to the range [0, 255]
            if (sum > 255) {
                sum = 255;
            } else if (sum < 0) {
                sum = 0;
            }

            output_image[x][y] = (unsigned char)sum;
        }
    }
}



//Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_2d[BMP_WIDTH][BMP_HEIGTH];
unsigned char output_3d[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

int x_coords[500];
int y_coords[500];
int countDetects = 0;
int T;
int count = 0;
int detects_old = 0;
int detecs_old_old = 0;


//Removed for memory purposes
// unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
// unsigned char gray_image[BMP_WIDTH][BMP_HEIGTH];
// unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH];
// unsigned char eroded_image[BMP_WIDTH][BMP_HEIGTH];
// unsigned char detect_image[BMP_WIDTH][BMP_HEIGTH];


//int cell_detected = 0;



//Main function
int main(int argc, char** argv)
{
  //argc counts how may arguments are passed
  //argv[0] is a string with the name of the program
  //argv[1] is the first command line argument (input image)
  //argv[2] is the second command line argument (output image)

  //Checking that 2 arguments are passed
  if (argc != 3)
  {
      fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
      exit(1);
  }


  

  //Load image from file
  read_bitmap(argv[1], input_image);

  //Run inversion

  rgb2gray(input_image, output_2d);

  //printf("Optimized threshold: %d\n",Otsu(output_2d));
  T = Otsu(output_2d);
  Binarize(output_2d, output_2d, T);

  while(1){
    erode(output_2d);
    countDetects = Detection(output_2d, x_coords, y_coords, countDetects);
    detects_old = countDetects;
    count++;
    if (detects_old == countDetects && count > 10){
      break;
    }

  }
  // for (int i = 0; i < 18; i++){
  //   // sleep(2);
  //   erode(output_2d);
  //   // Convert23D(output_2d, output_3d);
  //   //DrawCrosses(input_image, x_coords, y_coords, countDetects);
  //   // write_bitmap(output_3d, argv[2]);

  //   countDetects = Detection(output_2d, x_coords, y_coords, countDetects);
  // }

  printf("Number of cells detected: %d\n", countDetects);
  Convert23D(output_2d, output_3d);
  DrawCrosses(input_image, x_coords, y_coords, countDetects);
  write_bitmap(input_image, argv[2]);

  printf("Done!\n");
  return 0;
}

//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp
//To run: .\main.out <input file path> <output file path>
