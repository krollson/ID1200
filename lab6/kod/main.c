#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int clear_file(char *file)
{
    int fd = open(file, O_RDWR);
    if (fd == -1)
    {
        perror("open clearing");
        return -1;
    }

    if (ftruncate(fd, 0) == -1)
    {
        perror("ftruncate clearing");
        close(fd);
        return -1;
    }

    size_t new_size = 2 * 1024 * 1024; // 2 MB
    if (ftruncate(fd, new_size) == -1)
    {
        perror("ftruncate resizing");
        close(fd);
        return -1;
    }

    char *map_ptr = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_ptr == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return -1;
    }

    close(fd);

    printf("File cleared\n");
    return 1;
}

int main(int argc, char *argv[])
{
    char *mmaped_ptr;
    int fd;
    struct stat sb;
    pid_t pid, fv;
    sem_t *sem_parent;
    sem_t *sem_child;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
    {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int cleared_file = clear_file(argv[1]); // jag vet inte om man ska göra det här, 
                                            // men har med det för att få det att kännas som en ny fil varje gång
    if (cleared_file == -1)
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

    sem_parent = sem_open("/parent_sem", O_CREAT | O_EXCL, 0600, 0);
    if (sem_parent == SEM_FAILED)
    {
        perror("parent sem open\n");
        exit(EXIT_FAILURE);
    }
    sem_child = sem_open("/child_sem", O_CREAT | O_EXCL, 0600, 0);
    if (sem_child == SEM_FAILED)
    {
        perror("child sem open\n");
        exit(EXIT_FAILURE);
    }
    fv = fork();
    if (fv < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (fv == 0)
    {
        // Child
        mmaped_ptr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        pid = getpid();
        // printf("Child process, pid = %d; mmap address %p\n", pid, mmaped_ptr); // print q1
        char *write = "01234";
        strncpy(&mmaped_ptr[0], write, 5);
        int mr = msync(mmaped_ptr, sb.st_size, MS_SYNC);
        if (mr == -1)
        {
            perror("msync child\n");
            exit(EXIT_FAILURE);
        }
        sem_post(sem_child);
        sem_wait(sem_parent);
        char read[6];
        strncpy(read, &mmaped_ptr[4096], 5);
        read[5] = '\0';
        printf("Child process, pid = %d; read from mmaped[4096] %s\n", pid, read); // print q2, 3
    }
    else
    {
        // Parent process
        mmaped_ptr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        pid = getpid();
        // printf("Parent process, pid = %d; mmap address %p\n", pid, mmaped_ptr); // print q1
        char *write = "56789";
        strncpy(&mmaped_ptr[4096], write, 5);
        int mr = msync(mmaped_ptr, sb.st_size, MS_SYNC);
        if (mr == -1)
        {
            perror("msync parent\n");
            exit(EXIT_FAILURE);
        }
        sem_post(sem_parent);
        sem_wait(sem_child);
        char read[6];
        strncpy(read, mmaped_ptr, 5);
        read[5] = '\0';
        printf("Parent process, pid = %d; read from mmaped[0] %s\n", pid, read); // // print q2, 3
    }

    wait(NULL);
    sem_close(sem_child);
    sem_unlink("/child_sem");
    sem_close(sem_parent);
    sem_unlink("/parent_sem");
    munmap(mmaped_ptr, sb.st_size);
    close(fd);

    exit(EXIT_SUCCESS);
}