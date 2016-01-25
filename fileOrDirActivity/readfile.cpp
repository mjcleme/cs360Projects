#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <stdlib.h>

using namespace std;

main(int argc, char **argv)
{
    struct stat filestat;

    if(stat(argv[1], &filestat)) {
        cout <<"ERROR in stat\n";
    }
    if(S_ISREG(filestat.st_mode)) {
        FILE *fp = fopen(argv[1], "r");
        char *buffer = (char *)malloc(filestat.st_size);
        fread(buffer, filestat.st_size, 1, fp);
        printf("File: \n%s\n", buffer);
        free(buffer);
        fclose(fp);
    }
    if(S_ISDIR(filestat.st_mode)) {
        int len;
        DIR *dirp;
        struct dirent *dp;

        dirp = opendir(argv[1]);
        while ((dp = readdir(dirp)) != NULL)
            printf("name %s\n", dp->d_name);
        (void)closedir(dirp);
    }
    
}


