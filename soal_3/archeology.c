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
    char full_path[MAX_BUFFER];
    snprintf(full_path, MAX_BUFFER, "%s%s", source_path, path);
    printf("Full path: %s\n", full_path);

    statbuf->st_mode = S_IFREG | 0644;
    statbuf->st_nlink = 1;
    statbuf->st_size = 0;
    char apath[MAX_BUFFER + 4];
    FILE *fd;
    int i = 0;
    while (1)
    {
        snprintf(apath, MAX_BUFFER + 4, "%s.%03d", full_path, i++);
        printf("Checking fragment: %s\n", apath);
        fd = fopen(apath, "rb");
        if (!fd)
        {
            if (i == 1)
            {
                printf("File not found: %s\n", apath);
                printf("Leaving fuze_getattr with error for %s\n", path);
                return -ENOENT;
            }
            break;
        }
        fseek(fd, 0L, SEEK_END);
        statbuf->st_size += ftell(fd);
        fclose(fd);
    }
    printf("File size: %ld\n", statbuf->st_size);
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
    snprintf(full_path, MAX_BUFFER, "%s%s", source_path, path);
    printf("Full path: %s\n", full_path);

    DIR *dirpath = opendir(full_path);
    struct dirent *de;
    struct stat st;
    if (!dirpath)
    {
        printf("Could not open directory: %s\n", full_path);
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
    snprintf(full_path, MAX_BUFFER, "%s%s", source_path, path);
    printf("Full path: %s, size: %zu, offset: %ld\n", full_path, sz, os);

    char apath[MAX_BUFFER + 4];
    FILE *fdir;
    int i = 0;
    size_t size_read;
    size_t sum_read = 0;
    while (sz > 0)
    {
        snprintf(apath, MAX_BUFFER + 4, "%s.%03d", full_path, i++);
        printf("Reading from fragment: %s\n", apath);
        fdir = fopen(apath, "rb");
        if (!fdir)
        {
            printf("Could not open fragment: %s\n", apath);
            break;
        }
        fseek(fdir, 0L, SEEK_END);
        size_t sz_part = ftell(fdir);
        fseek(fdir, 0L, SEEK_SET);
        if (os >= sz_part)
        {
            printf("Skipping %s due to offset\n", apath);
            os -= sz_part;
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
    snprintf(full_path, MAX_BUFFER, "%s%s", source_path, path);
    printf("Full path: %s, size: %zu, offset: %ld\n", full_path, sz, os);

    char apath[MAX_BUFFER + 4];
    FILE *fdir;
    int pcur = os / MAX_SPLIT;
    size_t pos = os % MAX_SPLIT;
    size_t sum_write = 0;
    while (sz > 0)
    {
        snprintf(apath, MAX_BUFFER + 4, "%s.%03d", full_path, pcur++);
        printf("Writing to fragment: %s\n", apath);
        fdir = fopen(apath, "r+b");
        if (!fdir)
        {
            fdir = fopen(apath, "wb");
            if (!fdir)
            {
                printf("Could not create fragment: %s\n", apath);
                printf("Leaving fuze_write with error for %s\n", path);
                return -errno;
            }
        }
        fseek(fdir, pos, SEEK_SET);
        size_t sz_write = (sz > (MAX_SPLIT - pos)) ? (MAX_SPLIT - pos) : sz;
        printf("Writing %zu bytes to %s\n", sz_write, apath);
        fwrite(buffer, 1, sz_write, fdir);
        fclose(fdir);
        buffer += sz_write;
        sz -= sz_write;
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
    snprintf(full_path, MAX_BUFFER, "%s%s.000", source_path, path);
    printf("Creating file: %s\n", full_path);

    int rez = creat(full_path, md);
    if (rez == -1)
    {
        printf("Could not create file: %s\n", full_path);
        printf("Leaving fuze_create with error for %s\n", path);
        return -errno;
    }
    close(rez);
    printf("Leaving fuze_create for %s\n", path);
    return 0;
}

int fuze_unlink(const char *path)
{
    printf("Entering fuze_unlink for %s\n", path);
    char full_path[MAX_BUFFER];
    char apath[MAX_BUFFER + 4];
    snprintf(full_path, MAX_BUFFER, "%s%s", source_path, path);
    printf("Full path: %s\n", full_path);

    int pcur = 0;
    while (1)
    {
        snprintf(apath, MAX_BUFFER + 4, "%s.%03d", full_path, pcur++);
        printf("Unlinking fragment: %s\n", apath);
        int rez = unlink(apath);
        if (rez == -1)
        {
            if (errno == ENOENT)
                break;
            printf("Could not unlink fragment: %s\n", apath);
            printf("Leaving fuze_unlink with error for %s\n", path);
            return -errno;
        }
    }
    printf("Leaving fuze_unlink for %s\n", path);
    return 0;
}

int fuze_truncate(const char *path, off_t sz)
{
    printf("Entering fuze_truncate for %s\n", path);
    char full_path[MAX_BUFFER];
    char apath[MAX_BUFFER + 4];
    snprintf(full_path, MAX_BUFFER, "%s%s", source_path, path);
    printf("Full path: %s, size: %ld\n", full_path, sz);

    int pcurr_t = 0;
    size_t sz_part;
    off_t sz_rmn = sz;
    while (sz_rmn > 0)
    {
        snprintf(apath, MAX_BUFFER + 4, "%s.%03d", full_path, pcurr_t++);
        if (sz_rmn > MAX_SPLIT)
            sz_part = MAX_SPLIT;
        else
            sz_part = sz_rmn;
        printf("Truncating fragment: %s to size: %zu\n", apath, sz_part);
        int rez = truncate(apath, sz_part);
        if (rez == -1)
        {
            printf("Could not truncate fragment: %s\n", apath);
            printf("Leaving fuze_truncate with error for %s\n", path);
            return -errno;
        }
        sz_rmn -= sz_part;
    }
    int pcurr_u = 0;
    while (1)
    {
        snprintf(apath, MAX_BUFFER + 4, "%s.%03d", full_path, pcurr_u++);
        int rez = unlink(apath);
        if (rez == -1)
        {
            if (errno == ENOENT)
                break;
            printf("Could not unlink fragment: %s\n", apath);
            printf("Leaving fuze_truncate with error for %s\n", path);
            return -errno;
        }
    }
    printf("Leaving fuze_truncate for %s\n", path);
    return 0;
}

int fuze_utimens(const char *path, const struct timespec tspec[2])
{
    printf("Entering fuze_utimens for %s\n", path);
    char full_path[MAX_BUFFER];
    snprintf(full_path, MAX_BUFFER, "%s%s", source_path, path);
    printf("Full path: %s\n", full_path);

    char apath[MAX_BUFFER + 4];
    int pcurr = 0;
    while (1)
    {
        snprintf(apath, MAX_BUFFER + 4, "%s.%03d", full_path, pcurr++);
        printf("Updating timestamps for fragment: %s\n", apath);
        int res = utimensat(AT_FDCWD, apath, tspec, 0);
        if (res == -1)
        {
            if (errno == ENOENT)
                break;
            printf("Could not update timestamps for fragment: %s\n", apath);
            printf("Leaving fuze_utimens with error for %s\n", path);
            return -errno;
        }
    }
    printf("Leaving fuze_utimens for %s\n", path);
    return 0;
}

int main(int argc, char *argv[])
{
    struct fuse_operations fuze_oper = {
        .getattr = fuze_getattr,
        .readdir = fuze_readdir,
        .read = fuze_read,
        .write = fuze_write,
        .create = fuze_create,
        .unlink = fuze_unlink,
        .truncate = fuze_truncate,
        .utimens = fuze_utimens,
    };
    printf("Starting FUSE filesystem\n");
    return fuse_main(argc, argv, &fuze_oper, NULL);
}
