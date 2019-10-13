#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "serial.h"


#define CMD_TEST	0
#define CMD_EXE		1
#define PACK_LEN	32

char *serial_default="/dev/ttyUSB0";
char *exe_default="kernel.img";


void run_test(char *serial_path){
	char buf[PACK_LEN];
	serial_open(serial_path);
	memset(buf, 0, PACK_LEN);
	printf("Loader: TEST\n");
	serial_send(buf, PACK_LEN);
	memset(buf, 0, PACK_LEN);
	serial_recv(buf, PACK_LEN);
	serial_close();
	printf("Bootloader: %s\n", buf);
}

void boot_prog(char *serial_path, char *exe_path){
	FILE *fd;
	int n;
	int size;
	char msg[PACK_LEN];

	// open serial and file
	serial_open(serial_path);
	fd=fopen(exe_path, "rb");
	if(!fd){
		printf("ERROR: file_open: %s.\n %s\n", strerror(errno), exe_path);
		exit(1);
	}
	// get file size
	fseek(fd, 0L, SEEK_END);
	size=ftell(fd);
	fseek(fd, 0L, SEEK_SET);

	// send request
	printf("Loader: %d B\n", size);
	msg[0]=CMD_EXE;
	sprintf(msg+1, "%d", size);
	serial_send(msg, PACK_LEN);
	memset(msg, 0, PACK_LEN);
	serial_recv(msg, PACK_LEN);
	printf("Bootloader: %s\n", msg);

	// send data
	printf("Loader: Sending data.\n");
	while(size){
		n=fread(msg, 1, PACK_LEN, fd);
		if(!n){
			printf("ERROR: fread: %s.\n %s\n", strerror(errno), exe_path);
			exit(3);
		}
		serial_send(msg, n);
		size-=n;
	}
	printf("Loader: Sending complete.\n");
	fclose(fd);

	memset(msg, 0, PACK_LEN);
	serial_recv(msg, PACK_LEN);
	serial_close();
	printf("Bootloader: %s\n", msg);
}


int main(int argc, char **argv){
	int c;
	int test=0;
	char *exe_path=exe_default;
	char *serial_path=serial_default;

	while((c=getopt(argc, argv, "s:i:th"))!=-1){
		switch(c){
		case 's':
			serial_path=optarg;
			break;
		case 'i':
			exe_path=optarg;
			break;
		case 't':
			test=1;
			break;
		case 'h':
			printf("Usage:\n");
			printf(" -t        : run test\n");
			printf(" -h        : print help\n");
			printf(" -s [path] : set serial path\n");
			printf(" -i [path] : set executable path\n");
			exit(0);
		default:
			printf(" Illegal options.\n");
			exit(1);
		}
	}

	if(test)
		run_test(serial_path);
	else
		boot_prog(serial_path, exe_path);

	return 0;
}
