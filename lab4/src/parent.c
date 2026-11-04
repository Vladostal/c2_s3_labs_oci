#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define RESULT_SIZE 64

int create_process() {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(-1);
    }
    return pid;
}

int create_shared_file(off_t size) {
    char name[] = "/tmp/lab4_XXXXXX";
    int fd = mkstemp(name);
    if (fd == -1) {
        perror("mkstemp");
        return -1;
    }
    if (unlink(name) == -1) {
        perror("unlink");
        close(fd);
        return -1;
    }
    if (ftruncate(fd, size) == -1) {
        perror("ftruncate");
        close(fd);
        return -1;
    }
    return fd;
}

int print_results(int fd, off_t size) {
    char *data = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    int result = 0;
    if (write(STDOUT_FILENO, data, strnlen(data, size)) == -1) {
        perror("write");
        result = -1;
    }
    if (munmap(data, size) == -1) {
        perror("munmap");
        result = -1;
    }
    return result;
}

int main() {
    char *file_name = NULL;
    size_t size = 0;
    if (getline(&file_name, &size, stdin) == -1) {
        fprintf(stderr, "file name is expected\n");
        free(file_name);
        return -1;
    }
    file_name[strcspn(file_name, "\n")] = '\0';

    int file_fd = open(file_name, O_RDONLY);
    if (file_fd == -1) {
        perror("open");
        free(file_name);
        return -1;
    }
    free(file_name);

    struct stat info;
    if (fstat(file_fd, &info) == -1) {
        perror("fstat");
        close(file_fd);
        return -1;
    }
    off_t shared_size = (info.st_size / 2 + 1) * RESULT_SIZE + 1;
    int shared_fd = create_shared_file(shared_size);
    if (shared_fd == -1) {
        close(file_fd);
        return -1;
    }

    pid_t pid = create_process();
    if (pid == 0) {
        if (dup2(file_fd, STDIN_FILENO) == -1 || dup2(shared_fd, STDOUT_FILENO) == -1) {
            perror("dup2");
            _exit(-1);
        }
        close(file_fd);
        close(shared_fd);
        execl(CHILD_PATH, "lab4_child", NULL);
        perror("execl");
        _exit(-1);
    }
    close(file_fd);

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        close(shared_fd);
        return -1;
    }

    int result = print_results(shared_fd, shared_size);
    if (close(shared_fd) == -1) {
        perror("close");
        result = -1;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "parent: child process failed, exit\n");
        return -1;
    }
    return result;
}
