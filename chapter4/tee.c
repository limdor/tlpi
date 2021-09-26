#include "lib/error_functions.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>

void teeUsageError(){
    usageErr("tee [-a] <file_name>\n");
}

void parseTeeParameters(int argc, char** argv, bool* append, char const**filename){
    int opt;
    *append = false;
    *filename = NULL;
    opterr = 0; // Prevent getopt from printing errors
    while((opt = getopt(argc, argv, "a:")) != -1){
        switch (opt){
            case 'a':
                *append = true;
                *filename = optarg;
                break;
            default:
                teeUsageError();
        }
    }

    if(*filename == NULL){
        if(*filename == NULL && optind >= argc){
            teeUsageError();
        }else{
            if(optind + 1 < argc){
                teeUsageError();
            }
            *filename = argv[1];
        }
    } else if(optind < argc){
        teeUsageError();
    }
}

int main(int argc, char** argv){
    char const* filename;
    bool append;

    parseTeeParameters(argc, argv, &append, &filename);

    int const createFlags = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    int openFlags = O_WRONLY | O_CREAT;
    if(append){
        printf("Appending");
        openFlags |= O_APPEND;
    }else{
        printf("Writting");
    }
    printf(" to file %s the content of std input until EOF is found\n", filename);
    printf("You can manually enter EOF from console with Control+D (Linux/Unix) or Control+Z (Windows)\n");

    int const fileFd = open(filename, openFlags, createFlags);

    unsigned char const BUFFER_SIZE = 100;
    char buffer[BUFFER_SIZE];

    ssize_t bytes_read;
    do{
        bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE);
        if(bytes_read == -1){
            fprintf(stderr, "Error when reading from std input: %d\n", errno);
            return -1;
        }

        ssize_t const bytes_written_stdout = write(STDOUT_FILENO, buffer, bytes_read);
        if(bytes_written_stdout == -1){
            fprintf(stderr, "Error when writting to std output\n");
            return -1;
        }

        ssize_t const bytes_written_file = write(fileFd, buffer, bytes_read);
        if(bytes_written_file == -1){
            fprintf(stderr, "Error when writting to std output\n");
            return -1;
        }
    } while(bytes_read != 0);

    return 0;
}
