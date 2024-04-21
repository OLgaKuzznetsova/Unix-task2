#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>

static int locks_counter = 0;
static char *filename_lck = NULL;

void interrupt_handler(int signum)
{
        FILE *fp = NULL;
        free(filename_lck);
        fp = fopen("result.txt", "a+");
        if (fp == NULL){
              fprintf(stderr, "error: failed to open result.txt.\n");  
              exit(EXIT_FAILURE);
        }
        fprintf(fp, "Process ID: %d. Number of successful locks: %d.\n", getpid(), locks_counter);
        if (fclose(fp) != 0){
              fprintf(stderr, "error: failed to close result.txt.\n");  
              exit(EXIT_FAILURE);  
        }
        exit(EXIT_SUCCESS);
}


int create_lck_file()
{
        int fd_lck_file = -1;
        char pid_in_str_format[10];

        while (fd_lck_file == -1) {
                fd_lck_file = open(filename_lck,
                               O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR);
        }
        sprintf(pid_in_str_format, "%d", getpid());
        int buf_size = strlen(pid_in_str_format) + 1;
        int written_data = write(fd_lck_file, pid_in_str_format, buf_size);

        if (written_data != buf_size) {
                fprintf(stderr, "error: failed to write bytes in .lck file.\n");
                close(fd_lck_file);
                return -1;
        }

        if (close(fd_lck_file) == -1){
                fprintf(stderr, "error: failed to close .lck file.\n");
                return -1;
        }
        return 0;
        
}

int remove_lck_file()
{
        pid_t pid = getpid();
        char pid_in_str_format[10];
        int fd_lck_file = open(filename_lck, O_RDONLY);
        if (fd_lck_file == -1)
        {
                fprintf(stderr, "error: failed to open .lck  file for reading.\n");
                return -1;
        }
        lseek(fd_lck_file, 0, SEEK_SET);
        int reading_bytes = read(fd_lck_file, pid_in_str_format, 10);
        if (reading_bytes == -1) {
                close(fd_lck_file);
                fprintf(stderr, "error: failed to read bytes from .lck file.\n");
                return -1;
        }

        int pid_in_lck_file = atoi(pid_in_str_format);

        if (pid_in_lck_file != pid) {
                close(fd_lck_file);
                fprintf(stderr, "error: process ids are different\n");
                return -1;
        }

        if (close(fd_lck_file) == -1) {
                fprintf(stderr, "error: failed to close file.\n");
                return -1;
        }

        if (unlink(filename_lck) == -1){
                fprintf(stderr, "error: unable to remove .lck file\n");
                return -1; 
        }
        return 0;

        
}

int main(int argc, char **argv)
{
        signal(SIGINT, interrupt_handler);
        if (argc == 1) {
                fprintf(stderr, "error: file name missing.\n");
                exit(EXIT_FAILURE);
        } else if (argc > 2) {
                fprintf(stderr, "error: only one argument is expected - the file name.\n");
                exit(EXIT_FAILURE);
        }
        char *filename = argv[1];
        FILE *fp = fopen(filename, "a+");
        if (fp == NULL){
              fprintf(stderr, "error: failed to open file.\n");  
              exit(EXIT_FAILURE);
        }
        fprintf(fp, "Process id: %d\n", getpid());
        if (fclose(fp) != 0){
              fprintf(stderr, "error: failed to close result.txt.\n");  
              exit(EXIT_FAILURE);  
        }
        const char *lck = ".lck";  
        filename_lck = malloc((strlen(filename) + strlen(lck) + 1) * sizeof(char));
        if (filename_lck == NULL) {
                fprintf(stderr, "error: insufficient memory available.\n");
                exit(EXIT_FAILURE);
        }

        sprintf(filename_lck, "%s%s", filename, lck);

        while (true) {
                if (create_lck_file() == -1){
                        continue;
                }
                sleep(1);
                if (remove_lck_file() == -1){
                        continue;
                }
                locks_counter++;
        }
}