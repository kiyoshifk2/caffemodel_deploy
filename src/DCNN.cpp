
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <string.h>
#include <math.h>
#include <process.h>


static float d1[28*28]/*, dd1[28*28][5*5]*/;
static float w1[20][5*5];
static float b1[20];
static float d2[24*24][20];
static float d3[12*12][20]/*, dd3[12*12][20][5*5]*/;
static float w3[50][20*5*5];
static float b3[50];
static float d4[8*8][50];
static float d5[4*4][50], dxchange[50][4*4];
static float w5[500][4*4*50];
static float b5[500];
static float d6[500];
static float w6[10][500];
static float b6[10];
static float d7[10];

static void NNcnv1(float *in, int inlen, float *out, int outlen, float *w, float *b);

/********************************************************************************/
/*		NNmax_pool																*/
/********************************************************************************/
static void NNmax_pool(int x, int y, int hw, float *in, int in_w, int num, float *out)
{
	int i, r, xx, yy;
	float max;
	
	for(i=0; i<num; i++){
		max = -100000000;
		for(yy=0; yy<hw; yy++){
			for(xx=0; xx<hw; xx++){
				r = (x+xx) + (y+yy)*in_w;
				if(max < in[r*num + i]){
					max = in[r*num + i];
				}
			}
		}
		out[i] = max;
	}
}
/********************************************************************************/
/*		NNans																	*/
/********************************************************************************/
static void NNcnv(int x, int y, int hw, float *in, int in_w, int num1, float *out, int num2, float *w, float *b)
{
	int i, r, q, xx, yy;
	float work[28*5*5];
	
	for(yy=0; yy<hw; yy++){
		for(xx=0; xx<hw; xx++){
			q = xx+yy*hw;
			r = (x+xx)+(y+yy)*in_w;
			for(i=0; i<num1; i++)
				work[i*hw*hw+q] = in[r*num1+i];
		}
	}
	NNcnv1(work, hw*hw*num1, out, num2, w, b);
}

static void NNcnv1(float *in, int inlen, float *out, int outlen, float *w, float *b)
{
	float sum;
	int i, j;
	
	for(i=0; i<outlen; i++){
		sum = b[i];
		for(j=0; j<inlen; j++){
			sum += in[j] * w[j];
		}
		w += inlen;
		out[i] = sum;
	}
}
/********************************************************************************/
/*		net																		*/
/********************************************************************************/
void net(char in[28*28], float out[10])
{
	int i, x, y, p;
	float max, sum;
	
	//	“ü—Íƒf[ƒ^ì¬
	for(i=0; i<28*28; i++){
		d1[i] = (float)(unsigned char)in[i] / 256;
	}
	
	//	convolution 5x5 out 20
	for(x=0; x<24; x++){
		for(y=0; y<24; y++){
			p = x + y*24;
			NNcnv(x,y, 5, (float*)d1, 28, 1, d2[p], 20, (float*)w1, b1);
		}
	}
	//	pooling MAX kernel 2 stride 2
	for(x=0; x<12; x++){
		for(y=0; y<12; y++){
			p = x + y*12;
			NNmax_pool(x*2, y*2, 2, (float*)d2, 24, 20, d3[p]);
		}
	}
	//	convolution 5x5 out 50
	for(x=0; x<8; x++){
		for(y=0; y<8; y++){
			p = x + y*8;
			NNcnv(x,y, 5, (float*)d3, 12, 20, d4[p], 50, (float*)w3, b3);
		}
	}
	//	pooling Max kernel 2 stride 2
	for(x=0; x<4; x++){
		for(y=0; y<4; y++){
			p = x + y*4;
			NNmax_pool(x*2, y*2, 2, (float*)d4, 8, 50, d5[p]);
		}
	}
	//	InnerProduct in 4*4*50 out 500
	for(x=0; x<4*4; x++){
		for(y=0; y<50; y++){
			dxchange[y][x] = d5[x][y];
		}
	}
	NNcnv1((float*)dxchange, 4*4*50, d6, 500, (float*)w5, b5);
	//	Relu
	for(i=0; i<500; i++){
		if(d6[i] < 0){
			d6[i] = 0;
		}
	}
	//	InnerProduct in 500 out 10
	NNcnv1(d6, 500, d7, 10, (float*)w6, b6);
	//	softmax
	max = -10000000;
	for(i=0; i<10; i++){
		if(max < d7[i])
			max = d7[i];
	}
	sum = 0;
	for(i=0; i<10; i++){
		d7[i] = (float)exp(d7[i]-max);
		sum += d7[i];
	}
	for(i=0; i<10; i++){
		d7[i] /= sum;
	}
	
	memcpy(out, d7, 10*sizeof(float));
}
/********************************************************************************/
/*		NNinit																	*/
/********************************************************************************/
void NNinit()
{
	int fd, len, i;
	char str[256], buf[100];
	
//	strcpy(str, curdir);
	strcpy(str, "mnist-10000.dat");
	if((fd=open(str, O_RDONLY|O_BINARY))==-1){
		printf("*** %s open error\n", str);
		exit(0);
	}
	len = read(fd, w1, sizeof(w1));
	len = read(fd, b1, sizeof(b1));
	len = read(fd, w3, sizeof(w3));
	len = read(fd, b3, sizeof(b3));
	len = read(fd, w5, sizeof(w5));
	len = read(fd, b5, sizeof(b5));
	len = read(fd, w6, sizeof(w6));
	len = read(fd, b6, sizeof(b6));
	if(len != sizeof(b6)){
		printf("*** %s too short\n", str);
		exit(0);
	}
	len = read(fd, buf, 100);
	if(len){
		printf("*** %s too larg\n", str);
		exit(0);
	}
	close(fd);
}
