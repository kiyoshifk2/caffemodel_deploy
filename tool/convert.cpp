#include <stdio.h>
#include <io.h>
#include <process.h>
#include <fcntl.h>
#include <windows.h>
//#include <unistd.h>						/* read()							*/

unsigned char buf[100000000];
int bufptr;
int filelen;
int outfd;

int getnumber()
{
	int number, tmp;
	
	tmp = buf[bufptr++];
	number = tmp & 0x7f;
	if((tmp & 0x80)==0)
		return number;
	tmp = buf[bufptr++];
	number += (tmp & 0x7f)*128;
	if((tmp & 0x80)==0)
		return number;
	tmp = buf[bufptr++];
	number += (tmp & 0x7f)*128*128;
	if((tmp & 0x80)==0)
		return number;
	tmp = buf[bufptr++];
	number += (tmp & 0x7f)*128*128*128;
	if((tmp & 0x80)==0)
		return number;
	printf("number too large\n");
	exit(1);
}

void dumphex(int start, int len)
{
	FILE *fp;
	int i;
	
	fp = fopen("hex.log", "wa");
	if(fp==0)
		return;
	fprintf(fp, "start=%d len=%d\n", start, len);
	for(i=start; i<start+len; i++){
		fprintf(fp, "%02x%c", buf[i], i%16==0?'\n':' ');
	}
	fprintf(fp, "\n");
	fclose(fp);
}

void output(char *name, int endptr, int len)
{
	if(write(outfd, buf+bufptr, len) != len){
		printf("output write error\n");
		exit(1);
	}
//	bufptr += len;
	if(bufptr > endptr){
		printf("output buffer overrun\n");
		exit(1);
	}
}

void Parameter_No7(char *name, int endptr)
{
	int number, tag, tag1, endptr1, outputsize, tmp;
	char name1[256];
	FILE *fp;
	
	for(number=1; ; number++){
		if(bufptr==endptr)
			return;
		tag = getnumber();
		if(tag==0){
			printf("tag==0 exit\n");
			close(outfd);
			exit(0);
		}
		if(bufptr==endptr)
			return;
		if(bufptr > endptr){
			printf("V1Layer overrun\n");
			exit(1);
		}
		tag1 = tag >> 3;
		switch(tag1){
		case 1:
			tmp = getnumber();
			sprintf(name1, "%s.%02d-%d-num%d", name, number, tag1, tmp);
//			fp = fopen(name1, "w");
//			fclose(fp);
			break;
		case 2:
			tmp = getnumber();
			sprintf(name1, "%s.%02d-%d-channels%d", name, number, tag1, tmp);
//			fp = fopen(name1, "w");
//			fclose(fp);
			break;
		case 3:
			tmp = getnumber();
			sprintf(name1, "%s.%02d-%d-height%d", name, number, tag1, tmp);
//			fp = fopen(name1, "w");
//			fclose(fp);
			break;
		case 4:
			tmp = getnumber();
			sprintf(name1, "%s.%02d-%d-width%d", name, number, tag1, tmp);
//			fp = fopen(name1, "w");
//			fclose(fp);
			break;
		case 5:
			outputsize = getnumber();
			sprintf(name1, "%s.%02d-%d-%d", name, number, tag1, outputsize);
// No1
			output(name1, endptr, outputsize);
			bufptr += outputsize;
			break;
		default:
			outputsize = getnumber();
			sprintf(name1, "%s.%02d-%d-%d", name, number, tag1, outputsize);
// No2
//			output(name1, endptr, outputsize);
			bufptr += outputsize;
			break;
		}
	}
}

void LayerParameter(char *name, int endptr)
{
	int number, tag, tag1, endptr1, len;
	char name1[256];
	
	for(number=1; ; number++){
		if(bufptr==endptr)
			return;
		tag = getnumber();
		if(tag==0){
			printf("tag==0 exit\n");
			close(outfd);
			exit(0);
		}
		if(bufptr==endptr)
			return;
		if(bufptr > endptr){
			printf("V1Layer overrun\n");
			exit(1);
		}
		tag1 = tag >> 3;
		switch(tag1){
		case 7:
			sprintf(name1, "%s.%02d-%d", name, number, tag1);
			len = getnumber();
//			dumphex(bufptr, len);
			endptr1 = bufptr + len;
			Parameter_No7(name1, endptr1);
			if(endptr1 != bufptr){
				printf("data size mismatch\n");
				exit(1);
			}
			break;
		default:
			len = getnumber();
			sprintf(name1, "%s.%02d-%d-%d", name, number, tag1, len);
// No3
//			output(name1, endptr, len);
			bufptr += len;
			break;
		}
	}
}

void V1LayerParameter(char *name, int endptr)
{
	int number, tag, tag1, endptr1, len, tmp;
	char name1[256];
	FILE *fp;
	
	for(number=1; ; number++){
		if(bufptr==endptr)
			return;
		tag = getnumber();
		if(tag==0){
//			printf("tag==0 exit\n");
//			exit(0);
			continue; //$$$$$
		}
		if(bufptr==endptr)
			return;
		if(bufptr > endptr){
			printf("V1Layer overrun\n");
			exit(1);
		}
		tag1 = tag >> 3;
		switch(tag1){
		case 5: // data
			tmp = getnumber();
			sprintf(name1, "%s.%02d-%d-data%d", name, number, tag1, tmp);
//			fp = fopen(name1, "w");
//			fclose(fp);
			break;
		case 6:
			sprintf(name1, "%s.%02d-%d", name, number, tag1);
			len = getnumber();
			endptr1 = bufptr + len;
			Parameter_No7(name1, endptr1);
			if(endptr1 != bufptr){
				printf("data size mismatch\n");
				exit(1);
			}
			break;
		case 7: //fix 32
			len = 4;
			sprintf(name1, "%s.%02d-%d-%d", name, number, tag1, len);
// No4
//			output(name1, endptr, len);
			bufptr += len;
			break;
		default:
			len = getnumber();
			sprintf(name1, "%s.%02d-%d-%d", name, number, tag1, len);
// No5
			output(name1, endptr, len);
			bufptr += len;
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int fd1, fd2, number, tag, tag1, endptr, outputsize;
	char name[256];
	
	if(argc != 3){
		printf("> caffemodel_analize  caffemodel_file  output_dir\n");
		return 1;
	}
	if((fd1 = open(argv[1], O_RDONLY|O_BINARY)) < 0){
		printf("%s open error\n", argv[1]);
		return 1;
	}
	filelen = read(fd1, buf, 100000000);
	close(fd1);
	if((outfd = open(argv[2], O_WRONLY|O_CREAT|O_BINARY|O_TRUNC, 0777)) < 0){
		printf("%s open error\n", argv[2]);
		return 1;
	}
	bufptr = 0;
	for(number=1; ; number++){
		tag = getnumber();
		if(bufptr==filelen){
			printf("success\n");
			return 0;
		}
		if(bufptr > filelen){
			printf("main overrun\n");
			exit(1);
		}
		tag1 = tag >> 3;
		if((tag & 7) != 2){
			printf("illegal tag=%d\n", tag);
			exit(1);
		}
		switch(tag1){
		case 1:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			outputsize = getnumber();
			sprintf(name, "%s\\%03d-%d-%d", argv[2], number, tag1, outputsize);
// No6
//			output(name, filelen, outputsize);
			bufptr += outputsize;
			break;
		case 2:
			sprintf(name, "%s\\%03d-%d", argv[2], number, tag1);
			endptr = bufptr + getnumber();
			V1LayerParameter(name, endptr);
			if(endptr != bufptr){
				printf("data size mismatch\n");
				exit(1);
			}
			break;
		case 100:
			sprintf(name, "%s\\%03d-%d", argv[2], number, tag1);
			endptr = bufptr + getnumber();
			LayerParameter(name, endptr);
			if(endptr != bufptr){
				printf("data size mismatch\n");
				exit(1);
			}
			break;
		default:
			printf("illegal tag1=%d\n", tag1);
			exit(1);
		}
	}
	close(outfd);
}
