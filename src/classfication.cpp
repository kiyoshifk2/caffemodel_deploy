#include <stdio.h>
#include <io.h>
#include <process.h>
#include <fcntl.h>
#include <windows.h>



void net(char in[28*28], float out[10]);
void NNinit();

char HEADER[14]={'B','M',0,0,0,0,0,0,0,0,0,0,0,0};
char INFO[40];


unsigned char bmp_buf[10000];


char header[14]={
0x42,0x4d,0x66,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,
};
char info[40]={
0x28,0x00,0x00,0x00,0x1c,0x00,0x00,0x00,0x1c,0x00,0x00,0x00,0x01,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x30,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

/********************************************************************************/
/*		zyoban																	*/
/********************************************************************************/
void zyoban(int cnvflg, char *out, char *in)
{
	int x1, y1, x2, y2, tmp;
	
	for(y1=0; y1<28; y1++){
		for(x1=0; x1<28; x1++){
			x2 = x1;
			y2 = y1;
			if(cnvflg & 1){
				tmp = x2;
				x2 = y2;
				y2 = tmp;
			}
			else if(cnvflg & 2){
				x2 = 27-x2;
			}
			else if(cnvflg & 4){
				y2 = 27-y2;
			}
			out[y2*28 + x2] = in[y1*28 + x1];
		}
	}
}
/********************************************************************************/
/*		main																	*/
/********************************************************************************/
int main(int argc, char *argv[])
{
	int fd;
	int i, cnvflg, x, y, len, pt;
	char in[28*28];
	float out[10];
	
	if(argc != 2){
		printf("> class <bmp file>\n");
		return 0;
	}
	
	NNinit();
	
	if((fd=open(argv[1], O_RDONLY, O_BINARY)) < 0){
		printf("*** file open error %s\n", argv[1]);
		return 0;
	}
	len = read(fd, bmp_buf, 10000);
	close(fd);
	
	if(memcmp(header, bmp_buf, 14) || memcmp(info, &bmp_buf[14], 40) || len!=2406){
		printf("*** file must 24bit bitmap and 28x28 dot made by paint.\n");
		return 0;
	}
	for(pt=0,y=27; y>=0; y--){
		for(x=0; x<28; x++){
			in[y*28+x] = (bmp_buf[pt + 54]+bmp_buf[pt+1 + 54]+bmp_buf[pt+2 + 54])/3;
			pt += 3;
		}
	}
	
//	printf("cnvflg    0   1   2   3   4   5   6   7   8   9\n");
	printf("€–Ú      0   1   2   3   4   5   6   7   8   9\n");
	net(in, out);				// •ª—ÞŽÀs
//	printf("     %d:", cnvflg);
	printf("Šm—¦ %c ", '%');
	for(i=0; i<10; i++){
		printf(" %3.0f",out[i]*100);
	}
	printf("\n");
	
	return 0;
}
