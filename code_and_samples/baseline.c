
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "cbmp.h"
#include <math.h>
#include <time.h>

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

//Converts the 3D gray scale to true 2D gray scale
void rgb2gray(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char gray_image[BMP_WIDTH][BMP_HEIGTH]){
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGTH; y++)
    {
      gray_image[x][y] = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2])*0.3333;
    }
  }
}

//Binarizes the image with threshold = 90
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

//Converts the 2D image to 3D image, used for writing the image
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

//Erosion function, uses a 3x3 structuring element with vertical and horizontal lines
void erode(unsigned char output_image[BMP_WIDTH][BMP_HEIGTH]) {
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

//Detects cells in the image, returns the number of cells detected
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

//Draws crosses on the detected cells with a 3-pixel width
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

  //Convert to gray-scale
  rgb2gray(input_image, output_2d);

  //Binarize the image
  Binarize(output_2d, output_2d);

//Cell erosion and detection, exits when no new cells are detected after 10 erosions.
  while(1){
    erode(output_2d);
    Convert23D(output_2d, output_3d);
    write_bitmap(output_3d, argv[2]);
    countDetects = Detection(output_2d, x_coords, y_coords, countDetects);
    detects_old = countDetects;
    count++;
    if (detects_old == countDetects && count > 10){
      break;
    }

  }

  //Prints the number of cells detected, draws crosses on the detected cells and writes the image to file.
  printf("Number of cells detected: %d\n", countDetects);
  Convert23D(output_2d, output_3d);
  DrawCrosses(input_image, x_coords, y_coords, countDetects);
  write_bitmap(input_image, argv[2]);

  printf("Done!\n");
  return 0;
}
