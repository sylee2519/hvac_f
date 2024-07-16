#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <mpi.h>

#define FILE_COUNT 1000  // Increased for distribution

// ... [rest of the functions remain the same as in the previous example] ...

void generate_files(uint32_t filecount, int rank) {
    char tfn[256];
    char buffer[4096];
    int testfile = 0;
    srand(time(NULL) + rank);  // Unique seed for each process
    
    for (int lcv = 0; lcv < filecount; lcv++) {
        snprintf(tfn, sizeof(tfn), "./testfile.%d.4096.%d", rank, lcv);
        if ((testfile = open(tfn, O_WRONLY | O_CREAT | O_TRUNC,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
            perror("Cannot open output file\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        sprintf(buffer, "This is file %d from process %d and this was a successfully redirected read\n", lcv, rank);
        write(testfile, (void *)buffer, 4096);
        close(testfile);
    }
}

void cleanup_files(uint32_t filecount, int rank) {
    char tfn[256];
    for (int lcv = 0; lcv < filecount; lcv++) {
        snprintf(tfn, sizeof(tfn), "./testfile.%d.4096.%d", rank, lcv);
        unlink(tfn);
    }
}

void test_file_read(uint32_t iterations, int rank, int size) {
    char tfn[256];
    char buffer[4096];
    int testfile = 0;
    srand(time(NULL) + rank);

    for (int lcv = 0; lcv < iterations; lcv++) {
        uint32_t id = rand() % FILE_COUNT;
        int file_owner = id % size;
        snprintf(tfn, sizeof(tfn), "./testfile.%d.4096.%d", file_owner, id / size);
        
        if (file_owner == rank) {
            if ((testfile = open(tfn, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
                fprintf(stderr, "Cannot open output file %s\n", tfn);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            read(testfile, buffer, 4096);
            fprintf(stderr, "Process %d: Contents of file %s:\n%s\n", rank, tfn, buffer);
            close(testfile);
        }
    }
}

int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int files_per_process = FILE_COUNT / size;
    
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    
    printf("Process %d of %d running on %s\n", rank, size, hostname);

    generate_files(files_per_process, rank);
    MPI_Barrier(MPI_COMM_WORLD);
    
    test_file_read(files_per_process * 2, rank, size);
    
    MPI_Barrier(MPI_COMM_WORLD);
    cleanup_files(files_per_process, rank);

    if (rank == 0) {
        printf("Ending test program\n");
    }

    MPI_Finalize();
    return 0;
}
