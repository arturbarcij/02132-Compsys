#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//another method to run code with "&" between commands
//To run (win): .\main.exe .\samples\easy\1EASY.bmp .\samples\easy\1EASY_inv.bmp & .\main.exe .\samples\easy\2EASY.bmp .\samples\easy\2EASY_inv.bmp ...

int main() {
    char command[256];  // Buffer to hold the command string
    //TIME ANALYSIS
    clock_t start, end;

    printf("EASY TEST: \n");
    for (int i = 0; i < 10; i++) {
        
        double cpu_time_used;start = clock();
        // Construct the command with the current value of i
        sprintf(command, ".\\main_copy2.exe .\\samples\\easy\\%dEASY.bmp .\\samples\\easy\\%dEASY_inv.bmp", i, i);

        // Execute the command
        system(command);


        /* The code that has to be measured. */
        end = clock();
        
        int local_cpu_time_used;
        local_cpu_time_used = cpu_time_used;
        local_cpu_time_used = end - start;
        printf("Total time: %f ms\n", cpu_time_used * 1000.0 /CLOCKS_PER_SEC);
        printf("\n");
    }

    printf("MEDIUM TEST: \n");
    for (int i = 0; i < 10; i++) {
        
        double cpu_time_used;start = clock();
        // Construct the command with the current value of i
        sprintf(command, ".\\main_copy2.exe .\\samples\\medium\\%dMEDIUM.bmp .\\samples\\medium\\%dMEDIUM_inv.bmp", i, i);

        // Execute the command
        system(command);


        /* The code that has to be measured. */
        end = clock();
        
        int local_cpu_time_used;
        local_cpu_time_used = cpu_time_used;
        local_cpu_time_used = end - start;
        printf("Total time: %f ms\n", cpu_time_used * 1000.0 /CLOCKS_PER_SEC);
        printf("\n");
    }

    printf("HARD TEST: \n");
    for (int i = 0; i < 10; i++) {
        
        double cpu_time_used;start = clock();
        // Construct the command with the current value of i
        sprintf(command, ".\\main_copy2.exe .\\samples\\hard\\%dHARD.bmp .\\samples\\hard\\%dHARD_inv.bmp", i, i);

        // Execute the command
        system(command);


        /* The code that has to be measured. */
        end = clock();
        
        int local_cpu_time_used;
        local_cpu_time_used = cpu_time_used;
        local_cpu_time_used = end - start;
        printf("Total time: %f ms\n", cpu_time_used * 1000.0 /CLOCKS_PER_SEC);
        printf("\n");
    }

    return 0;
}
