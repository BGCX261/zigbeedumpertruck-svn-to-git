#ifndef _RS232_H
#define _RS232_H
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

class Rs232Interface {
	public:
		Rs232Interface(const char* dev, const int baudrate);
		~Rs232Interface();
		bool init();
		ssize_t putRaw(const void* buf, size_t count);
		bool putChar(unsigned char c);
		int bufferLen();
		std::string getString(int bytes);
		unsigned char getChar();
		int lastError() const;
	private:
		const char* dev;
		const int baudrate;
		int fd;
		int error;
};
#endif
