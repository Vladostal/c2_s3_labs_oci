#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/wait.h"

#define BUF_SIZE 256

int create_process() {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(-1);
    }
    return pid;
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

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t pid = create_process();
    if (pid == 0) {
        if (dup2(file_fd, STDIN_FILENO) == -1 || dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(-1);
        }
        close(file_fd);
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        execl("./child", "child", NULL);
        perror("execl");
        exit(-1);
    }

    close(file_fd);
    close(pipe_fd[1]);

    char buf[BUF_SIZE];
    ssize_t count;
    while ((count = read(pipe_fd[0], buf, sizeof(buf))) > 0) {
        if (write(STDOUT_FILENO, buf, count) == -1) {
            perror("write");
            return -1;
        }
    }
    if (count == -1) {
        perror("read");
        return -1;
    }
    close(pipe_fd[0]);

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return -1;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "parent: child process failed, exit\n");
        return -1;
    }
    return 0;
}
