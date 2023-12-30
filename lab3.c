#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MAX_PATH_LENGTH 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* copy_directory(void* arg);
void* copy_file(void* arg);
typedef struct _QueueNode {
    char* src;
    char* dest;
} data;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_directory> <destination_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

   // Создаем целевой каталог
    // if (mkdir(argv[2], 0777) == -1) {
    //     perror("mkdir");
    //     exit(EXIT_FAILURE);
    // }

    pthread_t thread;
    pthread_attr_t attr;
    data *information =  malloc(sizeof(data));

    information->src = malloc(sizeof(((char**)argv)[1]));
    information->dest = malloc(sizeof(((char**)argv)[2]));
    strcpy(information->src, ((char**)argv)[1]);
    strcpy(information->dest, ((char**)argv)[2]);

    // information->src = ((char**)argv)[1];
    // information->dest = ((char**)argv)[2];
    printf("+%s\n", information->src);
    printf("-%s\n", information->dest);
    
    if (pthread_attr_init(&attr) != 0) {
        perror("pthread_attr_init");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&thread, &attr, copy_directory, (void*)information) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(thread, NULL) != 0) {
        perror("pthread_join");
        exit(EXIT_FAILURE);
    }
    free(information->src);
    free(information->dest);
    free(information);

    return 0;
}

// int create_pthread(void* args){
//     pthread_t thread;



    
//     if (pthread_create(&thread, NULL, copy_directory, (void *)args) != 0) {
//         fprintf(stderr, "Error creating thread\n");
//         exit(EXIT_FAILURE);
//     }

//     if (pthread_detach(thread) != 0) {
//         fprintf(stderr, "Error detaching thread\n");
//         exit(EXIT_FAILURE);
//     }
// }

void* copy_file(void* arg) {
    data *info = (data *)arg;
    int fd_src, fd_dest;
    char buf[4096];
    ssize_t n;

    pthread_mutex_lock(&mutex);  

    if ((fd_src = open(info->src, O_RDONLY)) == -1) {
        perror("open-1");
        pthread_mutex_unlock(&mutex); 
        exit(EXIT_FAILURE);
    }

    if ((fd_dest = open(info->dest, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
        printf("2");
        perror("open-2");
        close(fd_src);
        pthread_mutex_unlock(&mutex); 
        exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&mutex); 
    while ((n = read(fd_src, buf, sizeof(buf))) > 0) {
        if (write(fd_dest, buf, n) != n) {
            perror("write");
            close(fd_src);
            close(fd_dest);
            exit(EXIT_FAILURE);
        }
    }

    if (n == -1) {
        perror("read");
    }
    free(info->src);
    free(info->dest);
    free(info);
    close(fd_src);
    close(fd_dest);
}

void* copy_directory(void* arg) {
    data *infor = (data *)arg;
    // char* src_path = ((char**)arg)[1];
    // char* dest_path = ((char**)arg)[2];
    //printf("----------%s\n",src_path);
    printf("%s\n", infor->src);
    printf("%s\n", infor->dest);
    DIR *dir;
    //data in_info[100];
    struct dirent *entry;
    struct stat statbuf;
    pthread_t tid[100];
    int count = 0;
    if (mkdir(infor->dest, 0777) == -1) {
        perror("mkdir++");
        exit(EXIT_FAILURE);
    }

    if ((dir = opendir(infor->src)) == NULL) {
        perror(infor->src);
        pthread_exit(NULL);
    }

    while ((entry = readdir(dir)) != NULL) {
        char src[MAX_PATH_LENGTH];
        char dest[MAX_PATH_LENGTH];

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; 
        }

        snprintf(src, sizeof(src), "%s/%s", infor->src, entry->d_name);
        snprintf(dest, sizeof(dest), "%s/%s", infor->dest, entry->d_name);

        if (lstat(src, &statbuf) == -1) {
            perror("lstat");
            continue;
        }

        data *infor1 =  malloc(sizeof(data));
        infor1->src = malloc(sizeof(src));
        infor1->dest = malloc(sizeof(src));
        pthread_attr_t attr;
        if (pthread_attr_init(&attr) != 0) {
            perror("pthread_attr_init");
            exit(EXIT_FAILURE);
        }
        strcpy(infor1->src, src);
        strcpy(infor1->dest, dest);

        if (S_ISDIR(statbuf.st_mode)) {

           // mkdir(, statbuf.st_mode);
        //    if (access(dest, F_OK) == -1) {
        //         // Директория не существует, создаем ее
        //         if (mkdir(dest, statbuf.st_mode) == -1) {
        //             perror("mkdir");
        //             pthread_exit(NULL);
        //         }
        //     }
            //printf("%s\n",src);
            pthread_create(&tid[count], &attr, copy_directory, (void*)infor1);
            //create_pthread((void*[]){arg, src, dest});
            //copy_directory();
        } else if (S_ISREG(statbuf.st_mode)) {
            pthread_create(&tid[count], &attr, copy_file, (void*)infor1);
            copy_file((void*)infor1);
        }
        count++;
    }
    for (int i = 0; i < count; i++) {
        pthread_join(tid[i], NULL);
    }
    //pthread_join(tid, NULL);
    closedir(dir);
    pthread_exit(NULL);
}
