# SISOP-4-2024-MH-IT23
1. Waktu pengerjaan dimulai hari Kamis (16 Mei 2024) hingga hari Selasa (21 Mei 2024) pukul 23.59 WIB.
2. Praktikan diharapkan membuat laporan penjelasan dan penyelesaian soal dalam bentuk Readme(github).
3. Format nama repository github “Sisop-[Nomor Modul]-2023-[Kode Dosen Kelas]-[Nama Kelompok]” (contoh:Sisop-4-2024-MH-IT01).
4. Struktur repository seperti berikut:
			—soal_1:
				— inikaryakita.c
                                       
      —soal_2:
	      — pastibisa.c

      —soal_3:
				— archeology.c
				

	Jika melanggar struktur repo akan dianggap sama dengan curang dan menerima konsekuensi sama dengan melakukan kecurangan.
6. Setelah pengerjaan selesai, semua script bash, awk, dan file yang berisi cron job ditaruh di github masing - masing kelompok, dan link github diletakkan pada form yang disediakan. Pastikan github di setting ke publik.
7. Commit terakhir maksimal 10 menit setelah waktu pengerjaan berakhir. Jika melewati maka akan dinilai berdasarkan commit terakhir.
8. Jika tidak ada pengumuman perubahan soal oleh asisten, maka soal dianggap dapat diselesaikan.
9. Jika ditemukan soal yang tidak dapat diselesaikan, harap menuliskannya pada Readme beserta permasalahan yang ditemukan.
10. Praktikan tidak diperbolehkan menanyakan jawaban dari soal yang diberikan kepada asisten maupun praktikan dari kelompok lainnya.
11. Jika ditemukan indikasi kecurangan dalam bentuk apapun di pengerjaan soal shift, maka nilai dianggap 0.
12. Pengerjaan soal shift sesuai dengan modul yang telah diajarkan.
13. Zip dari repository dikirim ke email asisten penguji dengan subjek yang sama dengan nama judul repository, dikirim sebelum deadline dari soal shift
14. Jika terdapat revisi soal akan dituliskan pada halaman terakhir

### SOAL 1
1. fungsi untuk menambah watermark
```
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
```
fungsi untuk reverse file
```
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
```
fungsi fuse
```
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

```
fungsi main yang akan memanggil fungsi fuse
```
int main(int argc, char *argv[]) {
    struct stat st;
    
    if (stat(wm_folder, &st) == -1) {
        mkdir(wm_folder, 0700);
    }

    printf("Gallery path: %s\n", gallery_path);

    return fuse_main(argc, argv, &xmp_oper, NULL);
}

```

Error : <br />
ketika program dimount ke fuse, tidak ada fungsi yang berhasil berjalan dengan benar. Watermark tidak muncul pada gambar, isi file test juga tidak di reverse. <br />
REVISI <br />
```
#define FUSE_USE_VERSION 31  
#include <fuse.h>            
#include <errno.h>           
#include <dirent.h>          
#include <stdio.h>           
#include <stdlib.h>          
#include <string.h>          
#include <sys/stat.h>        
#include <sys/types.h>       
#include <unistd.h>          

#define MAX_PATH_LENGTH 512 

void apply_watermark(const char *src_path, const char *dst_path) {
    char command[1024];  
    snprintf(command, sizeof(command), "convert \"%s\" -gravity south -pointsize 36 -fill white -draw \"text 0,0 'inikaryakita.id'\" \"%s\"",
             src_path, dst_path);
    system(command); 
}

void reverse_file_content(const char *src_path, const char *dst_path) {
    FILE *file = fopen(src_path, "r");  
    // Membuka file sumber untuk membaca
    FILE *outputFile = fopen(dst_path, "w");  
    // Membuka file tujuan untuk write

    if (file == NULL || outputFile == NULL) {  
        // Memeriksa eror
        perror("fopen");
        return;
    }

    fseek(file, 0, SEEK_END);  
    long fileSize = ftell(file);  
    fseek(file, 0, SEEK_SET);  

    char *content = malloc(fileSize + 1);  
    fread(content, 1, fileSize, file);  
    content[fileSize] = '\0';  

    for (long i = fileSize - 1; i >= 0; i--) {  
        fputc(content[i], outputFile);
    }

    free(content);  
    fclose(file);  
    fclose(outputFile);  
}

static int do_getattr(const char *path, struct stat *st) {
    memset(st, 0, sizeof(struct stat));  
    if (strcmp(path, "/") == 0 || strcmp(path, "/gallery") == 0 || strcmp(path, "/bahaya") == 0) {
        st->st_mode = S_IFDIR | 0755;  
        st->st_nlink = 2;  
    } else {
        st->st_mode = S_IFREG | 0644;  
        st->st_nlink = 1;
        st->st_size = 1024;
    }
    return 0;  
}

static int do_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void)offset;  
    (void)fi;      

    if (strcmp(path, "/") == 0) {  
        filler(buf, ".", NULL, 0);  
        filler(buf, "..", NULL, 0);  
        filler(buf, "gallery", NULL, 0);  
        filler(buf, "bahaya", NULL, 0);  
    } else if (strcmp(path, "/gallery") == 0 || strcmp(path, "/bahaya") == 0) { 
        DIR *dp;
        struct dirent *de;
        if ((dp = opendir(path)) == NULL) { 
            return -errno;
        }
        while ((de = readdir(dp)) != NULL) {  
            filler(buf, de->d_name, NULL, 0);  
        }
        closedir(dp);  
    }
    return 0;  
}

static int do_open(const char *path, struct fuse_file_info *fi) {
    return 0;  
}

static int do_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void)fi;  

    char src_path[MAX_PATH_LENGTH];  
    char temp_path[MAX_PATH_LENGTH];  
    snprintf(src_path, sizeof(src_path), ".%s", path);  

    if (strstr(path, "/gallery/") == path) {  
        snprintf(temp_path, sizeof(temp_path), "/tmp/watermarked%s", path + strlen("/gallery"));  
        apply_watermark(src_path, temp_path);  
        FILE *file = fopen(temp_path, "r");  
        if (file == NULL) {
            return -errno;  
        }
        fseek(file, offset, SEEK_SET);  
        size_t bytes_read = fread(buf, 1, size, file);  
        fclose(file);  
        return bytes_read;  
        snprintf(temp_path, sizeof(temp_path), "/tmp/reversed%s", path + strlen("/bahaya"));  
        reverse_file_content(src_path, temp_path);  
        FILE *file = fopen(temp_path, "r");  
        if (file == NULL) {
            return -errno;  
        }
        fseek(file, offset, SEEK_SET);  
        size_t bytes_read = fread(buf, 1, size, file);  
        fclose(file);  
        return bytes_read;  
    }
    return -ENOENT;  
    
}

static struct fuse_operations operations = {
    .getattr = do_getattr,  
    .readdir = do_readdir,  
    .open = do_open,        
    .read = do_read,        
};

int main(int argc, char *argv[]) {
    mkdir("gallery", 0755); 
    mkdir("bahaya", 0755);
    return fuse_main(argc, argv, &operations, NULL);  
}
```


### SOAL 2
### REVISI

### pastibisa.c

1. Main Function (main): Fungsi utama ini memeriksa apakah argumen yang diberikan cukup. Jika tidak, fungsi ini menampilkan pesan cara penggunaan yang benar. Selanjutnya, fungsi ini memanggil fuse_main dengan argumen yang diberikan serta struktur fuse_operations (xmp_oper) untuk memulai operasi FUSE.
```
#define FUSE_USE_VERSION 29

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>

const char *base_dir = "/home/syahrhm/sisop4/sensitif";
const char *log_file = "/home/syahrhm/sisop4/logs-fuse.log";
const char *password = "bisatapimati";

void decode_base64(const char *input, char *output);
void decode_rot13(const char *input, char *output);
char *decode_hex(const char *data);
char *reverse(const char *data);
void createLog(const char *status, const char *tag, const char *info);
int checkPass();
static int xmp_getattr(const char *path, struct stat *stbuf);
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

static struct fuse_operations xmp_oper = {
   .getattr = xmp_getattr,
   .readdir = xmp_readdir,
   .read = xmp_read,
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mountpoint>\n", argv[0]);
        return 1;
    }
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
```

2. Get Attributes (xmp_getattr): Fungsi ini menyusun jalur file dengan menggabungkan base_dir dan path yang diberikan. Kemudian, atribut file diambil menggunakan lstat, dan hasilnya dikembalikan. Jika terjadi kesalahan saat mengambil atribut, kesalahan tersebut dikembalikan.

```
static int xmp_getattr(const char *path, struct stat *stbuf) {
    char fpath[512];
    snprintf(fpath, sizeof(fpath), "%s%s", base_dir, path);
    int res = lstat(fpath, stbuf);
    if (res == -1) {
        return -errno;
    }
    return 0;
}
```

3. Read Directory (xmp_readdir): Fungsi ini menyusun jalur direktori dengan menggabungkan base_dir dan path. Direktori kemudian dibuka menggunakan opendir. Jika direktori adalah /rahasia, kata sandi diperiksa menggunakan checkPass. Jika pemeriksaan kata sandi gagal, log dicatat dan kesalahan akses dikembalikan. Entitas direktori dibaca satu per satu dengan readdir, dan buffer diisi menggunakan filler. Direktori kemudian ditutup dan hasilnya dikembalikan.

```
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    char fpath[512];
    snprintf(fpath, sizeof(fpath), "%s%s", base_dir, path);

    DIR *dp = opendir(fpath);
    if (dp == NULL) {
        createLog("ERROR", "opendir", strerror(errno));
        return -errno;
    }

    struct dirent *de;
    int res = 0;
    if (strncmp(path, "/rahasia", 8) == 0) {
        if (!checkPass()) {
            createLog("FAILED", "access", "Attempt to access rahasia folder with incorrect password");
            closedir(dp);
            return -EACCES; // Access denied if password is wrong
        }
    }

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        res = filler(buf, de->d_name, &st, 0);
        if (res != 0) {
            createLog("ERROR", "filler", "Buffer full or error");
            break;
        }
    }

    closedir(dp);
    return 0;
}
```

4. Read File (xmp_read): Fungsi ini menyusun jalur file dengan menggabungkan base_dir dan path. Jika file berada di dalam direktori /rahasia, kata sandi diperiksa menggunakan checkPass. Jika pemeriksaan gagal, log dicatat dan kesalahan akses dikembalikan. File dibuka menggunakan open dan isinya dibaca menggunakan pread ke buffer sementara decoded. Jika file berada dalam direktori /pesan/, konten didekode berdasarkan jenis enkode (base64, rot13, hex, atau reverse), dan log dicatat. File kemudian ditutup dan hasil pembacaan atau kesalahan dikembalikan.

```
static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char fpath[512];
    snprintf(fpath, sizeof(fpath), "%s%s", base_dir, path);

    if (strncmp(path, "/rahasia", 8) == 0) {
        if (!checkPass()) {
            createLog("FAILED", "access", "Attempt to access rahasia folder with incorrect password");
            return -EACCES; 
        } else {
            createLog("SUCCESS", "access", "Attempt to access rahasia folder with correct password");
        }
    }

    int fd = open(fpath, O_RDONLY);
    if (fd == -1) {
        return -errno;
    }

    char decoded[1024];
    int res = pread(fd, decoded, sizeof(decoded) - 1, offset);
    if (res == -1) {
        res = -errno;
        createLog("FAILED", "readFile", strerror(errno));
    } else {
        decoded[res] = '\0';

        if (strstr(path, "/pesan/") != NULL) {
            if (strstr(path, "base64") != NULL) {
                decode_base64(decoded, buf);
                createLog("SUCCESS", "decodeFile", "Base64");
            } else if (strstr(path, "rot13") != NULL) {
                decode_rot13(decoded, buf);
                createLog("SUCCESS", "decodeFile", "ROT13");
            } else if (strstr(path, "hex") != NULL) {
                char *decoded_hex = decode_hex(decoded);
                strncpy(buf, decoded_hex, size);
                free(decoded_hex);
                createLog("SUCCESS", "decodeFile", "Hex");
            } else if (strstr(path, "rev") != NULL) {
                char *decoded_rev = reverse(decoded);
                strncpy(buf, decoded_rev, size);
                free(decoded_rev);
                createLog("SUCCESS", "decodeFile", "Reverse");
            } else {
                strncpy(buf, decoded, size);
                createLog("SUCCESS", "readFile", path);
            }
        } else {
            strncpy(buf, decoded, size);
        }
    }

    close(fd);
    return res;
}
```

5. Base64 Decode (decode_base64): Fungsi ini mendekode string yang dienkode base64 menjadi format aslinya. Fungsi ini menggunakan tabel karakter base64 untuk mengonversi setiap karakter input menjadi karakter yang didekodekan.

```
void decode_base64(const char *input, char *output) {
    static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i, j = 0;
    unsigned char k;
    int len = strlen(input);

    for (i = 0; i < len; i += 4) {
        k = strchr(base64_chars, input[i]) - base64_chars;
        k = (k << 2) | ((strchr(base64_chars, input[i + 1]) - base64_chars) >> 4);
        output[j++] = k;

        if (input[i + 2] != '=') {
            k = ((strchr(base64_chars, input[i + 1]) - base64_chars) << 4) | ((strchr(base64_chars, input[i + 2]) - base64_chars) >> 2);
            output[j++] = k;
        }

        if (input[i + 3] != '=') {
            k = ((strchr(base64_chars, input[i + 2]) - base64_chars) << 6) | (strchr(base64_chars, input[i + 3]) - base64_chars);
            output[j++] = k;
        }
    }

    output[j] = '\0';
}
```

6. ROT13 Decode (decode_rot13): Fungsi ini mendekode string yang dienkode ROT13 menjadi format aslinya. Fungsi ini menggeser setiap karakter alfabet sebanyak 13 posisi dalam alfabet, mengembalikan karakter asli.

```
void decode_rot13(const char *data, char *decoded) {
    int i;
    int len = strlen(data);
    for (i = 0; i < len; i++) {
        if (isalpha(data[i])) {
            if (islower(data[i])) {
                decoded[i] = ((data[i] - 'a' + 13) % 26) + 'a';
            } else {
                decoded[i] = ((data[i] - 'A' + 13) % 26) + 'A';
            }
        } else {
            decoded[i] = data[i];
        }
    }
    decoded[len] = '\0';
}
```

7. Hex Decode (decode_hex): Fungsi ini mendekode string yang dienkode hex menjadi format aslinya. Fungsi ini mengalokasikan memori untuk string yang didekode dan mengembalikan pointer ke string tersebut, yang didekodekan dengan membaca setiap pasangan karakter heksadesimal.

```
char *decode_hex(const char *data) {
    size_t len = strlen(data) / 2;
    char *decoded = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        sscanf(data + 2 * i, "%2hhx", &decoded[i]);
    }
    decoded[len] = '\0';
    return decoded;
}
```

8. Reverse (reverse): Fungsi ini membalikkan string yang diberikan. Fungsi ini mengalokasikan memori untuk string yang dibalik dan mengembalikan pointer ke string tersebut dengan menyalin karakter dari belakang ke depan.

```
char *reverse(const char *data) {
    size_t len = strlen(data);
    char *reversed = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        reversed[i] = data[len - 1 - i];
    }
    reversed[len] = '\0';
    return reversed;
}
```

9. Create Log (createLog): Fungsi ini membuka file log untuk menambahkan entri baru. Fungsi ini menulis log dengan format yang mencakup status, tag, informasi, dan waktu saat log dibuat. Setelah menulis log, fungsi ini menutup file log.

```
void createLog(const char *status, const char *tag, const char *info) {
    FILE *log_fp = fopen(log_file, "a");
    if (log_fp) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        fprintf(log_fp, "[%s]::%02d/%02d/%04d-%02d:%02d:%02d::[%s]::[%s]\n",
                status,
                t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                t->tm_hour, t->tm_min, t->tm_sec,
                tag, info);
        fclose(log_fp);
    }
}
```

10. Check Password (checkPass): Fungsi ini meminta pengguna untuk memasukkan kata sandi dan memeriksa apakah kata sandi yang dimasukkan cocok dengan kata sandi yang ditentukan (password). Berdasarkan hasil pemeriksaan, log dicatat dan fungsi ini mengembalikan hasil pemeriksaan.

```
int checkPass() {
    char input[256];
    printf("Enter password: ");
    scanf("%255s", input);
    if (strcmp(input, password) == 0) {
        createLog("SUCCESS", "access", "Password correct");
        return 1;
    } else {
        createLog("FAILED", "access", "Incorrect password attempt");
        return 0;
    }
}
```

- Berikut dokumentasi ketika PASTIBISA.C di jalankan :
  
- dokumentasi ketika file berhasil di pindahkan ke folder kosong dan dapat mengenkripsi isi file
![Screenshot 2024-05-25 225149](https://github.com/Aceeen/Sisop-4-2024-MH-IT23/assets/151058945/7af3b71f-a33b-4f20-aad8-bb7685a2b9ad)

- dokumentasi bahwa gagal membuka folder rahasia-berkas dan berhasil menampilkan log
![Screenshot 2024-05-25 225233](https://github.com/Aceeen/Sisop-4-2024-MH-IT23/assets/151058945/a014d841-5c57-44a4-bb1c-eeb310d6ffe8)


### Soal 3
[archeology.c]
```
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
```
Melakukan define terhadap version fuse yang akan digunakan, library yang akan digunakan, size buffer dan maximum size dari fragment file pada relics yaitu 10 Kb serta melakukan spesifikasi base directory dimana file fragment akan disimpan
```
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
```
Membuat fungsi getattr dimana akan menghandle pengambilan atribut file dengan parameter path dan statbuf dengan cara menginisialisasi statbuf dengan UID dan GID, akses dan waktu modifikasi, cek apakah path root dan set mode directory serta menghitung link.

```
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
```
Membuat fungsi readdir dimana akan membaca isi dari sebuah directory dengan parameter path, buffer, filler, offset. Berfungsi untuk melakukan listing terhadap sebuah directory dan menghapus '.000' lalu memasukkannya ke listing directory
```
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
```
Membuat fungsi read dimana akan membaca data dari sebuah file dengan parameter path, buffer, sz, os. Berfungsi untuk membaca data dari fragment file yang benar berdasarkan ukuran dan offset file tersebut dengan cara mencari dan menentukan fragment yang akan dibaca berdasarkan offset serta membaca data yang diperlukan dari setiap fragment sampai ukuran yang diinginkan tercapai.
```
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
```
Membuat fungsi write dimana akan menulis data dari sebuah file dengan parameter, path, buffer, sz, os. Berfungsi untuk menulis data ke fragment file dan membuat fragment baru bila diperlukan dengan cara mencari serta menentukan fragment yang akan ditulis berdasarkan offset serta menulis data ke fragment yang sesuai dan membuatnya apabila belum ada.
```
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
```
Membuat fungsi create dimana akan membuat file baru dengan parameter path, md. Berfungsi untuk membuat file baru dengan fragment '.000' dengan cara membuat path untuk fragment pertama yaitu '.000' dan membuat filenya.
```
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
```
Membuat fungsi unlink untuk menghapus sebuah file dengan parameter path. Berfungsi untuk menghapus seluruh fragment dari sebuah file dengan cara melakukan loop terhadap seluruh fragment dan menghapus setiap fragment file sampai tidak ada lagi fragment file.
```
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
```
Membuat file truncate untuk mengurangi size sebuah file ke size yang diinginkan dengan parameter path dan sz. Berfungsi untuk mengurangi atau menambah size file dan menyesuaikan fragment file terhadap size file.
```
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
```
Membuat file utimens dimana akan mengupdate timestamp dari sebuah file dengan parameter path dan timespec. Berfunsi untuk menentukan access serta modifikasi waktu ke seluruh fragment file dengan cara melakukan loop terhadap keseluruhan fragment lalu melakukan update ke timestamps keseluruhan fragment file.
```
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
```
Pada fungsi main kita melakukan inisialisasi FUSE dan memulai filesystemnya dengan operasi yang telah kita buat dengan cara mendefinisikan sebuah struktu 'fuse_operations' dengan pointer ke functions yang akan diimplementasikan lalu memanggil 'fuse_main' untuk memulai filesystem FUSE.

Listing directory [nama_bebas]
![image](https://github.com/v0rein/SISOP-4-2024-MH-IT23/assets/143814923/fae936fe-0124-4918-b2ab-6fb1aed78073)

Hasil copy ke directory apapun dari [nama_bebas] dan memasukkan file ke [nama_bebas] lalu cek di directory relics
![image](https://github.com/v0rein/SISOP-4-2024-MH-IT23/assets/143814923/6dbea102-8c6e-4d33-9fa5-a96fa980eb04)

Delete file di [nama_bebas] maka fragment file di relics juga terhapus
![image](https://github.com/v0rein/SISOP-4-2024-MH-IT23/assets/143814923/7725b979-a213-4ee2-8ea3-5f3f96826152)

Cek directory report menggunakan samba share
![image](https://github.com/v0rein/SISOP-4-2024-MH-IT23/assets/143814923/bd3bc3b7-d1c8-4eb4-a36b-4d1e6f019be0)


