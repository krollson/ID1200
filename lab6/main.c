#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>

int clear_file(char *file) {
    int fd = open(file, O_RDWR);
    if (fd == -1) {
        perror("open clearing");
        return -1;
    }

    if (ftruncate(fd, 0) == -1) {
        perror("ftruncate clearing");
        close(fd);
        return -1;
    }

    size_t new_size = 2 * 1024 * 1024; // 2 MB
    if (ftruncate(fd, new_size) == -1) {
        perror("ftruncate resizing");
        close(fd);
        return -1;
    }

    char *map_ptr = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_ptr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }

    for (size_t i = 0; i < new_size; ++i) {
        map_ptr[i] = 0;
    }

    if (munmap(map_ptr, new_size) == -1) {
        perror("munmap");
        close(fd);
        return -1;
    }

    close(fd);

    printf("File cleared and resized to 2 MB successfully.\n");
    return 1;
}

int main(int argc, char *argv[])
{
    char *mmaped_ptr;
    int fd;
    struct stat sb;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
    {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int cleared_file = clear_file(argv[1]);
    if(cleared_file == -1)
    {
        exit(EXIT_FAILURE);
    }
    
    fd = open(argv[1], O_RDWR);
    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (fstat(fd, &sb) == -1)
    {
        perror("fstat");
        exit(EXIT_FAILURE);
    }

    int pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // Child
        mmaped_ptr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        // printf("Child process, pid = %d; mmap address %p\n", pid, mmaped_ptr);
        char* write = "01234";
        strncpy(&mmaped_ptr[0], write, 5);
        char read[6];
        strncpy(read, &mmaped_ptr[4096], 5);
        read[5] = '\0';
        printf("Child process, pid = %d; read from mmaped[4096] %s\n", pid, read);
    }
    else
    {
        // Parent process
        mmaped_ptr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        // printf("Parent process, pid = %d; mmap address %p\n", pid, mmaped_ptr);
        char* write = "56789";
        strncpy(&mmaped_ptr[4096], write, 5);
        char read[6];
        strncpy(read, mmaped_ptr, 5);
        read[5] = '\0';
        printf("Parent process, pid = %d; read from mmaped[0] %s\n", pid, read);
    }

    // Wait for child process to finish
    wait(NULL);

    close(fd);

    exit(EXIT_SUCCESS);
}