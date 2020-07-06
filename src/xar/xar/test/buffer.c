#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>
#include <xar/xar.h>

int32_t err_callback(int32_t sev, int32_t err, xar_errctx_t ctx, void *usrctx)
{
	printf("error callback invoked\n");
	return 0;
}

int main(int argc, char *argv[])
{
	int fd;
	unsigned char *buffer;
	struct stat sb;
	ssize_t red;
	xar_t x;
	xar_file_t f, f2;

	if( argc < 2 ) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		exit(1);
	}

	fd = open(argv[1], O_RDONLY);
	if( fd < 0 ) {
		fprintf(stderr, "Unable to open file %s\n", argv[1]);
		exit(2);
	}

	if( fstat(fd, &sb) < 0 ) {
		fprintf(stderr, "Unable to stat file %s\n", argv[1]);
		exit(3);
	}

	buffer = malloc(sb.st_size);
	if( buffer == NULL ) {
		fprintf(stderr, "Unable to allocate memory\n");
		exit(4);
	}

	red = read(fd, buffer, sb.st_size);
	if( red <= 0 ) {
		fprintf(stderr, "Error reading from file\n");
		exit(5);
	}
	if( red < sb.st_size )
		fprintf(stderr, "Incomplete read\n");

	x = xar_open("/tmp/test.xar", WRITE);
 	if( x == NULL ) {
		fprintf(stderr, "Error creating xarchive\n");
		exit(6);
	}

	xar_register_errhandler(x, err_callback, NULL);

	memset(&sb, 0, sizeof(sb));

	sb.st_mode = S_IFDIR | S_IRWXU;
	f = xar_add_folder(x, NULL, "mydir", &sb);
	if( !f ) {
		fprintf(stderr, "Error adding parent to archive\n");
		exit(7);
	}

	f2 = xar_add_frombuffer(x, f, "secondfile", buffer, red);
	if( !f ) {
		fprintf(stderr, "Error adding child to archive\n");
		exit(8);
	}

	xar_close(x);
	close(fd);
	exit(0);
}
