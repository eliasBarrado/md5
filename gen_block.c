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

#define DEBUG 1

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


void print_bin(uint32_t hex){
	int bin[32];
	for (int i=32; i>0; i--) {

		if ( (hex & mask_bit[i]) == mask_bit[i] ) {
			bin[i-1] = 1;

		} else {
			bin[i-1] = 0;
		}
	}
	int k =0;
	for (int i=32; i>0; i--) {
		printf("%d", bin[i-1]);
		k++;
		if(k%4 == 0 && k!=0){
			printf(" ");
		}
		

	}
	printf("\n");
}


//uint32_t X;
uint32_t seed32_1, seed32_2; 
uint32_t frandom() {
  //X = (1103515245*X + 12345) & 0xffffffff;
  //return X;
  uint32_t t = seed32_1 ^ (seed32_1 << 10);
  seed32_1 = seed32_2;
  seed32_2 = (seed32_2^(seed32_2>>10))^(t^(t>>13));
  return seed32_1;
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

	uint32_t Q[65], Q1[61], m[16], m1[16], QM0, QM1, QM2, QM3;


	//Initialization vectors
  	QM3 = IV1;  QM0 = IV2;
  	QM1 = IV3;  QM2 = IV4;

	// Hex table
	// 0 = 0000  1 = 0001  2 = 0010  3 = 0011
	// 4 = 0100  5 = 0101  6 = 0110  7 = 0111
	// 8 = 1000  9 = 1001  A = 1010  B = 1011
	// C = 1100  D = 1101  E = 1110  F = 1111

  	uint64_t i = 0;
	while(1){

		
		// Q[1]  = .... .... .... .... .... .... .... .... 
	    Q [1]  = frandom();
	    Q1[1]  = Q[1];
	    	    
	    /*
	    if(i%1000000 == 0){
			printf("%ld\n", i);
			//printf("%08x\n",Q[1]);
		}
		*/
		i = i + 1;

	    // Q[2]  = .... .... .... .... .... .... .... .... 
	    Q [2]  = frandom();
	    Q1[2]  = Q[2];

	    // Q[3]  = .... .... .... 0... .... 0... .... ....
	    Q [3]  = frandom() & 0xfff7f7ff;
	    Q1[3]  = Q[3];

	    // Q[4]  = 1... .... 0^^^ 1^^^ ^^^^ 1^^^ ^... .... 
	    Q [4]  = (frandom() & 0x7f00007f) | 0x80080800 | (Q[3] & 0x0077f780);
	    Q1[4]  = Q[4];

	    //  Q[5]  = 1... 1.0. 01.. 0000 0000 0000 001. .1.1
	    // ∆Q[5]  = 0000 0000 0-++ ++++ ++++ ++++ ++00 0000
	    Q [5]  = (frandom() & 0x7530001a) | 0x88400025;
	    Q1[5]  = Q[5] ^ 0x007fffc0;

	    //  Q[6]  = 0000 001^ 0111 1111 1011 1100 0100 ^0^1
	    // ∆Q[6]  = +000 0000 +000 0000 0000 0000 0-00 0000
	    Q [6]  = 0x027fbc41 | (Q[5] & 0x0100000a);
	    Q1[6]  = Q[6] ^ 0x80800040;

	    /*
	    if( (F(Q[ 6], Q[ 5], Q[ 4]) ^ F(Q1[ 6], Q1[ 5], Q1[ 4])) != 0x00004400 ){
			continue;
		}
		*/


	    //  Q[7]  = 0000 0011 1111 1110 1111 1000 0010 0000 
	    // ∆Q[7]  = ++++ ++-- -000 0000 0000 -+++ ++-+ ++++
	    Q [7] = 0x03fef820;
	    Q1[7] = Q[7] ^ 0xff800fff;

	    //  Q[8]  = 0000 0001 1..1 0001 0.0. 0101 0100 0000 
	    // ∆Q[8]  = 0000 0000 -00- +++- +000 0000 0000 000+	
	    Q [8]  = (frandom() & 0x00605000) | 0x01910540;
	    Q1[8]  = Q[8] ^ 0x009f8001;

	    //  Q[9]  = 1111 1011 ...1 0000 0.1^ 1111 0011 1101 
	    // ∆Q[9]  = -000 0000 0000 0000 0000 000- ++00 00+-
	    Q [9]  = (frandom() & 0x00e04000) | 0xfb102f3d | (Q[8] & 0x00001000);
	    Q1[9] = Q[9] ^ 0x800001c3;

	    //  Q[10] = 01.. .... 0..1 1111 1101 ...0 01.. ..00 
	    // ∆Q[10] = +000 0000 0000 0000 00+- 0000 0000 0000
	    Q [10] = (frandom() & 0x3f600e3c) | 0x401fd040;
	    Q1[10] = Q[10] ^ 0x10003000;

	    //  Q[11] = 00.. .... .... 0001 1100 ...0 11.. ..10 
	    // ∆Q[11] = ++00 0000 0000 0000 0000 0000 0000 0000
	    Q [11] = (frandom() & 0x3ff00e3c) | 0x0001c0c2;
	    Q1[11] = Q[11] ^ 0xc0000000;

	    //  Q[12] = 00.. ..^^ .... 1000 0001 ...1 0... .... 
	    // ∆Q[12] = +000 0000 0000 -+++ +++0 000- +000 0000
	    Q [12] = (frandom() & 0x3cf00e7f) | 0x00081100 | (Q[11] & 0x03000000);
	    Q1[12] = Q[12] ^ 0x800fe180;

	    //  Q[13] = 01.. ..01 .... 1111 111. ...0 0... 1... 
	    // ∆Q[13] = +000 00+- 0000 0000 0000 0000 0000 0000
	    Q [13] = (frandom() & 0x3cf01e77) | 0x410fe008;
	    Q1[13] = Q[13] ^ 0x83000000;

	    //  Q[14] = 0.0. ..00 .... 1011 111. ...1 1... 1... 
	    // ∆Q[14] = +000 0000 0000 0000 0000 0000 0000 0000
	    Q [14] = (frandom() & 0x5cf01e77) | 0x000be188;
	    Q1[14] = Q[14] ^ 0xf0000000;

	    //  Q[15] = 0.1. ..01 .... .... 1... .... .... 0... 
	    // ∆Q[15] = +000 0000 0000 0000 -000 0000 0000 +000
	    Q [15] = (frandom() & 0x5cff7ff7) | 0x21008000;
	    Q1[15] = Q[15] ^ 0x80008008;

	    //  Q[16] = 0.1. .... .... .... .... .... .... .... 
	    // ∆Q[16] = +0-0 0000 0000 0000 0000 0000 0000 0000
	    Q [16] = (frandom() & 0x5fffffff) | 0x20000000;
	    Q1[16] = Q[16] ^ 0xa0000000;

		//Compute first block 
	    m[ 0] = RR(Q[ 1] - QM0  ,  7) - F(QM0  , QM1  , QM2  ) - QM3   - 0xd76aa478;  
	    m[ 1] = RR(Q[ 2] - Q[ 1], 12) - F(Q[ 1], QM0  , QM1  ) - QM2   - 0xe8c7b756;
	    m[ 2] = RR(Q[ 3] - Q[ 2], 17) - F(Q[ 2], Q[ 1], QM0  ) - QM1   - 0x242070db;
	    m[ 3] = RR(Q[ 4] - Q[ 3], 22) - F(Q[ 3], Q[ 2], Q[ 1]) - QM0   - 0xc1bdceee;
	    m[ 4] = RR(Q[ 5] - Q[ 4],  7) - F(Q[ 4], Q[ 3], Q[ 2]) - Q[ 1] - 0xf57c0faf;
	    m[ 5] = RR(Q[ 6] - Q[ 5], 12) - F(Q[ 5], Q[ 4], Q[ 3]) - Q[ 2] - 0x4787c62a;
	    m[ 6] = RR(Q[ 7] - Q[ 6], 17) - F(Q[ 6], Q[ 5], Q[ 4]) - Q[ 3] - 0xa8304613;
	    m[ 7] = RR(Q[ 8] - Q[ 7], 22) - F(Q[ 7], Q[ 6], Q[ 5]) - Q[ 4] - 0xfd469501;
	    m[ 8] = RR(Q[ 9] - Q[ 8],  7) - F(Q[ 8], Q[ 7], Q[ 6]) - Q[ 5] - 0x698098d8;
	    m[ 9] = RR(Q[10] - Q[ 9], 12) - F(Q[ 9], Q[ 8], Q[ 7]) - Q[ 6] - 0x8b44f7af;
	    m[10] = RR(Q[11] - Q[10], 17) - F(Q[10], Q[ 9], Q[ 8]) - Q[ 7] - 0xffff5bb1; 
	    m[11] = RR(Q[12] - Q[11], 22) - F(Q[11], Q[10], Q[ 9]) - Q[ 8] - 0x895cd7be;
	    m[12] = RR(Q[13] - Q[12],  7) - F(Q[12], Q[11], Q[10]) - Q[ 9] - 0x6b901122;
	    m[13] = RR(Q[14] - Q[13], 12) - F(Q[13], Q[12], Q[11]) - Q[10] - 0xfd987193;
	    m[14] = RR(Q[15] - Q[14], 17) - F(Q[14], Q[13], Q[12]) - Q[11] - 0xa679438e;
	    m[15] = RR(Q[16] - Q[15], 22) - F(Q[15], Q[14], Q[13]) - Q[12] - 0x49b40821; 

	    //Compute second block 
	    m1[ 0] = RR(Q1[ 1] - QM0   ,  7) - F(QM0   , QM1   , QM2  )  - QM3    - 0xd76aa478;  
	    m1[ 1] = RR(Q1[ 2] - Q1[ 1], 12) - F(Q1[ 1], QM0   , QM1  )  - QM2    - 0xe8c7b756;
	    m1[ 2] = RR(Q1[ 3] - Q1[ 2], 17) - F(Q1[ 2], Q1[ 1], QM0  )  - QM1    - 0x242070db;
	    m1[ 3] = RR(Q1[ 4] - Q1[ 3], 22) - F(Q1[ 3], Q1[ 2], Q1[ 1]) - QM0    - 0xc1bdceee;
	    m1[ 4] = RR(Q1[ 5] - Q1[ 4],  7) - F(Q1[ 4], Q1[ 3], Q1[ 2]) - Q1[ 1] - 0xf57c0faf;
	    m1[ 5] = RR(Q1[ 6] - Q1[ 5], 12) - F(Q1[ 5], Q1[ 4], Q1[ 3]) - Q1[ 2] - 0x4787c62a;
	    m1[ 6] = RR(Q1[ 7] - Q1[ 6], 17) - F(Q1[ 6], Q1[ 5], Q1[ 4]) - Q1[ 3] - 0xa8304613;
	    m1[ 7] = RR(Q1[ 8] - Q1[ 7], 22) - F(Q1[ 7], Q1[ 6], Q1[ 5]) - Q1[ 4] - 0xfd469501;
	    m1[ 8] = RR(Q1[ 9] - Q1[ 8],  7) - F(Q1[ 8], Q1[ 7], Q1[ 6]) - Q1[ 5] - 0x698098d8;
	    m1[ 9] = RR(Q1[10] - Q1[ 9], 12) - F(Q1[ 9], Q1[ 8], Q1[ 7]) - Q1[ 6] - 0x8b44f7af;
	    m1[10] = RR(Q1[11] - Q1[10], 17) - F(Q1[10], Q1[ 9], Q1[ 8]) - Q1[ 7] - 0xffff5bb1; 
	    m1[11] = RR(Q1[12] - Q1[11], 22) - F(Q1[11], Q1[10], Q1[ 9]) - Q1[ 8] - 0x895cd7be;
	    m1[12] = RR(Q1[13] - Q1[12],  7) - F(Q1[12], Q1[11], Q1[10]) - Q1[ 9] - 0x6b901122;
	    m1[13] = RR(Q1[14] - Q1[13], 12) - F(Q1[13], Q1[12], Q1[11]) - Q1[10] - 0xfd987193;
	    m1[14] = RR(Q1[15] - Q1[14], 17) - F(Q1[14], Q1[13], Q1[12]) - Q1[11] - 0xa679438e;
	    m1[15] = RR(Q1[16] - Q1[15], 22) - F(Q1[15], Q1[14], Q1[13]) - Q1[12] - 0x49b40821; 

	    if(DEBUG){
	    	for(int i = 1; i < 16; i++){
	    	print_bin(Q[i]);
	    	print_bin(Q1[i]);
	    	printf("\n");
	    	}
		}

		return 0;

	    if( (m[4] - m1[4]) != 0x80000000)
	    	continue;
	    
	    if( (m[5] - m1[5]) != 0x00000000)
	    	continue;

	    if( (m[6] - m1[6]) != 0x00000000)
	    	continue;
	    
	    if( (m[7] - m1[7]) != 0x00000000)
	    	continue;

	    if( (m[8] - m1[8]) != 0x00000000)
	    	continue;
	    
	    if( (m[9] - m1[9]) != 0x00000000)
	    	printf("%08x\n"  , ~(m[9] - m1[9]));
	    	printf("%ld\n",i);
	    	//return 0;
	    	continue;
	    /*
	    if( (m[10] ^ m1[10]) != 0x00000000)
	    	continue;
		*/


	    

	    uint32_t temp;
	    for( int k = 3; k<9 ; k++){
	    	temp = F(Q1[k], Q1[k-1], Q1[k-2]) - F(Q[k], Q[k-1], Q[k-2]);
	    	printf("%2.2d %08x\n", k, temp);
	    }


	    
	    for(int j = 0 ; j<16; j++){
	    	printf("%2.2i %08x\n"  , j, m [j]);
	    	printf("%2.2i %08x\n\n", j, m1[j]);
	    }
		
	    return 0;

	    // Q[17] = 0... .... .... ..0. ^... .... .... ^... 
	    Q[17] = GG(Q[13], Q[16], Q[15], Q[14], m[ 1],  5, 0xf61e2562);
	    if ( bit(Q[17],32) || bit(Q[17],18) || bit(Q[17],16) != bit(Q[16],16) || bit(Q[17],4) != bit(Q[16],4) )
	    	continue;
	    


		// Q[18] = 0.^. .... .... ..1. .... .... .... ....
	    Q[18] = GG(Q[14], Q[17], Q[16], Q[15], m[ 6],  9, 0xc040b340);
	    if ( bit(Q[18],32) || bit(Q[18],30) != bit(Q[17],30) || !bit(Q[18],18) )
	    	continue;
	    

	    
	    // Q[19] = 0... .... .... ..0. .... .... .... ....
	    Q[19] = GG(Q[15], Q[18], Q[17], Q[16], m[11], 14, 0x265e5a51);
	    if ( bit(Q[19],32) || bit(Q[19],18) )
	    	continue;
	    

	    // Q[20] = 0... .... .... .... .... .... .... ....
	    Q[20] = GG(Q[16], Q[19], Q[18], Q[17], m[ 0], 20, 0xe9b6c7aa); 
	    if ( bit(Q[20],32) )
	    	continue;

	    // Q[21] = 0... .... .... ..^. .... .... .... ....
	    Q[21] = GG(Q[17], Q[20], Q[19], Q[18], m[ 5],  5, 0xd62f105d); 
	    if ( bit(Q[21],32) || bit(Q[21],18) != bit(Q[20],18) )
	    	continue;

	    // Q[22] = 0... .... .... .... .... .... .... ....
	    Q[22] = GG(Q[18], Q[21], Q[20], Q[19], m[10],  9, 0x2441453); 
	    if ( bit(Q[22],32) )
	    	continue;

	    // Q[23] = 0... .... .... .... .... .... .... ....
	    Q[23] = GG(Q[19], Q[22], Q[21], Q[20], m[15], 14, 0xd8a1e681); 
	    if ( bit(Q[23],32) )
	    	continue;

	    // Q[24] = 0... .... .... .... .... .... .... ....
	    Q[24] = GG(Q[20], Q[23], Q[22], Q[21], m[ 4], 20, 0xe7d3fbc8); 
	    if ( !bit(Q[24],32) )
	    	continue;

	    // No conditions for Q[25]...Q[47




	    printf("Found ya\n");
	    return 0;

	}
}

int main (void){
	srand ( time(NULL) );
	//X = rand();
	seed32_1 = rand();
	seed32_2 = rand() % 1492637;
	block();
	uint32_t b = 0x80000002;
	uint32_t a = bit(b,31);
	int c;
	c = (a == 0);
	return 0;
}

