#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    //TIME ANALYSIS
    clock_t start, end;
        /* The code that has to be measured. */
        end = clock();
        
        local_cpu_time_used = end - start;
        printf("Total time: %f ms\n", cpu_time_used * 1000.0 /CLOCKS_PER_SEC);
    
    return 0;
}