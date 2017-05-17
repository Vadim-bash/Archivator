#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#define NAME_SIZE 255

int deskriptarch;
char archname[NAME_SIZE];

void alerterror(void)
{
	printf("error !!!");
	exit(1);
}
/*ffffff*/
void zip(char *path, char *dname)
{
	struct dirent *entry;
	struct stat statbuf;
	DIR *dir = opendir(path);

	if (dir == NULL) {
		printf("cannot open directory\n");
		exit(0);
	}
	chdir(path);
	if (write(deskriptarch, "{", 1) <= 0)
		alerterror();
	if (write(deskriptarch, dname, NAME_SIZE) != NAME_SIZE)
		alerterror();
	while (entry = readdir(dir)) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		lstat(entry->d_name, &statbuf);

		if (S_ISDIR(statbuf.st_mode))
			zip(entry->d_name, entry->d_name);
		else {
			int file;
			char *buf;

			if (write(deskriptarch, "f", 1) != 1)
				alerterror();
			if (write(deskriptarch, &statbuf.st_size, sizeof(size_t)) != sizeof(size_t))
				alerterror();
			if (write(deskriptarch, &entry->d_name, NAME_SIZE) != NAME_SIZE)
				alerterror();
			file = open(entry->d_name, O_RDONLY);
			buf = malloc(statbuf.st_size);
			if (read(file, buf, statbuf.st_size) != statbuf.st_size)
				alerterror();
			if (write(deskriptarch, buf, statbuf.st_size) != statbuf.st_size)
				alerterror();
			close(file);
			free(buf);
		}
	}
	if (write(deskriptarch, "}", 1) != 1)
		alerterror();
	closedir(dir);
	chdir("..");
}

void unzip(char *path)
{
	char name[NAME_SIZE];
	char flag;
	size_t n;

	chdir(path);
	while ((n = read(deskriptarch, &flag, 1)) != 0) {
		if (flag == '{') {
			if (read(deskriptarch, name, NAME_SIZE) != NAME_SIZE)
				alerterror();
			mkdir(name, S_IRUSR|S_IWUSR|S_IXUSR|S_IROTH);
			chdir(name);
		} else if (flag == '}') {
			chdir("..");
		} else {
			size_t size;
			size_t nb;
			int file;
			char *buf;

			if (read(deskriptarch, &size, sizeof(size_t)) != sizeof(size_t))
				alerterror();
			if (read(deskriptarch, name, NAME_SIZE) != NAME_SIZE)
				alerterror();
			file = open(name, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR|S_IROTH);
			buf = malloc(size);
			if (read(deskriptarch, buf, size) != size)
				alerterror();
			if (write(file, buf, size) != size)
				alerterror();
			close(file);
			free(buf);
		}
	}
}

void main(int argc, char *argv[])
{
	if (argc == 3 && strcmp(argv[1], "arch") == 0) {
		char *name = strrchr(argv[2], '/') + 1;

		strncat(archname, name, 251);
		strncat(archname, ".kva", 4);
		deskriptarch = open(archname, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR|S_IROTH);
		zip(argv[2], name);
		close(deskriptarch);
	} else if (argc == 4 && strcmp(argv[1], "razarch") == 0) {
		deskriptarch = open(argv[2], O_RDONLY);
		unzip(argv[3]);
		close(deskriptarch);
	} else {
		printf("wrong parameters!\n");
		printf("arch arch [directory to packaging]\n");
		printf("arch razarch [archive file] [path for unpackaging]\n");
	}
}
