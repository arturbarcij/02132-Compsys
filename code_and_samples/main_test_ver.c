//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.h"

//Function to invert pixels of an image (negative)
void makeWhiteNBlack(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH]){
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      output_image[x][y]=0;
      if(((input_image[x][y][0]+input_image[x][y][1]+input_image[x][y][2])/3)>90)
      {
        output_image[x][y]=255;
      }
    }
  }
}

void make3d(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image2[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]){
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      for(int c=0; c<BMP_CHANNELS;c++)
      output_image2[x][y][c] = output_image[x][y];
    }
  }

// hvis den er død har den værdi 0
// hvis den er levende har den værdi 255 basically > 0



void erode(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH]) {
    int arr_x[950];
    int arr_y[950];
    int i=0;
    for (int x = 0; x < BMP_WIDTH; x++) {
    
      for (int y = 0; y < BMP_HEIGTH; y++) {
      
        if(output_image[x][y] == 255){
          if(output_image[x][y + 1] == 0 || output_image[x][y - 1] == 0
          || output_image[x + 1][y] == 0 || output_image[x - 1][y] == 0) {
            arr_x[i]=x;
            arr_y[i]=y;
            i++;
          }
        } 
    }
  }
}
//sættes en kø(datastruktur tuple) så alle naboerne dør ikke med det samme
//et tuple med levende cell koordinater, som dør(eroded)
}


  //Declaring the array to store the image (unsigned char = unsigned 8 bit)
  unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
  unsigned char output_image[BMP_WIDTH][BMP_HEIGTH];
  unsigned char output_image2[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

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
  makeWhiteNBlack(input_image,output_image);

  make3d(output_image,output_image2);

  //Save image to file
  write_bitmap(output_image2, argv[2]);

  printf("Done!\n");
  return 0;
}
