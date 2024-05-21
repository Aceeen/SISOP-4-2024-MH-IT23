#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_BUFFER 1028
#define MAX_SPLIT 10000
static const char *source_path = "/home/marcell/Desktop/sisop/modul4/soal3/relics";

int fuze_getattr(const char *path, struct stat *statbuf)
{
    printf("Entering fuze_getattr for %s\n", path);
    memset(statbuf, 0, sizeof(struct stat));
    statbuf->st_uid = getuid();
    statbuf->st_gid = getgid();
    statbuf->st_atime = time(NULL);
    statbuf->st_mtime = time(NULL);
    if (strcmp(path, "/") == 0)
    {
        statbuf->st_mode = S_IFDIR | 0755;
        statbuf->st_nlink = 2;
        printf("Leaving fuze_getattr for /\n");
        return 0;
    }
    char fpath[MAX_BUFFER];
    sprintf(fpath, "%s%s", source_path, path);
    statbuf->st_mode = S_IFREG | 0644;
    statbuf->st_nlink = 1;
    statbuf->st_size = 0;
    char apath[MAX_BUFFER + 4];
    FILE *fd;
    int i = 0;
    while (1)
    {
        sprintf(apath, "%s.%03d", fpath, i++);
        fd = fopen(apath, "rb");
        if (!fd)
            break;
        fseek(fd, 0L, SEEK_END);
        statbuf->st_size += ftell(fd);
        fclose(fd);
    }
    if (i == 1)
    {
        printf("Leaving fuze_getattr with error for %s\n", path);
        return -errno;
    }
    printf("Leaving fuze_getattr for %s\n", path);
    return 0;
}

int fuze_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    printf("Entering fuze_readdir for %s\n", path);
    (void)fi;
    (void)offset;
    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);
    char full_path[MAX_BUFFER];
    sprintf(full_path, "%s%s", source_path, path);
    DIR *dirpath = opendir(full_path);
    struct dirent *de;
    struct stat st;
    if (!dirpath)
    {
        printf("Leaving fuze_readdir with error for %s\n", path);
        return -errno;
    }
    while ((de = readdir(dirpath)) != NULL)
    {
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (strstr(de->d_name, ".000") == NULL)
            continue;
        char kats[MAX_BUFFER];
        strcpy(kats, de->d_name);
        kats[strlen(kats) - 4] = '\0';
        printf("Adding %s to directory listing\n", kats);
        if (filler(buffer, kats, &st, 0))
            break;
    }
    closedir(dirpath);
    printf("Leaving fuze_readdir for %s\n", path);
    return 0;
}

int fuze_read(const char *path, char *buffer, size_t sz, off_t os, struct fuse_file_info *fi)
{
    printf("Entering fuze_read for %s\n", path);
    char full_path[MAX_BUFFER];
    sprintf(full_path, "%s%s", source_path, path);
    char apath[MAX_BUFFER + 4];
    FILE *fdir;
    int i = 0;
    size_t size_read;
    size_t sum_read = 0;
    while (sz > 0)
    {
        sprintf(apath, "%s.%03d", full_path, i++);
        fdir = fopen(apath, "rb");
        if (!fdir)
            break;
        fseek(fdir, 0L, SEEK_END);
        size_t size_part = ftell(fdir);
        fseek(fdir, 0L, SEEK_SET);
        if (os >= size_part)
        {
            printf("Skipping %s due to offset\n", apath);
            os -= size_part;
            fclose(fdir);
            continue;
        }
        fseek(fdir, os, SEEK_SET);
        size_read = fread(buffer, 1, sz, fdir);
        printf("Read %zu bytes from %s\n", size_read, apath);
        fclose(fdir);
        buffer += size_read;
        sz -= size_read;
        sum_read += size_read;
        os = 0;
    }
    printf("Leaving fuze_read for %s, total %zu bytes read\n", path, sum_read);
    return sum_read;
}

int fuze_write(const char *path, const char *buffer, size_t sz, off_t os, struct fuse_file_info *fi)
{
    printf("Entering fuze_write for %s\n", path);
    char full_path[MAX_BUFFER];
    sprintf(full_path, "%s%s", source_path, path);
    char apath[MAX_BUFFER + 4];
    FILE *fdir;
    int pcur = os / MAX_SPLIT;
    size_t pos = os % MAX_SPLIT;
    size_t sum_write = 0;
    while (sz > 0)
    {
        sprintf(apath, "%s.%03d", full_path, pcur++);
        fdir = fopen(apath, "r+b");
        if (!fdir)
        {
            fdir = fopen(apath, "wb");
            if (!fdir)
            {
                printf("Leaving fuze_write with error for %s\n", path);
                return -errno;
            }
        }
        fseek(fdir, pos, SEEK_SET);
        size_t sz_write;
        if (sz > (MAX_SPLIT - pos))
            sz_write = MAX_SPLIT - pos;
        else
            sz_write = sz;
        printf("Writing %zu bytes to %s\n", sz_write, apath);
        fwrite(buffer, 1, sz_write, fdir); // Tambahkan tanda titik koma di sini ;
        fclose(fdir);
        buffer += sz_write;
        sz -= sz_write; // Ganti size dengan sz
        sum_write += sz_write;
        pos = 0;
    }
    printf("Leaving fuze_write for %s, total %zu bytes written\n", path, sum_write);
    return sum_write;
}

int fuze_create(const char *path, mode_t md, struct fuse_file_info *fi)
{
    printf("Entering fuze_create for %s\n", path);
    (void)fi;
    char full_path[MAX_BUFFER];
    sprintf(full_path, "%s%s.000", source_path, path);
    int res = creat(full_path, md);
    if (res == -1)
    {
        printf("Leaving fuze_create with error for %s\n", path);
        return -errno;
    }
    close(res);
    printf("Leaving fuze_create for %s\n", path);
    return 0;
}

int main(int argc, char *argv[])
{
    struct fuse_operations fuze_oper;
    memset(&fuze_oper, 0, sizeof(struct fuse_operations));
    fuze_oper.getattr = fuze_getattr;
    fuze_oper.readdir = fuze_readdir;
    fuze_oper.read = fuze_read;
    fuze_oper.write = fuze_write;
    fuze_oper.create = fuze_create;
    return fuse_main(argc, argv, &fuze_oper, NULL);
}
