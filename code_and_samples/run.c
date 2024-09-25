#include <stdio.h>
#include <stdlib.h>

int main() {
    char command[256];  // Buffer to hold the command string
    for (int i = 0; i < 11; i++) {
        // Construct the command with the current value of i
        sprintf(command, ".\\main.exe .\\samples\\easy\\%dEASY.bmp .\\samples\\easy\\%dEASY_inv.bmp", i, i);

        // Execute the command
        system(command);
    }
    // for (int i = 0; i < 11; i++) {
    //     // Construct the command with the current value of i
    //     sprintf(command, ".\\main.exe .\\samples\\medium\\%dMEDIUM.bmp .\\samples\\medium\\%dMEDIUM_inv.bmp", i, i);

    //     // Execute the command
    //     system(command);
    // }
    return 0;
}
