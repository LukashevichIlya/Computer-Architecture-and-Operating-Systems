#include <dirent.h>
#include <grp.h>
#include <malloc.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 100
#define SIZE 10000

//////////////////////////////////////////////////////////////////////////////////

struct DirArray {
    struct dirent **dir_;
    size_t size_;
    size_t buffer_size_;
};

struct DirArray *MakeDirArray() {
    struct DirArray *new_array = (struct DirArray *) malloc(sizeof(struct DirArray));
    new_array->dir_ = (struct dirent **) malloc(sizeof(struct dirent *) * BUFFER_SIZE);
    new_array->size_ = 0;
    new_array->buffer_size_ = BUFFER_SIZE;
    return new_array;
}

void AddToDirArray(struct DirArray *array, struct dirent *new_dir) {
    array->dir_[array->size_] = new_dir;
    ++array->size_;

    if (array->buffer_size_ <= array->size_) {
        array->buffer_size_ += BUFFER_SIZE;
        array->dir_ = realloc(array->dir_, sizeof(struct dirent *) * array->buffer_size_);
    }
}

//////////////////////////////////////////////////////////////////////////////////

void PrintType(__mode_t st_mode) {
    if (S_ISREG(st_mode)) {
        putchar('-');
    } else if (S_ISDIR(st_mode)) {
        putchar('d');
    } else if (S_ISLNK(st_mode)) {
        putchar('l');
    } else if (S_ISCHR(st_mode)) {
        putchar('c');
    } else if (S_ISBLK(st_mode)) {
        putchar('b');
    } else if (S_ISFIFO(st_mode)) {
        putchar('p');
    } else {
        putchar('-');
    }
}

void PrintAttributes(__mode_t st_mode) {
    printf((st_mode & S_IRUSR) ? "r" : "-");
    printf((st_mode & S_IWUSR) ? "w" : "-");
    printf((st_mode & S_IXUSR) ? "x" : "-");
    printf((st_mode & S_IRGRP) ? "r" : "-");
    printf((st_mode & S_IWGRP) ? "w" : "-");
    printf((st_mode & S_IXGRP) ? "x" : "-");
    printf((st_mode & S_IROTH) ? "r" : "-");
    printf((st_mode & S_IWOTH) ? "w" : "-");
    printf((st_mode & S_IXOTH) ? "x" : "-");
    printf(" ");
}

void PrintLinksNumber(__nlink_t st_nlink) {
    printf("%zu ", st_nlink);
}

void PrintUser(__uid_t st_uid) {
    struct passwd *user_id = getpwuid(st_uid);

    if (!user_id) {
        printf("%d ", st_uid);
    } else {
        printf("%s ", user_id->pw_name);
    }
}

void PrintGroup(__gid_t st_gid) {
    struct group *group_id = getgrgid(st_gid);

    if (!group_id) {
        printf("%d ", st_gid);
    } else {
        printf("%s ", group_id->gr_name);
    }
}

void PrintSize(__off_t st_size) {
    printf("%zu ", st_size);
}

//////////////////////////////////////////////////////////////////////////////////

struct DirArray *PrintSingleDir(struct DirArray *array, char *path) {
    char new_path[SIZE];
    new_path[0] = 0;
    strcat(new_path, path);
    size_t path_length = strlen(new_path);

    if (new_path[path_length - 1] != '/') {
        new_path[path_length] = '/';
        ++path_length;
    }

    size_t directories_counter = 0;

    for (size_t i = 0; i < array->size_; ++i) {
        if (array->dir_[i]->d_name[0] == '.') {
            continue;
        }

        new_path[path_length] = 0;
        strcat(new_path, array->dir_[i]->d_name);

        struct stat current_stat;
        if (lstat(new_path, &current_stat) == -1) {
            perror("lstat");
        }

        if (S_ISDIR(current_stat.st_mode)) {
            array->dir_[directories_counter] = array->dir_[i];
            ++directories_counter;
        }

        PrintType(current_stat.st_mode);
        PrintAttributes(current_stat.st_mode);
        PrintLinksNumber(current_stat.st_nlink);
        PrintUser(current_stat.st_uid);
        PrintGroup(current_stat.st_gid);
        PrintSize(current_stat.st_size);
        printf("%s", array->dir_[i]->d_name);

        if (S_ISLNK(current_stat.st_mode)) {
            char link[SIZE];
            ssize_t size = readlink(new_path, link, sizeof(link));
            if (size >= 0) {
                link[size] = 0;
                printf(" -> %s", link);
            }
        }

        printf("\n");

    }

    printf("\n");

    array->size_ = directories_counter;

    return array;
}

int CompareDirNames(const void *first, const void *second) {
    const struct dirent **d_first = (const struct dirent **) first;
    const struct dirent **d_second = (const struct dirent **) second;

    return strcmp((*d_first)->d_name, (*d_second)->d_name);
}

void Traverse(char *path, struct DirArray *(*PrintSingleDir)(struct DirArray *arr, char *name)) {
    DIR *directory = opendir(path);

    if (directory) {
        struct dirent *dir;
        struct DirArray *array = MakeDirArray();

        printf("%s:\n", path);

        while ((dir = readdir(directory))) {
            AddToDirArray(array, dir);
        }

        qsort(array->dir_, array->size_, sizeof(array->dir_), CompareDirNames);
        array = PrintSingleDir(array, path);

        char new_path[SIZE];
        new_path[0] = 0;
        strcat(new_path, path);
        size_t path_length = strlen(new_path);

        if (new_path[path_length - 1] != '/') {
            new_path[path_length] = '/';
            new_path[++path_length] = '\0';
        }

        for (size_t i = 0; i < array->size_; ++i) {
            new_path[path_length] = 0;
            strcat(new_path, array->dir_[i]->d_name);
            Traverse(new_path, PrintSingleDir);
        }

        closedir(directory);

    } else {
        perror(path);
    }
}

int main(int argc, char **argv) {
    if (argc == 1) {
        Traverse(".", &PrintSingleDir);
    } else {
        Traverse(argv[1], &PrintSingleDir);
    }
    return 0;
}