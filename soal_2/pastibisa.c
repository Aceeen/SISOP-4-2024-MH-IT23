#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <time.h>
#include <ctype.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

// Variabel global
static const char *base_dir = "/path/to/base_dir"; // Sesuaikan dengan path yang benar
static const char *log_file = "/path/to/logs-fuse.log"; // Sesuaikan dengan path yang benar
static const char *password = "your_password"; // Ganti dengan password yang diinginkan

// Fungsi untuk mendekode Base64
void decode_base64(const char *input, char *output) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(input, -1);
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_get_mem_ptr(bio, &bufferPtr);

    memcpy(output, bufferPtr->data, bufferPtr->length);
    output[bufferPtr->length] = '\0';

    BIO_free_all(bio);
}

// Fungsi untuk mendekode ROT13
void decode_rot13(const char *input, char *output) {
    for (int i = 0; input[i] != '\0'; i++) {
        if ('a' <= input[i] && input[i] <= 'z') {
            output[i] = (input[i] - 'a' + 13) % 26 + 'a';
        } else if ('A' <= input[i] && input[i] <= 'Z') {
            output[i] = (input[i] - 'A' + 13) % 26 + 'A';
        } else {
            output[i] = input[i];
        }
    }
    output[strlen(input)] = '\0';
}

// Fungsi untuk mendekode Hexadecimal
void decode_hex(const char *input, char *output) {
    char buffer[3];
    int len = strlen(input) / 2;
    for (int i = 0; i < len; i++) {
        buffer[0] = input[2 * i];
        buffer[1] = input[2 * i + 1];
        buffer[2] = '\0';
        output[i] = (char) strtol(buffer, NULL, 16);
    }
    output[len] = '\0';
}

// Fungsi untuk membalikkan string
void reverse(const char *input, char *output) {
    int len = strlen(input);
    for (int i = 0; i < len; i++) {
        output[i] = input[len - i - 1];
    }
    output[len] = '\0';
}

// Fungsi untuk mencatat log
void createLog(const char *status, const char *tag, const char *information) {
    FILE *log_fp = fopen(log_file, "a");
    if (log_fp == NULL) {
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%d/%m/%Y-%H:%M:%S", t);

    fprintf(log_fp, "[%s]::%s::[%s]::[%s]\n", status, time_str, tag, information);
    fclose(log_fp);
}

// Fungsi untuk mengecek password
int checkPass(const char *input_pass) {
    return strcmp(input_pass, password) == 0;
}

static int xmp_getattr(const char *path, struct stat *stbuf) {
    int res;
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", base_dir, path);

    res = lstat(full_path, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", base_dir, path);
    
    DIR *dp;
    struct dirent *de;

    (void) offset;
    (void) fi;

    dp = opendir(full_path);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    closedir(dp);
    return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd;
    int res;
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", base_dir, path);

    (void) fi;
    fd = open(full_path, O_RDONLY);
    if (fd == -1)
        return -errno;

    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);

    // Dekode berdasarkan prefix pada nama file
    if (strstr(path, "base64_") == path) {
        decode_base64(buf, buf);
    } else if (strstr(path, "rot13_") == path) {
        decode_rot13(buf, buf);
    } else if (strstr(path, "hex_") == path) {
        decode_hex(buf, buf);
    } else if (strstr(path, "rev_") == path) {
        reverse(buf, buf);
    }

    return res;
}

static int xmp_access(const char *path, int mask) {
    if (strstr(path, "/rahasia") == path) {
        char input_pass[100];
        printf("Masukkan password: ");
        scanf("%s", input_pass);
        if (!checkPass(input_pass)) {
            createLog("FAILED", "access", "Password incorrect");
            return -EACCES;
        }
    }

    return 0;
}

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .read = xmp_read,
    .access = xmp_access,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
