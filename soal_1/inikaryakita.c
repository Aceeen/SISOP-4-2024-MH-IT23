#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

static const char *gallery_path = "/home/aceen/testing/portofolio/gallery";
static const char *wm_folder = "/home/aceen/testing/portofolio/gallery/wm.";
static const char *bahaya_path = "/home/aceen/testing/portofolio/bahaya";
static const char *script_file = "/home/aceen/testing/portofolio/bahaya/script.sh";

void add_watermark(const char *input, const char *output) {
    char command[1024];
    sprintf(command, "convert %s -gravity South -annotate 0 'inikaryakita.id' %s", input, output);
    system(command);
}

void reverse_file_content(const char *input, const char *output) {
    FILE *in = fopen(input, "r");
    if (in == NULL) {
        perror("fopen input");
        return;
    }

    fseek(in, 0, SEEK_END);
    long fsize = ftell(in);
    fseek(in, 0, SEEK_SET);

    char *content = malloc(fsize + 1);
    fread(content, 1, fsize, in);
    fclose(in);
    content[fsize] = '\0';

    FILE *out = fopen(output, "w");
    if (out == NULL) {
        perror("fopen output");
        free(content);
        return;
    }

    for (long i = fsize - 1; i >= 0; i--) {
        fputc(content[i], out);
    }

    fclose(out);
    free(content);
}

static int xmp_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    int res;
    char fpath[1000];
    
    sprintf(fpath, ".%s", path);
    res = lstat(fpath, stbuf);

    if (res == -1) return -errno;

    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
    (void) offset;
    (void) fi;
    (void) flags;

    DIR *dp;
    struct dirent *de;
    char fpath[1000];

    sprintf(fpath, ".%s", path);
    dp = opendir(fpath);
    if (dp == NULL) return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0, 0)) break;
    }

    closedir(dp);
    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi) {
    int res;
    char fpath[1000];

    sprintf(fpath, ".%s", path);
    res = open(fpath, fi->flags);
    if (res == -1) return -errno;

    close(res);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd;
    int res;
    char fpath[1000];

    sprintf(fpath, ".%s", path);
    fd = open(fpath, O_RDONLY);
    if (fd == -1) return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;

    close(fd);
    return res;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd;
    int res;
    char fpath[1000];

    sprintf(fpath, ".%s", path);
    fd = open(fpath, O_WRONLY);
    if (fd == -1) return -errno;

    res = pwrite(fd, buf, size, offset);
    if (res == -1) res = -errno;

    close(fd);
    return res;
}

static int xmp_rename(const char *from, const char *to, unsigned int flags) {
    int res;
    char ffrom[1000];
    char fto[1000];

    if (flags) return -EINVAL;

    sprintf(ffrom, ".%s", from);
    sprintf(fto, ".%s", to);

    if (strncmp(fto, wm_folder, strlen(wm_folder)) == 0) {
        // watermark
        add_watermark(ffrom, fto);
        unlink(ffrom); // rm file ori
    } else if (strncmp(fto, bahaya_path, strlen(bahaya_path)) == 0 && strncmp(fto + strlen(bahaya_path) + 1, "test", 4) == 0) {
        // reverse
        reverse_file_content(ffrom, fto);
        unlink(ffrom); // rm file ori
    } else {
        res = rename(ffrom, fto);
        if (res == -1) return -errno;
    }

    return 0;
}

static int xmp_chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) fi;
    int res;
    char fpath[1000];

    sprintf(fpath, ".%s", path);

    if (strcmp(fpath, script_file) == 0) {
        mode |= S_IXUSR | S_IXGRP | S_IXOTH;
    }

    res = chmod(fpath, mode);
    if (res == -1) return -errno;

    return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size) {
    int res;
    char fpath[1000];

    sprintf(fpath, ".%s", path);
    res = readlink(fpath, buf, size - 1);
    if (res == -1) return -errno;

    buf[res] = '\0';
    return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev) {
    int res;
    char fpath[1000];

    sprintf(fpath, ".%s", path);
    res = mknod(fpath, mode, rdev);
    if (res == -1) return -errno;

    return 0;
}

static int xmp_unlink(const char *path) {
    int res;
    char fpath[1000];

    sprintf(fpath, ".%s", path);
    res = unlink(fpath);
    if (res == -1) return -errno;

    return 0;
}

static int xmp_mkdir(const char *path, mode_t mode) {
    int res;
    char fpath[1000];

    sprintf(fpath, ".%s", path);
    res = mkdir(fpath, mode);
    if (res == -1) return -errno;

    return 0;
}

static int xmp_rmdir(const char *path) {
    int res;
    char fpath[1000];

    sprintf(fpath, ".%s", path);
    res = rmdir(fpath);
    if (res == -1) return -errno;

    return 0;
}

static int xmp_symlink(const char *from, const char *to) {
    int res;
    char ffrom[1000];
    char fto[1000];

    sprintf(ffrom, ".%s", from);
    sprintf(fto, ".%s", to);
    res = symlink(ffrom, fto);
    if (res == -1) return -errno;

    return 0;
}

static struct fuse_operations xmp_oper = {
    .getattr    = xmp_getattr,
    .readdir    = xmp_readdir,
    .open       = xmp_open,
    .read       = xmp_read,
    .write      = xmp_write,
    .rename     = xmp_rename, 
    .chmod      = xmp_chmod,
    .readlink   = xmp_readlink,
    .mknod      = xmp_mknod,
    .unlink     = xmp_unlink,
    .mkdir      = xmp_mkdir,
    .rmdir      = xmp_rmdir,
    .symlink    = xmp_symlink,
};

int main(int argc, char *argv[]) {
    struct stat st;
    
    if (stat(wm_folder, &st) == -1) {
        mkdir(wm_folder, 0700);
    }

    printf("Gallery path: %s\n", gallery_path);

    return fuse_main(argc, argv, &xmp_oper, NULL);
}
