#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char *mmaped_ptr;
    int fd;
    struct stat sb;
    sem_t *sem_parent, *sem_child;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
    {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
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