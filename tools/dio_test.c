#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
	void *buf;
	int fd;
	ssize_t ret;

	if (posix_memalign(&buf, 4096, 4096))
		return 1;

	memset(buf, 0x5a, 4096);

	fd = open("/data/local/tmp/dio_test.bin",
		  O_CREAT | O_RDWR | O_TRUNC | O_DIRECT, 0600);
	if (fd < 0) {
		perror("open write");
		return 2;
	}

	ret = write(fd, buf, 4096);
	if (ret < 0) {
		perror("write");
		return 3;
	}
	close(fd);

	fd = open("/data/local/tmp/dio_test.bin", O_RDONLY | O_DIRECT);
	if (fd < 0) {
		perror("open read");
		return 4;
	}

	ret = read(fd, buf, 4096);
	if (ret < 0) {
		perror("read");
		return 5;
	}

	close(fd);
	printf("O_DIRECT test ok, read=%zd\n", ret);
	return 0;
}
