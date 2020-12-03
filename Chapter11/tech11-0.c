#define FUSE_USE_VERSION 31
#include <errno.h>
#include <fuse3/fuse.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

void openFilesystem(const char* fileName)
{

    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        return;
    }

    struct stat st = {};
    stat(fileName, &st);
    if (st.st_size < sizeof(myFilesystem.numOfFiles)) {
        fclose(file);
        return;
    }

    privateData = &myFilesystem;
    fscanf(file, "%d", &(myFilesystem.numOfFiles));
    myFilesystem.files = (struct myFile_t*)calloc(
        myFilesystem.numOfFiles, sizeof(struct myFile_t));

    for (int i = 0; i < myFilesystem.numOfFiles; ++i)
        fscanf(
            file,
            "%ms %d",
            &(myFilesystem.files[i].fileName),
            &(myFilesystem.files[i].size));
    int scanned = fscanf(file, "\n\n");

    for (int i = 0; i < myFilesystem.numOfFiles; ++i) {
        myFilesystem.files[i].data =
            calloc(myFilesystem.files[i].size, sizeof(char));
        fread(
            myFilesystem.files[i].data,
            sizeof(char),
            myFilesystem.files[i].size,
            file);
    }

    fclose(file);
}

int findFile(const char* path)
{
    int indx = 0;
    while (indx < myFilesystem.numOfFiles &&
           0 != strcmp(path, myFilesystem.files[indx].fileName))
        ++indx;
    return indx;
}

void myDestroy(void* private_data)
{
    if (NULL == private_data)
        return;

    struct myFilesystem_t* data = (struct myFilesystem_t*)private_data;
    for (int i = 0; i < data->numOfFiles; ++i) {
        free(data->files[i].fileName);
        free(data->files[i].data);
    }
    free(data->files);
}

int myStat(const char* path, struct stat* st, struct fuse_file_info* fi)
{
    if (0 == strcmp("/", path)) {
        st->st_mode = S_IFDIR | 0555;
        st->st_nlink = 2;
        return 0;
    }

    int indx = findFile(path + 1);
    if (indx == myFilesystem.numOfFiles) {
        return -ENOENT;
    }

    st->st_mode = S_IFREG | 0444;
    st->st_nlink = 1;
    st->st_size = myFilesystem.files[indx].size;
    return 0;
}

int myReaddir(
    const char* path,
    void* out,
    fuse_fill_dir_t filler,
    off_t off,
    struct fuse_file_info* fi,
    enum fuse_readdir_flags flags)
{
    if (0 != strcmp(path, "/")) {
        return -ENOENT;
    }

    filler(out, ".", NULL, 0, 0);
    filler(out, "..", NULL, 0, 0);

    for (int i = 0; i < myFilesystem.numOfFiles; ++i)
        filler(out, myFilesystem.files[i].fileName, NULL, 0, 0);

    return 0;
}

int myOpen(const char* path, struct fuse_file_info* fi)
{
    int indx = findFile(path + 1);
    if (indx == myFilesystem.numOfFiles) {
        return -ENOENT;
    }

    if (O_RDONLY != (fi->flags & O_ACCMODE)) {
        return -EACCES;
    }
    return 0;
}

int myOpendir(const char* path, struct fuse_file_info* fi)
{
    if (0 != strcmp(path, "/")) {
        return -ENOENT;
    }
    return 0;
}

int myRead(
    const char* path,
    char* out,
    size_t size,
    off_t off,
    struct fuse_file_info* fi)
{
    int indx = findFile(path + 1);
    if (indx == myFilesystem.numOfFiles) {
        return -ENOENT;
    }

    if (off > myFilesystem.files[indx].size) {
        return 0;
    }

    if (off + size > myFilesystem.files[indx].size)
        size = myFilesystem.files[indx].size - off;

    const void* data = myFilesystem.files[indx].data + off;
    memcpy(out, data, size);
    return size;
}

static struct fuse_operations operations = {.getattr = myStat,
                                            .readdir = myReaddir,
                                            .open = myOpen,
                                            .opendir = myOpendir,
                                            .read = myRead,
                                            .destroy = myDestroy};

int main(int argc, char* argv[])
{
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    typedef struct {
        char* src;
    } myOptions_t;

    myOptions_t myOptions;
    memset(&myOptions, 0, sizeof(myOptions));

    struct fuse_opt optSpecs[] = {{"--src %s", offsetof(myOptions_t, src), 0},
                                  {NULL, 0, 0}};

    fuse_opt_parse(&args, &myOptions, optSpecs, NULL);
    if (myOptions.src) {
        openFilesystem(myOptions.src);
    }

    int ret = fuse_main(args.argc, args.argv, &operations, privateData);
    return ret;
}
