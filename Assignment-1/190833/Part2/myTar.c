#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<dirent.h>
#include<string.h>
#include<sys/stat.h>

// function to return min value
long long int min(long long int x, long long int y){
    if(x<y) return x;
    else return y;
}
  
int main(int argc, char** argv){

    // -c argument
    if(!strcmp(argv[1], "-c")){
        struct dirent *entry, *count; 

        // moves to the directory
        if(chdir(argv[2])){
            printf("Failed to complete creation operation" );
            exit(-1);
        }
        DIR *reading = opendir(".");
        if (!reading){  
            printf("Failed to complete creation operation");
            exit(-1);
        }

        open(argv[3], O_CREAT, 0644);
        int fd = open(argv[3], O_RDWR);
        if(fd < 0){
            printf("Failed to complete creation operation");
            exit(-1);
        }

        long int filecount = 0;
        while( (count = readdir(reading)) != NULL ){
            if(strcmp(count->d_name, argv[argc-1]) && strcmp(count->d_name, ".") && strcmp(count->d_name, "..")){
                filecount++;
            }
        }

        reading = opendir(".");
        if(!reading){
            printf("Failed to complete creation operation");
            exit(-1);
        }

        if(write(fd, &filecount, sizeof(long int)) != sizeof(long long int)){
            printf("Failed to complete creation operation");
            exit(-1);
        }

        while ( (entry=readdir(reading)) != NULL ){
            if( strcmp(entry->d_name, argv[argc-1]) && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")){
                //file name
                char *filename = (char*)malloc(sizeof(char)*16);
                filename = entry->d_name;
                if(write(fd, filename, sizeof(filename)) != sizeof(filename)){
                    printf("Failed to complete creation operation");
                    exit(-1);
                }

                //size of file
                struct stat st;
                int newfd = open(entry->d_name, O_RDONLY);
                if(newfd < 0){
                    printf("Failed to complete creation operation");
                    exit(-1);
                }
                fstat(newfd, &st);
                long long int size = st.st_size;
                if(write(fd, &size, sizeof(long long int)) != sizeof(long long int)){
                    printf("Failed to complete creation operation");
                    exit(-1);
                }

                //file contents
                long long int maxsize = 100000;
                long long int minsize = min(maxsize, size);
                char *buf; 
                for(; size>0; size = size - minsize){
                    minsize = min(size, maxsize); 
                    buf = (char*)malloc(sizeof(char)*minsize);
                    if(read(newfd, buf, minsize) != minsize){
                        printf("Failed to complete creation operation");
                        exit(-1);
                    }

                    if(write(fd, buf, minsize) != minsize){
                        printf("Failed to complete creation operation");
                        exit(-1);
                    }
                    free(buf); 
                }
                close(newfd);
            }
        }
        closedir(reading);  
        close(fd);
    }





    // -d argument
    else if(!strcmp(argv[1], "-d")){

        //pathname
        char *pathname = (char*)malloc(sizeof(char)*strlen(argv[2]));
        strcpy(pathname, argv[2]);
        long int i;
        for(i = strlen(argv[2])-1; i>=0; i--){
            if(pathname[i]=='/'){
                pathname[i+1] = '\0';
                break;
            }
        }
        //filename
        char *filename = (char*)malloc(sizeof(char)*(strlen(argv[2])+4));
        for(long int j=i+1; j<strlen(argv[2]); j++){
            if(argv[2][j]=='.'){
                break;
            }
            char z = argv[2][j];
            strncat(filename, &z,1);
        }

        if(chdir(pathname)){
            printf("Failed to complete extraction operation" );
            exit(-1);
        }

        char* tarname = (char*)malloc(sizeof(char)*(sizeof(filename)+4));

        strcpy(tarname, filename);
        strcat(filename, "Dump");
        strcat(tarname, ".tar");

        int fd = open(tarname, O_RDONLY);
        if(fd < 0){
            printf("Failed to complete extraction operation");
            exit(-1);
        }
        
        mkdir(filename, 0777);
    
        if(chdir(filename)){
            printf("Failed to complete extraction operation" );
            exit(-1);
        }
        
        //filecount
        long int filecount;
        read(fd, &filecount, sizeof(long int));

        while(filecount--){
            char *filename1 = (char*)malloc(sizeof(char)*16);
            long long int size;

            if(read(fd, filename1, sizeof(filename1)) != sizeof(filename1)){
                printf("Failed to complete extraction operation");
                exit(-1);
            }
            if(read(fd, &size, sizeof(long long int)) != sizeof(long long int)){
                printf("Failed to complete extraction operation");
                exit(-1);
            }
            open(filename1, O_CREAT, 0644);
            int newfd = open(filename1, O_RDWR);
            if(newfd < 0){
                printf("Failed to complete extraction operation");
                exit(-1);
            }

            long long int maxsize = 100000;
            long long int minsize = min(maxsize, size);
            char *buf ; 
            for(; size>0; size = size - minsize){
                minsize = min(size, maxsize); 
                buf = (char*)malloc(sizeof(char)*minsize);
                if(read(fd, buf, minsize) != minsize){
                    printf("Failed to complete extraction operation");
                    exit(-1);
                }
                if(write(newfd, buf, minsize) != minsize){
                    printf("Failed to complete extraction operation");
                    exit(-1);
                }
                free(buf); 
            }
            close(newfd);
        }
        close(fd);
    }





    // -e argument
    else if(!strcmp(argv[1], "-e")){

        char *pathname = (char*)malloc(sizeof(char)*strlen(argv[2]));
        strcpy(pathname, argv[2]);
        long int i;
        for(i = strlen(argv[2])-1; i>=0; i--){
            if(pathname[i]=='/'){
                pathname[i+1] = '\0';
                break;
            }
        }

        char *tarname = (char*)malloc(sizeof(char)*(strlen(argv[2])+4));
        for(int j=i+1; j<strlen(argv[2]); j++){
            char z = argv[2][j];
            strncat(tarname, &z,1);
        }

        if(chdir(pathname)){
            printf("Failed to complete extraction operation" );
            exit(-1);
        }

        int fd = open(tarname, O_RDONLY);
        if(fd < 0){
            printf("Failed to complete extraction operation");
            exit(-1);
        }

        mkdir("IndividualDump", 0777);

        long int filecount;
        read(fd, &filecount, sizeof(long int));

        if(chdir("IndividualDump")){
            printf("Failed to complete extraction operation" );
            exit(-1);
        }

        int flag=1;
        while(filecount--){
            char *filename = (char*)malloc(sizeof(char)*16);
            long long int size;

            if(read(fd, filename, sizeof(filename)) != sizeof(filename)){
                printf("Failed to complete extraction operation");
                exit(-1);
            }
            if(read(fd, &size, sizeof(long long int)) != sizeof(long long int)){
                printf("Failed to complete extraction operation");
                exit(-1);
            }

            if(!strcmp(filename, argv[3])){
                open(filename, O_CREAT, 0644);
                int newfd = open(filename, O_RDWR);
                if(newfd < 0){
                    printf("Failed to complete extraction operation");
                    exit(-1);
                }

                long long int maxsize = 100000;
                long long int minsize = min(maxsize, size);
                char *buf ; 
                for(; size>0; size = size - minsize){
                    minsize = min(size, maxsize);
                    buf = (char*)malloc(sizeof(char)*minsize);
                    if(read(fd, buf, minsize) != minsize){
                        printf("Failed to complete extraction operation");
                        exit(-1);
                    }
                    if(write(newfd, buf, minsize) != minsize){
                        printf("Failed to complete extraction operation");
                        exit(-1);
                    }
                    free(buf);      
                }

                flag=0;
                close(newfd);
                break;
            }
            else{
                lseek(fd, size, SEEK_CUR);
            }
        }
        if(flag){
            printf("No such file is present in tar file.");
        }
        close(fd);
    }




    // -l argument
    else if(!strcmp(argv[1], "-l")){

        int fd = open(argv[2], O_RDONLY);
        if(fd < 0){
            printf("Failed to complete list operation");
            exit(-1);
        }
        
        char *pathname = (char*)malloc(sizeof(char)*strlen(argv[2]));
        strcpy(pathname, argv[2]);

        long int i;
        for(i = strlen(argv[2])-1; i>=0; i--){
            if(pathname[i]=='/'){
                pathname[i+1] = '\0';
                break;
            }
        }

        char *tarname = (char*)malloc(sizeof(char)*(strlen(argv[2])+4));
        for(int j=i+1; j<strlen(argv[2]); j++){
            char z = argv[2][j];
            strncat(tarname, &z,1);
        }

        if(chdir(pathname)){
            printf("Failed to complete list operation" );
            exit(-1);
        }

        open("tarStructure", O_CREAT, 0644);
        
        int newfd = open("tarStructure", O_RDWR);
        if(newfd < 0){
            printf("Failed to complete list operation");
            exit(-1);
        }

        struct stat st;
        fstat(fd, &st);
        long long int size = st.st_size;

        char *size1 = (char*)malloc(sizeof(char)*(30));
        sprintf(size1, "%lld\n", size);
        if(write(newfd, size1, strlen(size1)) != strlen(size1)){
            printf("Failed to complete list operation");
            exit(-1);
        }

        long int filecount;
        if(read(fd, &filecount, sizeof(long int)) != sizeof(long int)){
            printf("Failed to complete list operation");
            exit(-1);
        }

        char *ffilecount = (char*)malloc(sizeof(char)*(15));
        sprintf(ffilecount, "%ld\n", filecount);
        if(write(newfd, ffilecount, strlen(ffilecount)) != strlen(ffilecount)){
            printf("Failed to complete list operation");
            exit(-1);
        }

        while(filecount--){
            char *filename = (char*)malloc(sizeof(char)*16);
            long long int size;

            if(read(fd, filename, sizeof(filename)) != sizeof(filename)){
                printf("Failed to complete list operation");
                exit(-1);
            }
            if(read(fd, &size, sizeof(long long int)) != sizeof(long long int)){
                printf("Failed to complete list operation");
                exit(-1);
            }
            if(write(newfd, filename, strlen(filename)) != strlen(filename)){
                printf("Failed to complete list operation");
                exit(-1);
            }

            char *ssize = (char*)malloc(sizeof(char)*(30));
            sprintf(ssize, " %lld\n", size);
            if(write(newfd, ssize, strlen(ssize)) != strlen(ssize) ){
                printf("Failed to complete list operation");
                exit(-1);
            }

            lseek(fd, size, SEEK_CUR);
        }
        close(fd);
        close(newfd);
    }

    return 0;
}