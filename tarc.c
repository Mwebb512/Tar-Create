//Matthew Webb
//tarc.c 
//C code that mimics how the OS creates a tar file
//3/18/24

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "jrb.h"
#include "fields.h"
#include "dllist.h"
#include <sys/types.h>
#include <sys/stat.h>

//DIRECTORY struct that is used for the dllist recursion to make it easier to store both paths
typedef struct directory {
	char full_path3[200];
    char path4[200];
} Directory;


//function that handles the operations of Tar
int tar(char* full_path, char* path2, JRB inodes){
    struct stat buf;
    int exists;

    exists = stat(full_path, &buf);

    //checks if file exist
    if (exists < 0){
        fprintf(stderr, "%s not found\n", full_path);
        jrb_free_tree(inodes);
        exit(1);
    }else{
        //check if symbolic link
        if(S_ISLNK(buf.st_mode)){
            return 1;
        }
    }
    



    //need size, name, inode, Mode, and Modidication file written in little endian
    int filename_size = strlen(path2);
    fwrite(&filename_size, 4, 1, stdout); //size of name
    fwrite(path2, 1, filename_size, stdout); //name
    fwrite(&buf.st_ino, 8, 1, stdout); //inode


    //check if node has already been processed
    if (jrb_find_int(inodes, buf.st_ino) != NULL){
        return 0;
    }else{
        jrb_insert_int(inodes, buf.st_ino, new_jval_s(full_path));
        
    }

    fwrite(&buf.st_mode, 4, 1, stdout); //mode
    fwrite(&buf.st_mtime, 8, 1, stdout); //modifitcation file
    
    //printf("path2: %d\n", filename_size);
    //If not a directory need File size and the bytes of the file
    if(S_ISDIR(buf.st_mode)){
        return 0;
    }else{

        //get the information from the file
        char* data[buf.st_size];
        FILE *file = fopen(full_path, "r");
        fread(&data, sizeof(char*), buf.st_size, file);
        //write to stdout
        fwrite(&buf.st_size, 8, 1, stdout); //file size
        fwrite(&data, 1, buf.st_size, stdout); //file data
        fclose(file);
    }

    return 0;

}


//recursive directory traversal to make sure all things are included in tar file
int traverse_directories(char* full_path, char* path2, JRB inodes){
    struct dirent *de;
    Dllist directories, tmp;
    DIR *d = opendir(full_path);
    char full_path2[200]; //full path with new directory
    char path3[200]; //relative path of file

    //dllist full of directories
    directories = new_dllist();

    //check if directory exist
    if (d == NULL) {
        perror("Error: directory does not exist");
        jrb_free_tree(inodes);
        free(directories);
        exit(1);
    }

    for(de = readdir(d); de != NULL; de = readdir(d)){
        //ignore "." and ".."
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
         
        }else{
            //create new paths from new directory
            snprintf(full_path2, sizeof(full_path2), "%s/%s", full_path, de->d_name);
            snprintf(path3, sizeof(path3), "%s/%s", path2, de->d_name);
            //print info
            tar(full_path2, path3, inodes);

            struct stat buf2;
            int exists3 = stat(full_path2, &buf2);

            if(exists3 < 0){
                fprintf(stderr, "%s not found\n", full_path);
                exit(1);
            }
            
            if (S_ISDIR(buf2.st_mode) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
                //create dllist for recursion
                Directory *dir2 = (Directory*) malloc(sizeof(Directory));
                strcpy(dir2->path4, path3);
                strcpy(dir2->full_path3, full_path2);
                dll_append(directories, new_jval_v(dir2));
            }
        }
    }

    

    closedir(d);
    
    //traverse dll_list to avoid to many open files
    dll_traverse(tmp, directories) {
        Directory *dir3 = (Directory*) tmp->val.v;
        traverse_directories(dir3->full_path3, dir3->path4, inodes);
        free(dir3);
    }

    free_dllist(directories);
    
}


int main(int argc, char* argv[]){
    
    if(argc != 2){
        perror("Error: incorrect number of arguments");
        exit(1);
    }

    char* full_path = strdup(argv[1]);
    char path2[200];
    char* prefix;
    char* suffix;
    JRB inodes = make_jrb();
    struct stat buf2;
    int exists2;

    char* slash = strrchr(full_path, '/');

    //get the name of file
    if(slash != NULL){
        strcpy(path2, slash +1);
    }else{
        strcpy(path2, full_path);
    }

    

    exists2 = stat(full_path, &buf2);
    if (exists2 < 0){
        fprintf(stderr, "%s not found\n", full_path);
        jrb_free_tree(inodes);
        
        exit(1);
    }else{
        tar(full_path, path2, inodes);
    }

    //traverse directories
    traverse_directories(full_path, path2, inodes);

    //printf("path2: %s\n", path2);
    return 0;
    
    jrb_free_tree(inodes);
    
}