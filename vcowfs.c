#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>

char image[100];
int prev_time, snapshot_delay, nodes[100], node_ptr = 0;

typedef struct Buffer {
	unsigned int id;
	bool directory;
	mode_t permission;
	uid_t uid;
	gid_t gid;
	off_t size;
	time_t timestamp;
	char name[100];
	char data[1024];
} Buffer;

void read_file(struct Buffer *buffer, char *chr, char line[2048]){
	chr = strtok(line, ",");
	buffer->id = atoi(chr);

	chr = strtok(NULL, ",");
	buffer->directory = atoi(chr);

	chr = strtok(NULL, ",");
	buffer->permission = strtoul(chr, NULL, 8);

	chr = strtok(NULL, ",");
	buffer->uid = strtoul(chr, NULL, 10);

	chr = strtok(NULL, ",");
	buffer->gid = strtoul(chr, NULL, 10);

	chr = strtok(NULL, ",");
	buffer->size = atoi(chr);

	chr = strtok(NULL, ",");
	buffer->timestamp = atoi(chr);

	chr = strtok(NULL, ",");
	strcpy(buffer->name, chr);

	chr = strtok(NULL, "\r\n");
	strcpy(buffer->data, chr);
}

char* get_dirpath(char* str) {
	int i, index=strlen(str)-1;
	char *tmp = malloc(1000);
	sprintf(tmp, "%s", str);
	for(i=0; i<=index; index--) {
		if(tmp[index] == '/') {
			tmp[index+1] = '\0';
			return tmp;
		}
	}
	return NULL;
}

char* get_filename(char* str) {
	int i, index=strlen(str)-1;
	for(i=0; i<=index; index--) {
		if(str[index] == '/') {
			return &str[index];	
		}
	}
	return NULL;
}

static int do_getattr(const char *path, struct stat *stbuf) {
	printf("\tAttributes Called %s\n", path);
	
	FILE *file = fopen(image, "r");
	if(file == NULL)
		return -errno;
	struct Buffer buffer;
	char *chr, line[2048];
	
	while(1) {		
		chr = fgets(line, 2048, file);
		if(chr == NULL)
			return -errno;
		read_file(&buffer, chr, line);

		char fpath[1000];
		sprintf(fpath, "%s", path);
		sprintf(fpath, "%s%s",get_dirpath(fpath), buffer.name);
		int i;
		for(i=0; i<10; i++) {
			if(nodes[i]==buffer.id && (!strcmp(path, fpath) || !strcmp(path, buffer.name))) {
				stbuf->st_uid = buffer.uid; // The owner of the file/directory is the user who mounted the filesystem
				stbuf->st_gid = buffer.gid; // The group of the file/directory is the same as the group of the user who mounted the filesystem
				stbuf->st_atime = buffer.timestamp; // The last "a"ccess of the file/directory is right now
				stbuf->st_mtime = buffer.timestamp; // The last "m"odification of the file/directory
				stbuf->st_size = buffer.size; //This specifies the size of a regular file in bytes. For files that are really devices this field isn’t usually meaningful. For symbolic links this specifies the length of the file name the link refers to.
				if(buffer.directory) { // //Specifies the mode of the file. This includes file type information (see Testing File Type) and the file permission bits (see Permission Bits).
					stbuf->st_mode = S_IFDIR | buffer.permission;
					stbuf->st_nlink = 2;
				}
				else {
					stbuf->st_mode = S_IFREG | buffer.permission;
					stbuf->st_nlink = 1;
				}
				goto Out;
			}
		}
		Out: NULL;
	}
	fclose(file);
	return 0;
}

static int do_mknod(const char *path, mode_t mode, dev_t rdev) {
	printf("\tMake Node Called %s %u %u\n", path, mode, rdev);
/*
	int res;
	char fpath[1000];
	sprintf(fpath, "%s%s", dirpath, path);
	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable
	if (S_ISREG(mode)) {
		printf("\t\tS_ISREG\n");
		res = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);

		char fpath_backup[1000];
		sprintf(fpath_backup, "%s%s%s%s", get_dirpath(fpath), "/archive", get_filename(fpath), ".1");
		printf("\t\t%s\n", fpath_backup);
		res = open(fpath_backup, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0) {
			printf("\t\t\tres = %d\n", res);
			res = close(res);
		}

		prev_time = time(NULL);
	}

	else if (S_ISFIFO(mode)) {
		printf("\t\t S_ISFIFO\n");
		res = mkfifo(fpath, mode);
	}

	else {
		printf("\t\t mknod\n");
		res = mknod(fpath, mode, rdev);
	}
	
	if (res == -1)
		return -errno;
*/
	return 0;
}

static int do_mkdir(const char *path, mode_t mode) {
	printf("\tMake Directory Called %s %u\n", path, mode);
/*
	int res;
	char fpath[1000];
	sprintf(fpath, "%s%s", dirpath, path);
	
	res = mkdir(fpath, mode);
	if (res == -1)
		return -errno;
	
	char fpath_archive[1000];
	sprintf(fpath_archive, "%s%s", fpath, "/archive");
	res = mkdir(fpath_archive, mode);
	printf("\t\tMake Directory %s/archive\n", path);

	if (res == -1)
		return -errno;
*/
	return 0;
}
static int do_unlink(const char *path) {
	/*
	printf("\tUnlink Called %s\n", path);

	int res;
	char fpath[1000];
	sprintf(fpath, "%s%s", dirpath, path);
	res = unlink(fpath);
	if (res == -1)
		return -errno;
*/
	return 0;
}

static int do_rmdir(const char *path) {
	printf("\tRemove Directory Called %s\n", path);
/*
	int res;
	char fpath[1000];
	sprintf(fpath, "%s%s", dirpath, path);
	res = rmdir(fpath);
	if (res == -1)
		return -errno;
*/
	return 0;
}

static int do_rename(const char *from, const char *to) {
	printf("\tRename Called %s %s\n", from, to);
/*
	int res;
	char ffrom[1000];
	sprintf(ffrom, "%s%s", dirpath, from);
	char fto[1000];
	sprintf(fto, "%s%s", dirpath, to);
	res = rename(ffrom, fto);
	if (res == -1)
		return -errno;
*/
	return 0;
}

static int do_chmod(const char *path, mode_t mode) {
	printf("\tChange Mode Called %s %u\n", path, mode);
/*
	int res;
	char fpath[1000];
	sprintf(fpath, "%s%s", dirpath, path);
	res = chmod(fpath, mode);
	if (res == -1)
		return -errno;
*/
	return 0;
}

static int do_chown(const char *path, uid_t uid, gid_t gid) {
	printf("\tChange Owner Called %s %d %d\n", path, uid, gid);
/*
	int res;
	char fpath[1000];
	sprintf(fpath, "%s%s", dirpath, path);
	res = lchown(fpath, uid, gid);
	if (res == -1)
		return -errno;
*/
	return 0;
}

static int do_truncate(const char *path, off_t size) {
	printf("\tTruncate Called %s %u\n", path, size);
/*
	int res;
	char fpath[1000];
	sprintf(fpath, "%s%s", dirpath, path);
	res = truncate(fpath, size);
	if (res == -1)
		return -errno;
*/
	return 0;
}

static int do_open(const char *path, struct fuse_file_info *fi) {
	printf("\tOpen Called %s\n", path);
/*
	int res;
	char fpath[1000];
	sprintf(fpath, "%s%s", dirpath, path);
	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	fi->fh = res;
	return 0;
*/
}

static int do_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	printf("\tRead Called %s\n", path);
	/*printf( "--> Trying to read %s, %u, %u\n", path, offset, size );
	
	char file54Text[] = "Hello World From File54!";
	char file349Text[] = "Hello World From File349!";
	char *selectedText = NULL;
	
	// ... //
	
	if ( strcmp( path, "/file54" ) == 0 )
		selectedText = file54Text;
	else if ( strcmp( path, "/file349" ) == 0 )
		selectedText = file349Text;
	else
		return -1;
	
	// ... //
	
	memcpy( buffer, selectedText + offset, size );
	return strlen( selectedText ) - offset;*/

	FILE *file = fopen(image, "r");
	if(file == NULL)
		return -errno;
	struct Buffer buffer;
	char *chr, line[2048];

	while(1) {
		chr = fgets(line, 2048, file);
		if(chr == NULL)
			break;
		read_file(&buffer, chr, line);

		char fpath[1000];
		sprintf(fpath, "%s", path);
		sprintf(fpath, "%s%s", get_dirpath(fpath), buffer.name);
		if(!buffer.directory && !strcmp(path, fpath)) {
			memcpy(buf, buffer.data + offset, size);
			return strlen(buffer.data - offset);
		}
	}
	fclose(file);
	return -1;

	/*int fd;
		int res;
		if(fi == NULL)
			fd = open(fpath, O_RDONLY);
		else
			fd = fi->fh;

		if (fd == -1)
			return -errno;

		res = pread(fd, buf, size, offset);
		if (res == -1)
			res = -errno;

		if(fi == NULL)
			close(fd);
		return res;*/
}

static int do_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	printf("\tWrite Called %s %s %u %u\n", path, buf, size, offset);
/*
	int fd;
	int res;

	char fpath[1000];
	sprintf(fpath, "%s%s", dirpath, path);
	(void) fi;
	if(fi == NULL)
		fd = open(fpath, O_WRONLY);
	else
		fd = fi->fh;
	if (fd == -1)
		return -errno;
	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;
	if(fi == NULL)
		close(fd);

	time_t second = time(NULL);
	if(second - prev_time > snapshot_delay) {
		printf("\t\tBackup in new file version!!\n");

		int version = 2;
		char fpath_backup[1000];
		sprintf(fpath_backup, "%s%s%s%c%d", get_dirpath(fpath), "/archive", get_filename(fpath), '.', version);
		FILE * fp = fopen(fpath_backup, "r");
		while(fp) {
			printf("\t\t\tVersion %d found, find newer version\n", version);
			version++;
			fclose(fp);
			sprintf(fpath_backup, "%s%s%s%c%d", get_dirpath(fpath), "/archive", get_filename(fpath), '.', version);

			fp = fopen(fpath_backup, "r");
		}
		printf("\t\t\tBackup in version %d\n", version);
		sprintf(fpath_backup, "%s%s%s%c%d", get_dirpath(fpath), "/archive", get_filename(fpath), '.', version);
		printf("\t\t%s\n", fpath_backup);
		res = open(fpath_backup, O_CREAT | O_EXCL | O_WRONLY);
		if (res >= 0) {
			printf("\t\t\tres = %d\n", res);
			res = close(res);
		}
		fd = open(fpath_backup, O_WRONLY);
		res = pwrite(fd, buf, size, offset);
		close(fd);
	}

	else {
		printf("\t\tBackup in last file version\n");
		int version = 2;
		char fpath_backup[1000];
		sprintf(fpath_backup, "%s%s%s%c%d", get_dirpath(fpath), "/archive", get_filename(fpath), '.', version);
		FILE * fp = fopen(fpath_backup, "r");
		while(fp) {
			version++;
			fclose(fp);
			sprintf(fpath_backup, "%s%s%s%c%d", get_dirpath(fpath), "/archive", get_filename(fpath), '.', version);

			fp = fopen(fpath_backup, "r");
		}
		version--;
		sprintf(fpath_backup, "%s%s%s%c%d", get_dirpath(fpath), "/archive", get_filename(fpath), '.', version);
		printf("\t\t\tBackup in version %d\n", version);
		fd = open(fpath_backup, O_WRONLY);
		res = pwrite(fd, buf, size, offset);
		close(fd);
	}
	prev_time = second;
		return res;
	*/
}

static int do_release(const char *path, struct fuse_file_info *fi) {
	printf("\tRelease Called %s\n", path);

	(void) path;
	close(fi->fh);
	return 0;
}

static int do_opendir(const char *path, struct fuse_file_info *fi) {
	printf("\tOpen Directory Called %s\n", path);

	return 0;
}

static int do_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	printf("\tRead Directory Called %s\n", path);
	//printf( "--> Getting The List of Files of %s\n", path );
	
	//filler( buf, ".", NULL, 0 ); // Current Directory
	//filler( buf, "..", NULL, 0 ); // Parent Directory
	
	/*if ( strcmp( path, "/" ) == 0 ) // If the user is trying to show the files/directories of the root directory show the following
	{
		filler( buf, "file54", NULL, 0 );
		filler( buf, "file349", NULL, 0 );
	}*/

	FILE *file = fopen(image, "r");
	if(file == NULL)
		return -errno;
	struct Buffer buffer;
	char *chr, line[2048];

	while(1) {
		chr = fgets(line, 2048, file);
		if(chr == NULL)
			return -errno;
		read_file(&buffer, chr, line);

		char fpath[1000];
		sprintf(fpath, "%s", path);
		sprintf(fpath, "%s%s", get_dirpath(fpath), buffer.name);
		if(buffer.directory && buffer.data != "-1" && (!strcmp(path, fpath) || !strcmp(path, buffer.name))) {
			char *tmp = strtok(buffer.data, " ");
			int i;
			for(i=0; i<100; i++)
				nodes[i] = 0;
			for(i=0; i<100; i++) {
				if(tmp == NULL)
					break;
				nodes[i] = atoi(tmp);
				tmp = strtok(NULL, " ");
			}
			break;
		}
	}
	fclose(file);

	file = fopen(image, "r");
	int i;
	for(i=0; i<100; i++) {
		if(!nodes[i])
			break;

		while(1) {
			chr = fgets(line, 2048, file);
			if(chr == NULL)
				return -errno;
			read_file(&buffer, chr, line);

			if(buffer.id == nodes[i]) {
				char fpath[1000];
				//sprintf(fpath, "%s%s", path, buffer.name);
				//printf("\t%s\n", buffer.name);
				filler(buf, buffer.name, NULL, 0);
				break;
			}
		}
	}
	fclose(file);

	/*DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	char fpath[1000];
	sprintf(fpath, "%s%s", dirpath, path);

	dp = opendir(fpath);
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

	closedir(dp);*/
	return 0;
}

static int do_releasedir(const char *path, struct fuse_file_info *fi) {
	printf("\tRelease Directory Called %s\n", path);

	return 0;
}

static int do_fsync(const char *path, int isdatasync, struct fuse_file_info *fi) {
	printf("\tFSync Called %s %d\n", path, isdatasync);

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

static int do_fsyncdir(const char *path, int isdatasync, struct fuse_file_info *fi) {
	printf("\tFSync Directory Called %s %d\n", path, isdatasync);

	return 0;
}

static struct fuse_operations operations = {
	.getattr	= do_getattr,
	.mknod		= do_mknod,
	.mkdir 		= do_mkdir,
	.unlink 	= do_unlink,
	.rmdir 		= do_rmdir,
	.rename 	= do_rename,
	.chmod 		= do_chmod,
	.chown 		= do_chown,
	.truncate 	= do_truncate,
	//.open 		= do_open,
	.read		= do_read,
	.write 		= do_write,
	.release 	= do_release,
    //.opendir 	= do_opendir,
	.readdir	= do_readdir,
	.releasedir = do_releasedir,
	.fsync 		= do_fsync
    //.fsyncdir 	= do_fsyncdir
};

int main(int argc, char *argv[])
{
	if(argc == 5 && !strcmp(argv[3], "-t")) {
		char *path[2];
		path[0] = argv[0];
		path[1] = argv[2];

		sprintf(image, "%s", argv[1]);

		snapshot_delay = atoi(argv[4]);
		prev_time = time(NULL);

		fuse_main(2, path, &operations, NULL);
	}

	else if(argc == 6 && !strcmp(argv[1],"-f") && !strcmp(argv[4], "-t")) {
		char *path[3];
		path[0] = argv[0];
		path[1] = argv[1];
		path[2] = argv[3];

		sprintf(image, "%s", argv[2]);

		snapshot_delay = atoi(argv[5]);
		prev_time = time(NULL);

		fuse_main(3, path, &operations, NULL);
	}

	else {
		printf("Please run following this instruction:\n");
		printf("\t./vcowfs <Image File> <Mount Point> -t <Auto-snapshot Delay (seconds)>\n");
		printf("\t./vcowfs -f <Image File> <Mount Point> -t <Auto-snapshot Delay (seconds)>\n");
	}
	return 0;
}
