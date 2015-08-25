#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <termios.h>
#include <iostream>
#include <string>
#include "rs232.h"


Rs232Interface::Rs232Interface(const char* dev, const int baudrate) : dev(dev), baudrate(baudrate), fd(0), error(0) {
}

Rs232Interface::~Rs232Interface() {
	if (fd) close(fd);
}

bool Rs232Interface::init() {
	fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
		perror("open_port failed");
		return false;
	}
	fcntl(fd, F_SETFL, FNDELAY);
	
	struct termios options;
	tcgetattr(fd, &options);
	
	cfsetispeed(&options, baudrate);
	cfsetospeed(&options, baudrate);
	
	options.c_cflag |= CLOCAL | CREAD;
	
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	
//	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // raw
	
//	options.c_iflag &= ~(IXON | IXOFF | IXANY); // mitään flow controlia
	
//	options.c_oflag &= ~OPOST; // raw
	
//	memset(options.c_cc, 0, sizeof(cc_t) * NCCS); // hus kontrollimerkit (tarviikoha tätä)
	
	tcsetattr(fd, TCSANOW, &options);
	return true;
}

ssize_t Rs232Interface::putRaw(const void* buf, size_t count) {
	return write(fd, buf, count);
}

bool Rs232Interface::putChar(unsigned char c) {
	int n = write(fd, &c, 1);
	//if (n == -1 && errno == EAGAIN) n = write(fd, &c, 1);
	if (n != 1) error = errno;
	else error = 0;
	return n == 1;
}

int Rs232Interface::lastError() const {
	return error;
}

int Rs232Interface::bufferLen() {
	int bytes;
	ioctl(fd, FIONREAD, &bytes);
	return bytes;
}

std::string Rs232Interface::getString(int bytes) {
	std::string buf;
	char cbuf[256];
	while (bytes) {
		int n = read(fd, cbuf, bytes > 255 ? 255 : bytes);
		if (n < 1) return buf;
		cbuf[n] = 0;
		buf += cbuf;
		bytes -= n;
	}
	return buf;
}

unsigned char Rs232Interface::getChar() {
	unsigned char c = 0;
	read(fd, &c, 1);
	return c;
}
