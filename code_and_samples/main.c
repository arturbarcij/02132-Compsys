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

//Function to convert to grey scale image (TEST)
void rbg2gray(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char gray_image[BMP_WIDTH][BMP_HEIGTH]){
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      int BoW = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2])/3;
      gray_image[x][y] = BoW;
    }
  }

}

//Function to convert the grey-scale image to a binary image (black and white) (TEST)
void gray2bin(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH][0]){
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      double BoW = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2])/3;

      if (BoW > 90){
        bin_image[x][y][0] = 255;
      }
      else {
        bin_image[x][y][0] = 0;
      }
      printf("%d\n", bin_image[x][y][0]);
    }
  }
}


  //Declaring the array to store the image (unsigned char = unsigned 8 bit)
  unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
  unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
  unsigned char gray_image[BMP_WIDTH][BMP_HEIGTH];
  unsigned char bin_image[BMP_WIDTH][BMP_HEIGTH];

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
  //rbg2gray(input_image,gray_image);
  gray2bin(input_image,bin_image);

  //Save image to file
  write_bitmap(bin_image, argv[2]);

  //Saves a grey scale image
  //write_bitmap(gray_image, argv[2]);


  printf("Done!\n");
  return 0;
}

//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp
//To run: .\main.out <input file path> <output file path>