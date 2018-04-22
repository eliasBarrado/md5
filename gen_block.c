#include <stdlib.h>
#include <stdio.h>
//#include <string.h>
#include <time.h>
//#include <ctype.h>
#include <math.h>
#include <stdint.h>
//#include <unistd.h>

#define RR(x, n) (((x) >> (n)) | ((x) << (32-(n))))
#define RL(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

uint32_t FF(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac){
 	a = F(b,c,d) + a + x + ac;
 	a = RL(a,s);
 	a = a + b;
 	return a;
 }

uint32_t GG(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac){
 	a = G(b,c,d) + a + x + ac;
 	a = RL(a,s);
 	a = a + b;
 	return a;
 }

uint32_t HH(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac){
 	a = H(b,c,d) + a + x + ac;
 	a = RL(a,s);
 	a = a + b;
 	return a;
 }
uint32_t II(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac){
 	a = I(b,c,d) + a + x + ac;
 	a = RL(a,s);
 	a = a + b;
 	return a;
 }


//mask_bit[i] is the number that has 1 in bit position i, 0 otherwise. Only used by bit function.
const uint32_t mask_bit[33] = { 0x0, 
                                0x00000001,0x00000002,0x00000004,0x00000008,0x00000010,0x00000020,0x00000040,0x00000080,
                                0x00000100,0x00000200,0x00000400,0x00000800,0x00001000,0x00002000,0x00004000,0x00008000,
                                0x00010000,0x00020000,0x00040000,0x00080000,0x00100000,0x00200000,0x00400000,0x00800000,
                                0x01000000,0x02000000,0x04000000,0x08000000,0x10000000,0x20000000,0x40000000,0x80000000 };

uint32_t X;
uint32_t frandom() {
  //X = (1664525*X + 1013904223) & 0xffffffff;
  X = (1103515245*X + 12345) & 0xffffffff;
  return X;
}

/*Returns the b-th bit of a
 if a = 0x4000001 
 	bit(a, 1) returns 1
 	bit(a,31) returns 1	*/
uint32_t bit(uint32_t a, uint32_t b) {
    if ((b==0) || (b > 32)) 
      return 0;
    else
      return (a & mask_bit[b]) >> (b-1);
}

//Default init vectors
uint32_t IV1=0x67452301; 
uint32_t IV2=0xefcdab89;
uint32_t IV3=0x98badcfe; 
uint32_t IV4=0x10325476;


int block(void){

	uint32_t Q[65], m[16] , QM0, QM1, QM2, QM3;


	//Initialization vectors
  	QM3 = IV1;  QM0 = IV2;
  	QM1 = IV3;  QM2 = IV4;

	// Hex table
	// 0 = 0000
	// 1 = 0001
	// 2 = 0010
	// 3 = 0011
	// 4 = 0100
	// 5 = 0101
	// 6 = 0110
	// 7 = 0111
	// 8 = 1000
	// 9 = 1001
	// A = 1010
	// B = 1011
	// C = 1100
	// D = 1101
	// E = 1110
	// F = 1111
  	int i = 0;
	while(1){

		if(i%1000000 == 0){
			printf("%2.2x\n", X);
	    	printf("%d\n", i);
		}

	    i = i +1;

		// Q[1]  = .... .... .... .... .... .... .... .... 
	    Q[1]  = frandom();

	    // Q[2]  = .... .... .... .... .... .... .... .... 
	    Q[2]  = frandom();

	    // Q[3]  = .... .... .... 0... .... 0... .... ....
	    Q[3]  = frandom() & 0xfff7f7ff;

	    // Q[4]  = 1... .... 0^^^ 1^^^ ^^^^ 1^^^ ^... .... 
	    Q[4]  = (frandom() & 0x7f00007f) + 0x80080800 + (Q[3] & 0x0077f780);

	    // Q[5]  = 1... 1.0. 01.. 0000 0000 0000 001. .1.1 
	    Q[5]  = (frandom() & 0x7530001a) + 0x88400025;

	    // Q[6]  = 0000 001^ 0111 1111 1011 1100 0100 ^0^1 
	    Q[6]  = 0x027fbc41 + (Q[5] & 0x0100000a);

	    // Q[7]  = 0000 0011 1111 1110 1111 1000 0010 0000 
	    Q[7]  = 0x03fef820;

	    // Q[8]  = 0000 0001 1..1 0001 0.0. 0101 0100 0000 
	    Q[8]  = (frandom() & 0x00605000) + 0x01910540;

	    // Q[9]  = 1111 1011 ...1 0000 0.1^ 1111 0011 1101 
	    Q[9]  = (frandom() & 0x00e04000) + 0xfb102f3d + (Q[8] & 0x00001000);

	    // Q[10] = 01.. .... 0..1 1111 1101 ...0 01.. ..00 
	    Q[10] = (frandom() & 0x3f600e3c) + 0x401fd040;

	    // Q[11] = 00.. .... .... 0001 1100 ...0 11.. ..10 
	    Q[11] = (frandom() & 0x3ff00e3c) + 0x0001c0c2;

	    // Q[12] = 00.. ..^^ .... 1000 0001 ...1 0... .... 
	    Q[12] = (frandom() & 0x3cf00e7f) + 0x00081100 + (Q[11] & 0x03000000);

	    // Q[13] = 01.. ..01 .... 1111 111. ...0 0... 1... 
	    Q[13] = (frandom() & 0x3cf01e77) + 0x410fe008;

	    // Q[14] = 0.0. ..00 .... 1011 111. ...1 1... 1... 
	    Q[14] = (frandom() & 0x5cf01e77) + 0x000be188;

	    // Q[15] = 0.1. ..01 .... .... 1... .... .... 0... 
	    Q[15] = (frandom() & 0x5cff7ff7) + 0x21008000;

	    // Q[16] = 0.1. .... .... .... .... .... .... .... 
	    Q[16] = (frandom() & 0x5fffffff) + 0x20000000;

		//Compute block 
	    m[ 0] = RR(Q[ 1] - QM0  ,  7) - F(QM0  , QM1  , QM2  ) - QM3   - 0xd76aa478;  
	    m[ 1] = RR(Q[17] - Q[16],  5) - G(Q[16], Q[15], Q[14]) - Q[13] - 0xf61e2562;
	    m[ 4] = RR(Q[ 5] - Q[ 4],  7) - F(Q[ 4], Q[ 3], Q[ 2]) - Q[ 1] - 0xf57c0faf;
	    m[ 5] = RR(Q[ 6] - Q[ 5], 12) - F(Q[ 5], Q[ 4], Q[ 3]) - Q[ 2] - 0x4787c62a;
	    m[ 6] = RR(Q[ 7] - Q[ 6], 17) - F(Q[ 6], Q[ 5], Q[ 4]) - Q[ 3] - 0xa8304613; 
	    m[10] = RR(Q[11] - Q[10], 17) - F(Q[10], Q[ 9], Q[ 8]) - Q[ 7] - 0xffff5bb1; 
	    m[11] = RR(Q[12] - Q[11], 22) - F(Q[11], Q[10], Q[ 9]) - Q[ 8] - 0x895cd7be; 
	    m[15] = RR(Q[16] - Q[15], 22) - F(Q[15], Q[14], Q[13]) - Q[12] - 0x49b40821; 

	    
	    // Q[17] = 0... .... .... ..0. ^... .... .... ^... 
	    Q[17] = GG(Q[13], Q[16], Q[15], Q[14], m[ 1],  5, 0xf61e2562);
	    if ( bit(Q[17],32) || bit(Q[17],18) || bit(Q[17],16) != bit(Q[17],16) || bit(Q[17],4) != bit(Q[16],4) )
	    	continue;
	    


		// Q[18] = 0.^. .... .... ..1. .... .... .... ....
	    Q[18] = GG(Q[14], Q[17], Q[16], Q[15], m[ 6],  9, 0xc040b340);
	    if ( bit(Q[18],32) || bit(Q[18],30) != bit(Q[18],30) || !bit(Q[18],18) )
	    	continue;
	    


	    // Q[19] = 0... .... .... ..0. .... .... .... ....
	    Q[19] = GG(Q[15], Q[18], Q[17], Q[16], m[11], 14, 0x265e5a51);
	    if ( bit(Q[19],32) || bit(Q[19],18) )
	    	continue;
	    

	    // Q[20] = 0... .... .... .... .... .... .... ....
	    if ( bit(Q[20],32) )
	    	continue;

	    // Q[21] = 0... .... .... ..^. .... .... .... ....
	    if ( bit(Q[21],32) || bit(Q[21],18) != bit(Q[20],18) )
	    	continue;

	    // Q[22] = 0... .... .... .... .... .... .... ....
	    if ( bit(Q[22],32) )
	    	continue;

	    // Q[23] = 0... .... .... .... .... .... .... ....
	    if ( bit(Q[23],32) )
	    	continue;

	    // Q[24] = 0... .... .... .... .... .... .... ....
	    if ( !bit(Q[24],32) )
	    	continue;

	    // No conditions for Q[25]...Q[47]



	    printf("Found ya\n");
	    return 0;

	}
}

int main (void){
	block();
	uint32_t b = 0x80000002;
	uint32_t a = bit(b,31);
	int c;
	c = (a == 0);
	printf("%d\n",c);
	printf("%2.32x\n", a);
	return 0;
}
