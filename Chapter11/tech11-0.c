#define FUSE_USE_VERSION 31
#include <fuse3/fuse.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <stdio.h>

#define _DEBUG

#ifdef _DEBUG
    FILE* logFile = NULL;
    #define LOG(...) fprintf(logFile, __VA_ARGS__); fflush(logFile);
#else
    #define LOG(...)
#endif

struct myFile_t {
    char* fileName;
    int size;
    void* data;
};

struct myFilesystem_t {
    int numOfFiles;
    struct myFile_t* files;
};

static struct myFilesystem_t myFilesystem = {};
static void* privateData = NULL;

void openFilesystem(const char* fileName) {
LOG("openFilesystem called for %s\n", fileName)
    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        LOG("\tfailed to open\n")
        return;
    }

    LOG("\topened\n")
    struct stat st = {};
    stat(fileName, &st);
    if (st.st_size < sizeof(myFilesystem.numOfFiles)) {
        LOG("\ttoo small: %ld\n", st.st_size)
        fclose(file);
        return;
    }

    privateData = &myFilesystem;
    fscanf(file, "%d", &(myFilesystem.numOfFiles));
    myFilesystem.files = (struct myFile_t*)calloc(myFilesystem.numOfFiles, sizeof(struct myFile_t));
    LOG("\tnumOfFiles %d\n", myFilesystem.numOfFiles)
    for (int i = 0; i < myFilesystem.numOfFiles; ++i)
        fscanf(file, "%ms %d", &(myFilesystem.files[i].fileName), &(myFilesystem.files[i].size));
    int scanned = fscanf(file, "\n\n");
    LOG("scanned \\ns: %d\n", scanned)

    for (int i = 0; i < myFilesystem.numOfFiles; ++i) {
        myFilesystem.files[i].data = calloc(myFilesystem.files[i].size, sizeof(char));
        fread(myFilesystem.files[i].data, sizeof(char), myFilesystem.files[i].size, file);
    }

    fclose(file);
    LOG("\texit openFilesystem")
}

int findFile(const char* path) {
    int indx = 0;
    while (indx < myFilesystem.numOfFiles && 0 != strcmp(path, myFilesystem.files[indx].fileName))
        ++indx;
    return indx;
}

void myDestroy(void* private_data) {
LOG("myDestroy called\n")
    if (NULL == private_data)
        return;

    struct myFilesystem_t* data = (struct myFilesystem_t*) private_data;
    for (int i = 0; i < data->numOfFiles; ++i) {
        free(data->files[i].fileName);
        free(data->files[i].data);
    }
    free(data->files);
    #ifdef _DEBUG   
        fclose(logFile);
    #endif
}

int myStat(const char* path, struct stat *st, struct fuse_file_info* fi) {
LOG("myStat called for %s\n", path)
    if (0 == strcmp("/", path)) {
LOG("\tstat root dir\n")
        st->st_mode = S_IFDIR | 0555;
        st->st_nlink = 2;
        return 0;
    }
    
    int indx = findFile(path+1);
    if (indx == myFilesystem.numOfFiles) {
        LOG("\tnot found\n")
        return -ENOENT;
    }
    LOG("\tfound\n")
    st->st_mode = S_IFREG | 0444;
    st->st_nlink = 1;
    st->st_size = myFilesystem.files[indx].size;
    return 0;
}

int myReaddir(const char* path, void* out, fuse_fill_dir_t filler, off_t off, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
LOG("myReaddir called %s\n", path)
    if (0 != strcmp(path, "/")) {
        LOG("\tnot found\n")
        return -ENOENT;
    }
    LOG("\tfound\n")
    filler(out, ".", NULL, 0, 0);
    filler(out, "..", NULL, 0, 0);

    for (int i = 0; i < myFilesystem.numOfFiles; ++i)
        filler(out, myFilesystem.files[i].fileName, NULL, 0, 0);

    return 0;
}

int myOpen(const char* path, struct fuse_file_info* fi) {
LOG("myOpen called for %s\n", path)
    int indx = findFile(path+1);
    if (indx == myFilesystem.numOfFiles) {
        LOG("\tnot found\n")
        return -ENOENT;
    }

    if (O_RDONLY != (fi->flags & O_ACCMODE)) {
        LOG("\thave no access\n")
        return -EACCES;
    }

    LOG("\tAllright\n")
    return 0;
}

int myOpendir(const char* path, struct fuse_file_info* fi) {
LOG("myOpendie called for %s\n", path)
    if (0 != strcmp(path, "/")) {
    LOG("\tNOT FOUND\n")
        return -ENOENT;
    }
    LOG("\tFOUND\n")
    return 0;
}

int myRead(const char* path, char* out, size_t size, off_t off, struct fuse_file_info* fi) {
LOG("myRead called for %s\n", path)
    int indx = findFile(path+1);
    if (indx == myFilesystem.numOfFiles) {
        LOG("\tnot found\n")
        return -ENOENT;
    }

    LOG("\tfounf\n")
    if (off > myFilesystem.files[indx].size) {  
        LOG("\ttoo big offset\n")
        return 0;
    }

    if (off + size > myFilesystem.files[indx].size)
        size = myFilesystem.files[indx].size - off;

    const void* data = myFilesystem.files[indx].data + off;
    memcpy(out, data, size);
    return size;
}

static struct fuse_operations operations = {
    .getattr = myStat,
    .readdir = myReaddir,
    .open = myOpen,
    .opendir = myOpendir,
    .read = myRead,
    .destroy = myDestroy
};

int main(int argc, char* argv[]) {
#ifdef _DEBUG
    logFile = fopen("logger.txt", "w");
    if (logFile)
        printf("logFile opened\n");
    else printf("fail to open logFile");
#endif
LOG("start\n")
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
LOG("FUSE_ARGS_INIT passed\n")
    typedef struct {
        char* src;
    } myOptions_t;

    myOptions_t myOptions;
    memset(&myOptions, 0, sizeof(myOptions));
LOG("memset passed\n")
    struct fuse_opt optSpecs[] = {
        {"--src %s", offsetof(myOptions_t, src), 0},
        {NULL, 0, 0}
    };
LOG("optSpecs defined\n")    
    fuse_opt_parse(&args, &myOptions, optSpecs, NULL);
LOG("fuse_opt_parse passed\n")
    if (myOptions.src) {
LOG("myOptions.src true\n")
        openFilesystem(myOptions.src);
    } else {LOG("myOptions.src false")}
LOG("daemon is to start\n")
    int ret = fuse_main(args.argc, args.argv, &operations, privateData);
LOG("daemin started\n")
    return ret;
}
