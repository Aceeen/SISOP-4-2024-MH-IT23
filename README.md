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
