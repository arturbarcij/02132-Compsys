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
}
// hvis den er død har den værdi 0
// hvis den er levende har den værdi 255 basically > 0



void erode(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH]) {
    int arr_x[100000];
    int arr_y[100000];
    int i = 0;
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            if (output_image[x][y] == 255) {
                // Check for out of bonds
                if ((x > 0 && output_image[x - 1][y] == 0) || (x==0||y==0||x== BMP_WIDTH-1||y==BMP_HEIGTH-1) || 
                    (x < BMP_WIDTH - 1 && output_image[x + 1][y] == 0) ||
                    (y > 0 && output_image[x][y - 1] == 0) ||
                    (y < BMP_HEIGTH - 1 && output_image[x][y + 1] == 0)) 
                {
                    arr_x[i] = x;
                    arr_y[i] = y;
                    i++;
                    // tjek boundary på array
                    if (i >= 100000) {
                        fprintf(stderr, "test boundary of array");
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

int cellDetect(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH]){
  int countDetects=0;
  int t=0;
  for (int x = 0; x < BMP_WIDTH-14; x++) 
  {
    for (int y = 0; y < BMP_HEIGTH-14; y++) 
    {
      for(int z= 0; z < 14; z++)
      {
        
        if((output_image[x+z][y]==255)||(output_image[x][y+z]==255)||(output_image[x+14][y+z]==255)||(output_image[x+z][y+14]==255))
        {
        }
        else
        {
        t=0;
          for(int p=0;p<12;p++)
          {
            for(int p=0;p<12;p++)
            {
              if(output_image[x+p+1][y+z+1]==255)
              {
                if(t==0)
                {
                  countDetects++;
                  t++;
                }
                output_image[x+p+1][y+z+1]=0;
                
                break;
              }
            }
          }
        }
      }
    }
  }        
  return countDetects;
}
//sættes en kø(datastruktur tuple) så alle naboerne dør ikke med det samme
//et tuple med levende cell koordinater, som dør(eroded)

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
  
  int detects=0;
  for(int c=0; c<12; c++)
  {
    erode(output_image);
    detects+= cellDetect(output_image);
  }

  printf("%d",detects);
  
  make3d(output_image,output_image2);

  write_bitmap(output_image2, argv[2]);

  //Save image to file
  

  printf("Done!\n");

  return 0;
}
