/*
 FUSE: Filesystem in Userspace
 Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
 This program can be distributed under the terms of the GNU GPL.
 See the file COPYING.
 gcc -Wall FS.c `pkg-config fuse --cflags --libs` -o FS
 
 Version 2.0
 Modified By: Ponpoorani Ravichandran
 Date: 11/27/2014
 Updates: Implemented a file system that manages processes in system.
 */

#define FUSE_USE_VERSION 26
#define BUFF_SIZE 100000
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>

static const char *proc_path = "/proc/";
static const char *hello_str = "Hello World!\n";
static const char *myproc_path = "/myproc";

void ls_Dir(const char *name, int level, void *buf, fuse_fill_dir_t filler);
int is_Proc(const char *p);
char* read_ProcInfo(const char *filename);

static int FS_getattr(const char *path, struct stat *stbuf) {
    int res = 0;
    printf("\n**************** In getAttr *********************");
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else if (strcmp(path, myproc_path) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(hello_str);
    } else if (is_Proc(path)) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(read_ProcInfo(path));
    }
    else
        res = -ENOENT; // No such file or directory
    
    return res;
}

static int FS_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi) {
    printf("\n**************** In readdir *********************");
    (void) offset;
    (void) fi;
    
    if (strcmp(path, "/") != 0)
        return -ENOENT; // No such file or directory
    
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    
    // list all the processes' names
    ls_Dir(proc_path, 0, buf, filler);
    
    return 0;
}

static int FS_open(const char *path, struct fuse_file_info *fi) {
    printf("\n**************** In Open *********************");
    if (!is_Proc(path + 1))
        return -ENOENT;  // No such file or directory
    
    if ((fi->flags & 3) != O_RDONLY)
        return -EACCES;  // Restricted access
    
    return 0;
}

static int FS_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi) {
    printf("\n**************** In read *********************");
    size_t len;
    (void) fi;
    if(is_Proc(path + 1)) {
        const char *p = read_ProcInfo(path);
        
        len = strlen(p);
        if (offset < len) {
            if (offset + size > len)
                size = len - offset;
            memcpy(buf, p + offset, size);
        }
        else {
            size = 0;
        }
        return size;
    }
    else {
        return 0;
    }
}

static struct fuse_operations FS_oper = {
    .getattr	= FS_getattr,
    .readdir	= FS_readdir,
    .open		= FS_open,
    .read		= FS_read,
};

// list the content of given direcotry
void ls_Dir(const char *name, int level, void *buf, fuse_fill_dir_t filler) {
    printf("\n**************** In listdir *********************");
    DIR *dir;
    struct dirent *entry;
    
    if (!(dir = opendir(name)) || !(entry = readdir(dir)))
        return;
    
    do {
       	// To check if file type is directory and a process directory (contains numbers in name)
        if ( (entry->d_type == DT_DIR) && (is_Proc(entry->d_name)) ) {
            char path[1024];
            int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
            path[len] = 0;
            
            filler(buf, entry->d_name, NULL, 0);
        }
    } while ( (entry = readdir(dir)) );
    closedir(dir);
}

// Check whether it is a process folder by checking whether string contains numbers.
int is_Proc(const char *p) {
    while(*p) {
        if (*p != '/' && (*p < '0' || *p > '9')) return 0;
        else p++;
    }
    return 1;
}

// read process info
char* read_ProcInfo(const char *filename) {
    printf("\n**************** In ReadProcInfo *********************");
    
    // generate full path
    char full_path[100] = "";
    strcat(full_path, "/proc");strcat(full_path, filename);strcat(full_path, "/status");
    char source[BUFF_SIZE + 1];
    
    FILE *fp = fopen(full_path, "r");
    if (fp != NULL) {
        size_t newLen = fread(source, sizeof(char), BUFF_SIZE, fp);
        if (newLen == 0) {
            fputs("Error reading file", stderr);
        } else {
            source[++newLen] = '\0';
        }
        
        fclose(fp);
    }
    return source;
}

// Main entry point of application / program.
int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &FS_oper, NULL);
}
