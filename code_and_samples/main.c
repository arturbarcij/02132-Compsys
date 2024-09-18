//To compile (linux/mac): gcc code_and_samples/cbmp.c code_and_samples/main.c -o main.out -std=c99
//To run (linux/mac): ./main.out code_and_samples/example.bmp code_and_samples/example_inv.bmp
//To run: .\main.out <input file path> <output file path>

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"

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

void Binarize(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH]){
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      if (input_image[x][y] > 90)
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

void erosion(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH], unsigned char eroded_image[BMP_WIDTH][BMP_HEIGTH]){
  unsigned char deleter[950][2];
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      if (input_image[x][y] == 255)
      {
        if (input_image[x-1][y] == 0 || input_image[x+1][y] == 0 || input_image[x][y-1] == 0 || input_image[x][y+1] == 0)
        {

        }
    }
  }
}
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
    printf("q1next: %f\n", q1next);
    mu1next = (q1prev*mu1+(t+1)*(CDF[t+1]))/q1next;
    printf("mu1next: %f\n", mu1next);
    mu2next = (mu-q1next*mu1next)/(1-q1next);
    printf("mu2next: %f\n", mu2next);
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

  printf("Example program - 02132 - A1\n");

  //Load image from file
  read_bitmap(argv[1], input_image);

  //Run inversion
  //invert(input_image,output_image);
  rgb2gray(input_image, gray_image);
  Binarize(gray_image, bin_image);

  //erosion(bin_image, eroded_image);

  Convert23D(bin_image, output_image);

  Otsu2(gray_image, 0);

  //Save image to file
  write_bitmap(output_image, argv[2]);

  printf("Done!\n");
  return 0;
}

//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp
//To run: .\main.out <input file path> <output file path>