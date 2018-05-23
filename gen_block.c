#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <unistd.h>


#define RR(x, n) (((x) >> (n)) | ((x) << (32-(n))))
#define RL(x, n) (((x) << (n)) | ((x) >> (32-(n))))

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define DEBUG 1



// Round functions F,G,H,I of the md5 compression function
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


// Prints binary representation of uint_32_t
// Only used for debug
void print_bin(uint32_t hex){
	int bin[32];
	for (int i=32; i>0; i--) {

		if ( (hex & mask_bit[i]) == mask_bit[i] ) {
			bin[i-1] = 1;

		} else {
			bin[i-1] = 0;
		}
	}
	int k = 0;
	for (int i=32; i>0; i--) {
		printf("%d", bin[i-1]);
		k++;
		// adds space between 4 consecutive digits 
		if(k%4 == 0 && k!=0)
			printf(" ");
	}
	printf("\n");
}


// Prints ∆Q[i] = (X'[i] - X[i]) for i = 0.....31
// Only used for debug
void printf_bsdr(uint32_t a, uint32_t b){
	int a_bin[32], b_bin[32];
	for (int i=32; i>0; i--) {
		if ( (a & mask_bit[i]) == mask_bit[i] ) {
			a_bin[i-1] = 1;

		} else {
			a_bin[i-1] = 0;
		}

		if ( (b & mask_bit[i]) == mask_bit[i] ) {
			b_bin[i-1] = 1;

		} else {
			b_bin[i-1] = 0;
		}
	}

	for (int i=0; i<32; i++){
		if( a_bin[i] - b_bin[i] == 1 ){
			printf("%3d ",i);
		}

		if( a_bin[i] - b_bin[i] == -1 ){
			if(i==0){
				printf(" -0 ");
			}else{
				printf("%3.d ", -i);
			}
		}
	}
	printf("\n");
}

// Random number generator
uint32_t seed32_1, seed32_2; 
uint32_t frandom() {
  uint32_t t = seed32_1 ^ (seed32_1 << 10);
  seed32_1 = seed32_2;
  seed32_2 = (seed32_2^(seed32_2>>10))^(t^(t>>13));
  return seed32_1;
}

//Returns the b-th bit of a 
//	For example if a = 0x4000001 
//		bit(a, 1) returns 1
//		bit(a,31) returns 1
//Provide a valid b ==> 0 < b < 33			
uint32_t bit(uint32_t a, uint32_t b) {
      return (a & mask_bit[b]) >> (b-1);
}


// Write block to disk - Returns 0 if ok, 1 if cannot open, 2 if write error
int write_block(char *fname,void *block) {
  
  FILE *f;
  size_t wrt;

  f = fopen(fname,"wb");
  if (f == NULL) 
    return 1;
  
  wrt = fwrite(block,1,64,f);
  
  fclose(f);
  
  if (wrt == 64) 
    return 0;
  
  return 2;

}


//Default init hash values
uint32_t IV1=0x67452301; 
uint32_t IV2=0xefcdab89;
uint32_t IV3=0x98badcfe; 
uint32_t IV4=0x10325476;

//Default init hash values
uint32_t IV1_1=0x67452301; 
uint32_t IV2_1=0xefcdab89;
uint32_t IV3_1=0x98badcfe; 
uint32_t IV4_1=0x10325476;


int block(void){

	uint32_t Q[65], Q1[65], m[16], m1[16], QM0, QM1, QM2, QM3, QM0_1, QM1_1, QM2_1, QM3_1;


	//Initialization 
  	QM3 = IV1;  QM0 = IV2;
  	QM1 = IV3;  QM2 = IV4;

	// Hex table
	// 0 = 0000  1 = 0001  2 = 0010  3 = 0011
	// 4 = 0100  5 = 0101  6 = 0110  7 = 0111
	// 8 = 1000  9 = 1001  A = 1010  B = 1011
	// C = 1100  D = 1101  E = 1110  F = 1111

  	uint64_t i = 1;
  	uint64_t j = 0;

  	int round17 = 0;
  	int round25 = 0;
  	int round48 = 0;
  	int round61 = 0;

  	int cont    = 0;

	while(1){



		if(i%1000000 == 0){
			printf("\n");
			printf("Total iterations: %ld\n", i );
			printf("Reached round 17: %d\n" , round17);
			printf("Reached round 25: %d\n" , round25);
			printf("Reach round48:    %d\n" , round48);
			printf("Reach round61:    %d\n" , round61);
			printf("Reach round17/total percent:   %8.8lf%%\n\n" , (double)(round17)/i*100);
			printf("Reach round25/total percent:   %8.8lf%%\n\n" , (double)(round25)/i*100);
			printf("Reach round48/total percent:   %8.8lf%%\n\n" , (double)(round48)/i*100);
			printf("Reach round61/total percent:   %8.8lf%%\n\n" , (double)(round61)/i*100);
			return 0;
		}
		i++;



		//  Q[i] is contructed as shown in the following example:
		//  	
		// 	Conditions:
		//				. ---> no condition on Q[i]
		//				0 ---> Q[i] must be 0
		//				1 ---> Q[i] must be 1
		//				^ ---> Q[i] must be equal to Q[i-1]
		//
		//	If we have bitcondition 0000 .... 0000 0000 1111 1111 1111 ^^^^
		// 
		//  	1. First Q[i] is generated at random
		//		2. Set to 0 all bits of Q[i] with bitcondition different than .
		//			In the example Q[i] = Q[i] & 0x0f000000              ----> 0000 .... 0000 0000 0000 0000 0000 0000
		//		3. Add the 1's
		//			In the example Q[i] = Q[i] | 0x0000fff0              ----> 0000 .... 0000 0000 1111 1111 1111 0000
		//		4. Add the required bits of Q[i-1]
		// 			In the example Q[i] = Q[i] | ( Q[i-1] & 0x0000000f)  ----> 0000 .... 0000 0000 1111 1111 1111 ^^^^
		//		
		//
		// Q1[i] is constructed satisfying 	∆Q[i] = Q1[i] - Q[i] ==> Q1 = Q ^ ∆Q
		

	    // Q[3]  = .... .... .... 0... .... 0... .0.. ....
	    Q [3]  = frandom() & 0xfff7f7bf;
	    Q1[3]  = Q[3];

	    // Q[4]  = 1... .... 0^^^ 1^^^ ^^^^ 1^^^ ^011 .... 
	    Q [4]  = (frandom() & 0x7f00000f) | 0x80080830 | (Q[3] & 0x0077f780);
	    Q1[4]  = Q[4];

	    //  Q[5]  = 1000 100. 0100 0000 0000 0000 0010 .1.1   bits 25 26 different
	    // ∆Q[5]  = 0000 0000 0-++ ++++ ++++ ++++ ++00 0000
	    Q [5]  = (frandom() & 0x0100000a) | 0x88400025;
	    Q1[5]  = Q[5] ^ 0x007fffc0;

	    //  Q[6]  = 0000 001^ 0111 1111 1011 1100 0100 ^0^1
	    // ∆Q[6]  = +000 0000 +000 0000 0000 0000 0-00 0000
	    Q [6]  = 0x027fbc41 | (Q[5] & 0x0100000a);
	    Q1[6]  = Q[6] ^ 0x80800040;

	    //  Q[7]  = 0000 0011 1111 1110 1111 1000 0010 0000 
	    // ∆Q[7]  = ++++ ++-- -000 0000 0000 -+++ ++-+ ++++
	    Q [7] = 0x03fef820;
	    Q1[7] = Q[7] ^ 0xff800fff;

	    m [ 6] = RR(Q [ 7] - Q [ 6], 17) - F(Q [ 6], Q [ 5], Q [ 4]) - Q [ 3] - 0xa8304613;
	    m1[ 6] = RR(Q1[ 7] - Q1[ 6], 17) - F(Q1[ 6], Q1[ 5], Q1[ 4]) - Q1[ 3] - 0xa8304613;

	    if( m1[ 6] != m[ 6] )
	    	continue;

	    //  Q[8]  = 0000 0001 1..1 0001 0.0. 0101 0100 0000 
	    // ∆Q[8]  = 0000 0000 -00- +++- +000 0000 0000 000+	
	    Q [8]  = (frandom() & 0x00605000) | 0x01910540;
	    Q1[8]  = Q[8] ^ 0x009f8001;

	    m [ 7] = RR(Q [ 8] - Q [ 7], 22) - F(Q [ 7], Q [ 6], Q [ 5]) - Q [ 4] - 0xfd469501;
	    m1[ 7] = RR(Q1[ 8] - Q1[ 7], 22) - F(Q1[ 7], Q1[ 6], Q1[ 5]) - Q1[ 4] - 0xfd469501;

	    if( m1[ 7] != m[ 7] )
	    	continue;

	    //  Q[9]  = 1111 1011 ...1 0000 0.1^ 1111 0011 1101 
	    // ∆Q[9]  = -000 0000 0000 0000 0000 000- ++00 00+-
	    Q [9]  = (frandom() & 0x00e04000) | 0xfb102f3d | (Q[8] & 0x00001000);
	    Q1[9] = Q[9] ^ 0x800001c3;

	    m [ 8] = RR(Q [ 9] - Q [ 8],  7) - F(Q [ 8], Q [ 7], Q [ 6]) - Q [ 5] - 0x698098d8;
	    m1[ 8] = RR(Q1[ 9] - Q1[ 8],  7) - F(Q1[ 8], Q1[ 7], Q1[ 6]) - Q1[ 5] - 0x698098d8;

	    if( m1[8] != m[8] )
	    	continue;

	    //  Q[10] = 0111 .... 0..1 1111 1101 ...0 01.. ..00 
	    // ∆Q[10] = +000 0000 0000 0000 00+- 0000 0000 0000
	    Q [10] = (frandom() & 0x0f600e3c) | 0x701fd040;
	    Q1[10] = Q[10] ^ 0x80003000;

	    m [ 9] = RR(Q [10] - Q [ 9], 12) - F(Q [ 9], Q [ 8], Q [ 7]) - Q [ 6] - 0x8b44f7af;
	    m1[ 9] = RR(Q1[10] - Q1[ 9], 12) - F(Q1[ 9], Q1[ 8], Q1[ 7]) - Q1[ 6] - 0x8b44f7af;

	    if( m1[9] != m[9] )
	    	continue;

	    //  Q[11] = 0010 0000 1... 0001 1100 0000 1100 0010 
	    // ∆Q[11] = ++00 0000 0000 0000 0000 0000 0000 0000
	    Q [11] = (frandom() & 0x00700000) | 0x2081c0c2;
	    Q1[11] = Q[11] ^ 0xc0000000;

	    m [10] = RR(Q [11] - Q [10], 17) - F(Q [10], Q [ 9], Q [ 8]) - Q [ 7] - 0xffff5bb1;
	    m1[10] = RR(Q1[11] - Q1[10], 17) - F(Q1[10], Q1[ 9], Q1[ 8]) - Q1[ 7] - 0xffff5bb1; 

	    if( (m1[10] != m[10]) )
	    	continue;

	    //  Q[12] = 000. ..^^ .... 1000 0001 ...1 0... ....  bits 25 26 different from paper
	    // ∆Q[12] = +000 0000 0000 -+++ +++0 000- +000 0000
	    Q [12] = (frandom() & 0x1cf00e7f) | 0x00081100 | (Q[11] & 0x03000000);
	    Q1[12] = Q[12] ^ 0x800fe180;

	    m [11] = RR(Q [12] - Q [11], 22) - F(Q [11], Q [10], Q [ 9]) - Q [ 8] - 0x895cd7be;
	    m1[11] = RR(Q1[12] - Q1[11], 22) - F(Q1[11], Q1[10], Q1[ 9]) - Q1[ 8] - 0x895cd7be;

	    if( (m1[11] - m[11]) != 0x00008000)
	    	continue;

	    //  Q[13] = 01.. ..01 .... 1111 111. ...0 0... 1... 
	    // ∆Q[13] = +000 00+- 0000 0000 0000 0000 0000 0000
	    Q [13] = (frandom() & 0x3cf01e77) | 0x410fe008;
	    Q1[13] = Q[13] ^ 0x83000000;

	    m [12] = RR(Q [13] - Q [12],  7) - F(Q [12], Q [11], Q [10]) - Q [ 9] - 0x6b901122;
	    m1[12] = RR(Q1[13] - Q1[12],  7) - F(Q1[12], Q1[11], Q1[10]) - Q1[ 9] - 0x6b901122;

	    if( m1[12] != m[12] )
	    	continue;

	    //  Q[14] = 0.0. ..00 .... 1011 111. ...1 1... 1... 
	    // ∆Q[14] = +000 0000 0000 0000 0000 0000 0000 0000
	    Q [14] = (frandom() & 0x5cf01e77) | 0x000be188;
	    Q1[14] = Q[14] ^ 0x80000000;

	    m [13] = RR(Q [14] - Q [13], 12) - F(Q [13], Q [12], Q [11]) - Q [10] - 0xfd987193;
	    m1[13] = RR(Q1[14] - Q1[13], 12) - F(Q1[13], Q1[12], Q1[11]) - Q1[10] - 0xfd987193;

	    if( m1[13] != m[13] )
	    	continue;

	    //  Q[15] = 0.1. ..01 .... ...0 1... .... .... 0... 
	    // ∆Q[15] = +000 0000 0000 0000 -000 0000 0000 +000
	    Q [15] = (frandom() & 0x5cfe7ff7) | 0x21008000;
	    Q1[15] = Q[15] ^ 0x80008008;

	    m [14] = RR(Q [15] - Q [14], 17) - F(Q [14], Q [13], Q [12]) - Q [11] - 0xa679438e;
	    m1[14] = RR(Q1[15] - Q1[14], 17) - F(Q1[14], Q1[13], Q1[12]) - Q1[11] - 0xa679438e;

	    if( (m1[14] - m[14]) != 0x80000000)
	    	continue;

	    //  Q[16] = 0!1. .... .... ..!. .... .... .... .... 
	    // ∆Q[16] = +0-0 0000 0000 0000 0000 0000 0000 0000
	    if( bit(Q[15], 31) ){
	    	if( bit(Q[15],18) ){
	    		Q [16] = ((frandom() & 0x1fffffff) | 0x20000000);
	    	}else{
	    		Q [16] = ((frandom() & 0x1fffffff) | 0x20020000);
	    	}
	    }else{
	    	if( bit(Q[15],18) ){
	    		Q [16] = ((frandom() & 0x1fffffff) | 0x60000000);
	    	}else{
	    		Q [16] = ((frandom() & 0x1fffffff) | 0x60020000);
	    	}
	    } 
	    Q1[16] = Q[16] ^ 0xa0000000;

	    m [15] = RR(Q [16] - Q [15], 22) - F(Q [15], Q [14], Q [13]) - Q [12] - 0x49b40821; 
	    m1[15] = RR(Q1[16] - Q1[15], 22) - F(Q1[15], Q1[14], Q1[13]) - Q1[12] - 0x49b40821; 

	    if( (m1[15] - m[15]) != 0x00000000)
	    	continue;

	    
	    round17++;

	    // For each Q[i]:
	    // 		the first  'if' checks the bitconditions
	    //		the second 'if' checks the differential path
	
		for(int k = 0; k < 128; k++){

			// Q[17] = 0!.. .... .... ..0. ^... .... .... ^...
			if( bit(Q[16],31)){ 
		    	Q [17] = (frandom() & 0x3ffd7ff7) | ( Q[16] & 0x00008008);
		    }else{
		    	Q [17] = (frandom() & 0x3ffd7ff7) | ( Q[16] & 0x00008008) | 0x40000000;
		    }
		    Q1[17] = Q[17] ^ 0x80000000;

		    m [ 1] = RR(Q [17] - Q [16], 5) - G(Q [16], Q [15], Q [14]) - Q [13] - 0xf61e2562;
			m1[ 1] = RR(Q1[17] - Q1[16], 5) - G(Q1[16], Q1[15], Q1[14]) - Q1[13] - 0xf61e2562;

			if( m1[1] != m[1] ) {
				i++;
				continue;
			}

		    // Q[18] = 0.^. .... .... ..1. .... .... .... ....
		    Q [18] = GG(Q [14], Q [17], Q [16], Q [15], m [ 6],  9, 0xc040b340);
		    Q1[18] = GG(Q1[14], Q1[17], Q1[16], Q1[15], m1[ 6],  9, 0xc040b340);

		    if ( bit(Q[18],32) || bit(Q[18],30) != bit(Q[17],30) || !bit(Q[18],18) ){
		    	i++;
		    	continue;
		    }

		    if ( (Q[18] ^ Q1[18]) != 0x80000000 ) {
		    	i++;
		    	continue;
		    }

		    // Q[19] = 0... .... .... ..0. .... .... .... ....
		    Q [19] = GG(Q [15], Q [18], Q [17], Q [16], m [11], 14, 0x265e5a51);
		    Q1[19] = GG(Q1[15], Q1[18], Q1[17], Q1[16], m1[11], 14, 0x265e5a51);

		    if ( bit(Q[19],32) || bit(Q[19],18) ) {
		    	i++;
		    	continue;
		    }

		    if ( (Q[19] ^ Q1[19]) != 0x80020000 ) {
		    	i++;
		    	continue;
		    } 
		    	

		    // Q[20] = 0... .... .... .!.. .... .... .... ....
		    if( bit(Q[19],19) ){
		    	Q [20] = frandom() & 0x7ffbffff;
		    }else{
		    	Q [20] = (frandom() & 0x7ffbffff) | 0x00040000;
		    }
		    Q1[20] = Q[20] ^ 0x80000000;


		    m [ 0] = RR(Q [20] - Q [19], 20) - G(Q [19], Q [18], Q [17]) - Q [16] - 0xe9b6c7aa;
			m1[ 0] = RR(Q1[20] - Q1[19], 20) - G(Q1[19], Q1[18], Q1[17]) - Q1[16] - 0xe9b6c7aa;

			if( m1[0] != m[0] ) {
				i++;
				continue;
			}

			Q [1] = RL( m [ 0] + F(QM0  , QM1  , QM2  ) + QM3 + 0xd76aa478, 7 ) + QM0;
			Q1[1] = Q[1];

			Q [2] = RL( m [1] + F(Q [ 1], QM0  , QM1  ) + QM2 + 0xe8c7b756, 12) + Q [1];
			Q1[2] = Q[2];

			m [2] = RR(Q [ 3] - Q [ 2], 17) - F(Q [ 2], Q [ 1], QM0   ) - QM1    - 0x242070db;
			m1[2] = m[2];

		    m [3] = RR(Q [ 4] - Q [ 3], 22) - F(Q [ 3], Q [ 2], Q [ 1]) - QM0    - 0xc1bdceee;
		    m1[3] = m[3];

		    m [4] = RR(Q [ 5] - Q [ 4],  7) - F(Q [ 4], Q [ 3], Q [ 2]) - Q [ 1] - 0xf57c0faf;
		    m1[4] = RR(Q1[ 5] - Q1[ 4],  7) - F(Q1[ 4], Q1[ 3], Q1[ 2]) - Q1[ 1] - 0xf57c0faf;

		    if( (m1[ 4] - m[ 4]) != 0x80000000){
		    	i++;
		    	continue;
		    }

		    m [5] = RR(Q [ 6] - Q [ 5], 12) - F(Q [ 5], Q [ 4], Q [ 3]) - Q [ 2] - 0x4787c62a;		    
		    m1[5] = RR(Q1[ 6] - Q1[ 5], 12) - F(Q1[ 5], Q1[ 4], Q1[ 3]) - Q1[ 2] - 0x4787c62a;

		    if( m1[ 5] != m[ 5] ) {
		    	i++;
		    	continue;  
		    }

		    // Q[21] = 0... .... .... ..^. .... .... .... ....
		    Q [21] = GG(Q [17], Q [20], Q [19], Q [18], m [ 5],  5, 0xd62f105d);
		    Q1[21] = GG(Q1[17], Q1[20], Q1[19], Q1[18], m1[ 5],  5, 0xd62f105d);
		    
		    // Message modification
		    if ( bit(Q[21],18) != bit(Q[20],18) ) {
		    	
		    	m [12] = m[12] - 0x01000000;
		    	m1[12] = m[12];

		    	Q [13] = RL( m [12] + F(Q [12], Q [11], Q [10]) + Q [ 9] + 0x6b901122, 7 ) + Q [12];
		    	Q1[13] = RL( m1[12] + F(Q1[12], Q1[11], Q1[10]) + Q1[ 9] + 0x6b901122, 7 ) + Q1[12];

		    	m [13] = RR(Q [14] - Q [13], 12) - F(Q [13], Q [12], Q [11]) - Q [10] - 0xfd987193;
		    	m1[13] = RR(Q1[14] - Q1[13], 12) - F(Q1[13], Q1[12], Q1[11]) - Q1[10] - 0xfd987193;

		    	m [14] = RR(Q [15] - Q [14], 17) - F(Q [14], Q [13], Q [12]) - Q [11] - 0xa679438e;
		    	m1[14] = RR(Q1[15] - Q1[14], 17) - F(Q1[14], Q1[13], Q1[12]) - Q1[11] - 0xa679438e;

		    	m [15] = RR(Q [16] - Q [15], 22) - F(Q [15], Q [14], Q [13]) - Q [12] - 0x49b40821;
		    	m1[15] = RR(Q1[16] - Q1[15], 22) - F(Q1[15], Q1[14], Q1[13]) - Q1[12] - 0x49b40821;

		    	m [ 1] = m[1] + 0x00000001;
		    	m1[ 1] = m[1];

		    	Q [ 2] = RL(m [1] + F(Q [ 1], QM0  , QM1  ) + QM2 + 0xe8c7b756, 12) + Q [1];
				Q1[ 2] = RL(m1[1] + F(Q1[ 1], QM0  , QM1  ) + QM2 + 0xe8c7b756, 12) + Q1[1];

				m [ 2] = RR(Q [ 3] - Q [ 2], 17) - F(Q [ 2], Q [ 1], QM0   ) - QM1    - 0x242070db;
		    	m [ 3] = RR(Q [ 4] - Q [ 3], 22) - F(Q [ 3], Q [ 2], Q [ 1]) - QM0    - 0xc1bdceee;
		    	m [ 4] = RR(Q [ 5] - Q [ 4],  7) - F(Q [ 4], Q [ 3], Q [ 2]) - Q [ 1] - 0xf57c0faf;

		    	m1[ 2] = RR(Q1[ 3] - Q1[ 2], 17) - F(Q1[ 2], Q1[ 1], QM0  )  - QM1    - 0x242070db;
		    	m1[ 3] = RR(Q1[ 4] - Q1[ 3], 22) - F(Q1[ 3], Q1[ 2], Q1[ 1]) - QM0    - 0xc1bdceee;
		    	m1[ 4] = RR(Q1[ 5] - Q1[ 4],  7) - F(Q1[ 4], Q1[ 3], Q1[ 2]) - Q1[ 1] - 0xf57c0faf;

		    	m [ 5] = m[5] - 0x00000800;
		    	m1[ 5] = m[5];

		    	Q [21] = GG(Q [17], Q [20], Q [19], Q [18], m [ 5],  5, 0xd62f105d);	
		    	Q1[21] = GG(Q1[17], Q1[20], Q1[19], Q1[18], m1[ 5],  5, 0xd62f105d);

		    } 

		    // Message modification
		    if( bit(Q[21],32) ){
		    	
		    	m [3] = m[3] + 0x00000008;
		    	m1[3] = m[3];

		    	Q [ 4] = RL( m [ 3] + F(Q [ 3], Q [ 2], Q [ 1]) + QM0 + 0xc1bdceee, 22 ) + Q [3];
		    	Q1[ 4] = RL( m1[ 3] + F(Q1[ 3], Q1[ 2], Q1[ 1]) + QM0 + 0xc1bdceee, 22 ) + Q1[3];

		    	m [ 5] = m[ 5] - 0x02000000;
		    	m1[ 5] = m[ 5];

		    	m [ 7] = m[ 7] - 0x02000000;
		    	m1[ 7] = m[ 7];

		    	Q [21] = GG(Q [17], Q [20], Q [19], Q [18], m [ 5],  5, 0xd62f105d);	
		    	Q1[21] = GG(Q1[17], Q1[20], Q1[19], Q1[18], m1[ 5],  5, 0xd62f105d);

		    }

		    if ( bit(Q[21],32) || bit(Q[21],18) != bit(Q[20],18) ) {
		    	i++;
		    	continue;
		    }

		    if ( (Q[21] ^ Q1[21]) != 0x80000000 ) {
		    	i++;
		    	continue;
		    }

		    break;

		} 
		

	    // Q[22] = 0... .... .... .... .... .... .... ....
	    Q [22] = GG(Q [18], Q [21], Q [20], Q [19], m [10],  9, 0x2441453);
	    Q1[22] = GG(Q1[18], Q1[21], Q1[20], Q1[19], m1[10],  9, 0x2441453); 

	    if( bit(Q[22],32) ){
	    	
	    	m [8] = m[8] + 0x00004000;
	    	m1[8] = m[8];

	    	Q [9] = RL( m [ 8] + F(Q [ 8], Q [ 7], Q [ 6]) + Q [ 5] + 0x698098d8, 7 ) + Q [8];
	    	Q1[9] = RL( m1[ 8] + F(Q1[ 8], Q1[ 7], Q1[ 6]) + Q1[ 5] + 0x698098d8, 7 ) + Q1[8];

	    	m [10] = m[10] - 0x00200000;
	    	m1[10] = m[10];

	    	m [12] = RR(Q [13] - Q [12],  7) - F(Q [12], Q [11], Q [10]) - Q [ 9] - 0x6b901122;
	    	m1[12] = RR(Q1[13] - Q1[12],  7) - F(Q1[12], Q1[11], Q1[10]) - Q1[ 9] - 0x6b901122;

	    	Q [22] = GG(Q [18], Q [21], Q [20], Q [19], m [10],  9, 0x2441453);
	    	Q1[22] = GG(Q1[18], Q1[21], Q1[20], Q1[19], m1[10],  9, 0x2441453); 

	    } 

	    if ( bit(Q[22],32) )
	    	continue;

	    if ( (Q[22] ^ Q1[22]) != 0x80000000 ) 
	    	continue;

	    // Q[23] = 0... .... .... .... .... .... .... ....
	    Q [23] = GG(Q [19], Q [22], Q [21], Q [20], m [15], 14, 0xd8a1e681);
	    Q1[23] = GG(Q1[19], Q1[22], Q1[21], Q1[20], m1[15], 14, 0xd8a1e681);

	    if ( bit(Q[23],32) ) {

	    		m [10] = m[10] + 0x00000800;
	    		m1[10] = m[10];

	    		m [11] = m [11] - 0x00000040;
	    		m1[11] = m1[11] - 0x00000040;

	    		Q [11] = RL( m [10] + F(Q [10], Q [ 9], Q [ 8]) + Q [ 7] + 0xffff5bb1, 17) + Q [10];
	    		Q1[11] = RL( m1[10] + F(Q1[10], Q1[ 9], Q1[ 8]) + Q1[ 7] + 0xffff5bb1, 17) + Q1[10];

	    		m [12] = RR(Q [13] - Q [12],  7) - F(Q [12], Q [11], Q [10]) - Q [ 9] - 0x6b901122;
	    		m1[12] = RR(Q1[13] - Q1[12],  7) - F(Q1[12], Q1[11], Q1[10]) - Q1[ 9] - 0x6b901122;

	    		m [13] = RR(Q [14] - Q [13], 12) - F(Q [13], Q [12], Q [11]) - Q [10] - 0xfd987193;
	    		m1[13] = RR(Q1[14] - Q1[13], 12) - F(Q1[13], Q1[12], Q1[11]) - Q1[10] - 0xfd987193;

	    		m [14] = m [14] + 0x00200000 - 0x10000000;
	    		m1[14] = m1[14] + 0x00200000 - 0x10000000;

	    		m [15] = m [15] - 0x00000040 -0x00010000;
	    		m1[15] = m1[15] - 0x00000040 -0x00010000;

	    		Q [23] = GG(Q [19], Q [22], Q [21], Q [20], m [15], 14, 0xd8a1e681);
	    		Q1[23] = GG(Q1[19], Q1[22], Q1[21], Q1[20], m1[15], 14, 0xd8a1e681);

		} 

	    if ( bit(Q[23],32) )
	    	continue;

	    if ( (Q[23] ^ Q1[23]) != 0x00000000 ) 
	    	continue;

	    // Q[24] = 1... .... .... .... .... .... .... ....
	    Q [24] = GG(Q [20], Q [23], Q [22], Q [21], m [ 4], 20, 0xe7d3fbc8);
	    Q1[24] = GG(Q1[20], Q1[23], Q1[22], Q1[21], m1[ 4], 20, 0xe7d3fbc8); 
	    
	    if ( !bit(Q[24],32) )
	    	continue;

	    if ( (Q[24] ^ Q1[24]) != 0x00000000 ) 
	    	continue;

	    round25++;

	    // No bitconditions for Q[25]...Q[47]
	    Q [25] = GG(Q [21], Q [24], Q [23], Q [22], m [ 9],  5, 0x21e1cde6);
	    Q1[25] = GG(Q1[21], Q1[24], Q1[23], Q1[22], m1[ 9],  5, 0x21e1cde6);

	    if ( (Q[25] ^ Q1[25]) != 0x00000000 ) 
	    	continue;
	    
	    Q [26] = GG(Q [22], Q [25], Q [24], Q [23], m [14],  9, 0xc33707d6);
	    Q1[26] = GG(Q1[22], Q1[25], Q1[24], Q1[23], m1[14],  9, 0xc33707d6);

	    if ( (Q[26] ^ Q1[26]) != 0x00000000 ) 
	    	continue;

	    Q [27] = GG(Q [23], Q [26], Q [25], Q [24], m [ 3], 14, 0xf4d50d87);
	    Q1[27] = GG(Q1[23], Q1[26], Q1[25], Q1[24], m1[ 3], 14, 0xf4d50d87);

	    if ( (Q[27] ^ Q1[27]) != 0x00000000 ) 
	    	continue;

	    Q [28] = GG(Q [24], Q [27], Q [26], Q [25], m [ 8], 20, 0x455a14ed);
	    Q1[28] = GG(Q1[24], Q1[27], Q1[26], Q1[25], m1[ 8], 20, 0x455a14ed);

	    if ( (Q[28] ^ Q1[28]) != 0x00000000 ) 
	    	continue;

	    Q [29] = GG(Q [25], Q [28], Q [27], Q [26], m [13],  5, 0xa9e3e905);
	    Q1[29] = GG(Q1[25], Q1[28], Q1[27], Q1[26], m1[13],  5, 0xa9e3e905);

	    if ( (Q[29] ^ Q1[29]) != 0x00000000 ) 
	    	continue;

	    Q [30] = GG(Q [26], Q [29], Q [28], Q [27], m [ 2],  9, 0xfcefa3f8);
	    Q1[30] = GG(Q1[26], Q1[29], Q1[28], Q1[27], m1[ 2],  9, 0xfcefa3f8);

	    if ( (Q[30] ^ Q1[30]) != 0x00000000 ) 
	    	continue;

	    Q [31] = GG(Q [27], Q [30], Q [29], Q [28], m [ 7], 14, 0x676f02d9);
	    Q1[31] = GG(Q1[27], Q1[30], Q1[29], Q1[28], m1[ 7], 14, 0x676f02d9);

	    if ( (Q[31] ^ Q1[31]) != 0x00000000 ) 
	    	continue;

	    Q [32] = GG(Q [28], Q [31], Q [30], Q [29], m [12], 20, 0x8d2a4c8a);
	    Q1[32] = GG(Q1[28], Q1[31], Q1[30], Q1[29], m1[12], 20, 0x8d2a4c8a);

	    if ( (Q[32] ^ Q1[32]) != 0x00000000 ) 
	    	continue;


	    Q [33] = HH(Q [29], Q [32], Q [31], Q [30], m [ 5],  4, 0xfffa3942);
	    Q1[33] = HH(Q1[29], Q1[32], Q1[31], Q1[30], m1[ 5],  4, 0xfffa3942);

	    if ( (Q[33] ^ Q1[33]) != 0x00000000 ) 
	    	continue;

	    Q [34] = HH(Q [30], Q [33], Q [32], Q [31], m [ 8], 11, 0x8771f681);
	    Q1[34] = HH(Q1[30], Q1[33], Q1[32], Q1[31], m1[ 8], 11, 0x8771f681);

	    if ( (Q[34] ^ Q1[34]) != 0x00000000 ) 
	    	continue;

	    Q [35] = HH(Q [31], Q [34], Q [33], Q [32], m [11], 16, 0x6d9d6122);
	    Q1[35] = HH(Q1[31], Q1[34], Q1[33], Q1[32], m1[11], 16, 0x6d9d6122);

	    if ( (Q1[35] - Q[35]) != 0x80000000 ) 
	    	continue;

	    Q [36] = HH(Q [32], Q [35], Q [34], Q [33], m [14], 23, 0xfde5380c);
	    Q1[36] = HH(Q1[32], Q1[35], Q1[34], Q1[33], m1[14], 23, 0xfde5380c);

	    if ( (Q1[36] - Q[36]) != 0x80000000 ) 
	    	continue;

	    Q [37] = HH(Q [33], Q [36], Q [35], Q [34], m [ 1],  4, 0xa4beea44);
	    Q1[37] = HH(Q1[33], Q1[36], Q1[35], Q1[34], m1[ 1],  4, 0xa4beea44);

	    if ( (Q1[37] - Q[37]) != 0x80000000 ) 
	    	continue;

	    Q [38] = HH(Q [34], Q [37], Q [36], Q [35], m [ 4], 11, 0x4bdecfa9);
	    Q1[38] = HH(Q1[34], Q1[37], Q1[36], Q1[35], m1[ 4], 11, 0x4bdecfa9);

	    if ( (Q1[38] - Q[38]) != 0x80000000 ) 
	    	continue;

	    Q [39] = HH(Q [35], Q [38], Q [37], Q [36], m [ 7], 16, 0xf6bb4b60);
	    Q1[39] = HH(Q1[35], Q1[38], Q1[37], Q1[36], m1[ 7], 16, 0xf6bb4b60);

	    if ( (Q1[39] - Q[39]) != 0x80000000 ) 
	    	continue;

	    Q [40] = HH(Q [36], Q [39], Q [38], Q [37], m [10], 23, 0xbebfbc70);
	    Q1[40] = HH(Q1[36], Q1[39], Q1[38], Q1[37], m1[10], 23, 0xbebfbc70);
	    
	    if ( (Q1[40] - Q[40]) != 0x80000000 ) 
	    	continue;

	    Q [41] = HH(Q [37], Q [40], Q [39], Q [38], m [13],  4, 0x289b7ec6);
	    Q1[41] = HH(Q1[37], Q1[40], Q1[39], Q1[38], m1[13],  4, 0x289b7ec6);

	    if ( (Q1[41] - Q[41]) != 0x80000000 ) 
	    	continue;

	    Q [42] = HH(Q [38], Q [41], Q [40], Q [39], m [ 0], 11, 0xeaa127fa);
	    Q1[42] = HH(Q1[38], Q1[41], Q1[40], Q1[39], m1[ 0], 11, 0xeaa127fa);

	    if ( (Q1[42] - Q[42]) != 0x80000000 ) 
	    	continue;

	    Q [43] = HH(Q [39], Q [42], Q [41], Q [40], m [ 3], 16, 0xd4ef3085);
	    Q1[43] = HH(Q1[39], Q1[42], Q1[41], Q1[40], m1[ 3], 16, 0xd4ef3085);

	    if ( (Q1[43] - Q[43]) != 0x80000000 ) 
	    	continue;

	    Q [44] = HH(Q [40], Q [43], Q [42], Q [41], m [ 6], 23, 0x4881d05);
	    Q1[44] = HH(Q1[40], Q1[43], Q1[42], Q1[41], m1[ 6], 23, 0x4881d05);

	    if ( (Q1[44] - Q[44]) != 0x80000000 ) 
	    	continue;

	    Q [45] = HH(Q [41], Q [44], Q [43], Q [42], m [ 9],  4, 0xd9d4d039);
	    Q1[45] = HH(Q1[41], Q1[44], Q1[43], Q1[42], m1[ 9],  4, 0xd9d4d039);

	    if ( (Q1[45] - Q[45]) != 0x80000000 ) 
	    	continue;

	    Q [46] = HH(Q [42], Q [45], Q [44], Q [43], m [12], 11, 0xe6db99e5);
	    Q1[46] = HH(Q1[42], Q1[45], Q1[44], Q1[43], m1[12], 11, 0xe6db99e5);

	    if ( (Q1[46] - Q[46]) != 0x80000000 ) 
	    	continue;

	    Q [47] = HH(Q [43], Q [46], Q [45], Q [44], m [15], 16, 0x1fa27cf8);
	    Q1[47] = HH(Q1[43], Q1[46], Q1[45], Q1[44], m1[15], 16, 0x1fa27cf8);

	    if ( (Q1[47] - Q[47]) != 0x80000000 ) 
	    	continue;

	    // Q[48] = m... .... .... .... .... .... .... ....
	    Q [48] = HH(Q [44], Q [47], Q [46], Q [45], m [ 2], 23, 0xc4ac5665);
	    Q1[48] = HH(Q1[44], Q1[47], Q1[46], Q1[45], m1[ 2], 23, 0xc4ac5665);

	    round48++;

	    if ( bit(Q[48],32) != bit(Q[46],32) )
	    	continue;

	    if ( (Q1[48] - Q[48]) != 0x80000000 )
	    	continue;

	    // Q[49] = m... .... .... .... .... .... .... ....
	    Q [49] = II(Q [45], Q [48], Q [47], Q [46], m [ 0],  6, 0xf4292244);
		Q1[49] = II(Q1[45], Q1[48], Q1[47], Q1[46], m1[ 0],  6, 0xf4292244);

		if ( bit(Q[49],32) != bit(Q[47],32) ) 
	    	continue;

		if ( (Q1[49] - Q[49]) != 0x80000000 ) 
	    	continue;

	    // Q[50] = #... .... .... .... .... .... .... ....
		Q [50] = II(Q [46], Q [49], Q [48], Q [47], m [ 7], 10, 0x432aff97);
		Q1[50] = II(Q1[46], Q1[49], Q1[48], Q1[47], m1[ 7], 10, 0x432aff97);

		if ( bit(Q[50],32) == bit(Q[48],32) )
			continue;

		if ( (Q1[50] - Q[50]) != 0x80000000 ) 
	    	continue;

	    // Q[51] = m... .... .... .... .... .... .... ....
		Q [51] = II(Q [47], Q [50], Q [49], Q [48], m [14], 15, 0xab9423a7);
		Q1[51] = II(Q1[47], Q1[50], Q1[49], Q1[48], m1[14], 15, 0xab9423a7);

		if ( bit(Q[51],32) != bit(Q[49],32) ) 
	    	continue;

		if ( (Q1[51] - Q[51]) != 0x80000000 ) 
	    	continue;
		
		// Q[52] = m... .... .... .... .... .... .... ....    
		Q [52] = II(Q [48], Q [51], Q [50], Q [49], m [ 5], 21, 0xfc93a039);
		Q1[52] = II(Q1[48], Q1[51], Q1[50], Q1[49], m1[ 5], 21, 0xfc93a039);

		if ( bit(Q[52],32) != bit(Q[50],32) ) 
	    	continue;

		if ( (Q1[52] - Q[52]) != 0x80000000 ) 
	    	continue;
	    
	    // Q[53] = m... .... .... .... .... .... .... ....
		Q [53] = II(Q [49], Q [52], Q [51], Q [50], m [12],  6, 0x655b59c3);
		Q1[53] = II(Q1[49], Q1[52], Q1[51], Q1[50], m1[12],  6, 0x655b59c3);

		if ( bit(Q[53],32) != bit(Q[51],32) ) 
	    	continue;

		if ( (Q1[53] - Q[53]) != 0x80000000 ) 
	    	continue;

	    // Q[54] = m... .... .... .... .... .... .... ....
		Q [54] = II(Q [50], Q [53], Q [52], Q [51], m [ 3], 10, 0x8f0ccc92);
		Q1[54] = II(Q1[50], Q1[53], Q1[52], Q1[51], m1[ 3], 10, 0x8f0ccc92);

		if ( bit(Q[54],32) != bit(Q[52],32) ) 
	    	continue;

		if ( (Q1[54] - Q[54]) != 0x80000000 ) 
	    	continue;
	    
	    // Q[55] = m... .... .... .... .... .... .... ....
		Q [55] = II(Q [51], Q [54], Q [53], Q [52], m [10], 15, 0xffeff47d);
		Q1[55] = II(Q1[51], Q1[54], Q1[53], Q1[52], m1[10], 15, 0xffeff47d);

		if ( bit(Q[55],32) != bit(Q[53],32) ) 
	    	continue;

		if ( (Q1[55] - Q[55]) != 0x80000000 ) 
	    	continue;

	    // Q[56] = m... .... .... .... .... .... .... ....
		Q [56] = II(Q [52], Q [55], Q [54], Q [53], m [ 1], 21, 0x85845dd1);
		Q1[56] = II(Q1[52], Q1[55], Q1[54], Q1[53], m1[ 1], 21, 0x85845dd1);

		if ( bit(Q[56],32) != bit(Q[54],32) ) 
	    	continue;

		if ( (Q1[56] - Q[56]) != 0x80000000 ) 
	    	continue;

	    // Q[57] = m... .... .... .... .... .... .... ....
		Q [57] = II(Q [53], Q [56], Q [55], Q [54], m [ 8],  6, 0x6fa87e4f);
		Q1[57] = II(Q1[53], Q1[56], Q1[55], Q1[54], m1[ 8],  6, 0x6fa87e4f);

		if ( bit(Q[57],32) != bit(Q[55],32) ) 
	    	continue;

		if ( (Q1[57] - Q[57]) != 0x80000000 ) 
	    	continue;

	    // Q[58] = m... .... .... .... .... .... .... ....
		Q [58] = II(Q [54], Q [57], Q [56], Q [55], m [15], 10, 0xfe2ce6e0);
		Q1[58] = II(Q1[54], Q1[57], Q1[56], Q1[55], m1[15], 10, 0xfe2ce6e0);

		if ( bit(Q[58],32) != bit(Q[56],32) ) 
	    	continue;

		if ( (Q1[58] - Q[58]) != 0x80000000 ) 
	    	continue;

	    // Q[59] = m... .... .... .... .... .... .... ....
		Q [59] = II(Q [55], Q [58], Q [57], Q [56], m [ 6], 15, 0xa3014314);
		Q1[59] = II(Q1[55], Q1[58], Q1[57], Q1[56], m1[ 6], 15, 0xa3014314);

		if ( bit(Q[59],32) != bit(Q[57],32) ) 
	    	continue;

		if ( (Q1[59] - Q[59]) != 0x80000000 ) 
	    	continue;
	    
	    // Q[60] = #... .... .... .... .... .... .... ....
		Q [60] = II(Q [56], Q [59], Q [58], Q [57], m [13],  21, 0x4e0811a1);
		Q1[60] = II(Q1[56], Q1[59], Q1[58], Q1[57], m1[13],  21, 0x4e0811a1);

		if ( bit(Q[60],32) == bit(Q[58],32) )
			continue;

		if ( (Q1[60] - Q[60]) != 0x80000000 ) 
	    	continue;

	    round61++;

	    // Q[61] = m... .... .... .... .... .... .... ....
		Q [61] = II(Q [57], Q [60], Q [59], Q [58], m [ 4],  6, 0xf7537e82);
		Q1[61] = II(Q1[57], Q1[60], Q1[59], Q1[58], m1[ 4],  6, 0xf7537e82);

		QM3_1 = QM3 + Q1[61];
		QM3   = QM3 + Q [61];		

		if ( bit(Q[61],32) != bit(Q[59],32) ) 
	    	continue;

		if ( (Q1[61] - Q[61]) != 0x80000000 ) 
	    	continue;

	    if( (QM3 ^ QM3_1) != 0x80000000 )
			continue;

	    // Q[62] = m... .... .... .... .... .... .... ....
		Q [62] = II(Q [58], Q [61], Q [60], Q [59], m [11], 10, 0xbd3af235);
		Q1[62] = II(Q1[58], Q1[61], Q1[60], Q1[59], m1[11], 10, 0xbd3af235);

		QM2_1 = QM2 + Q1[62];
		QM2   = QM2 + Q [62];

		if( bit(Q[62],32) != bit(Q[60],32) )
			continue;

		if( Q1[62] - Q[62] != 0x82000000 )
			continue;

		if (bit(QM2,26) ) 
        	continue;

		if( (QM2 ^ QM2_1) != 0x82000000 )
			continue;

		// Q[63] = m... .... .... .... .... .... .... ....
		Q [63] = II(Q [59], Q [62], Q [61], Q [60], m [ 2], 15, 0x2ad7d2bb);
		Q1[63] = II(Q1[59], Q1[62], Q1[61], Q1[60], m1[ 2], 15, 0x2ad7d2bb);	

		QM1_1 = QM1 + Q1[63];
		QM1   = QM1 + Q [63];

		if( bit(Q[63],32) != bit(Q[61],32) )
			continue;

		if( Q1[63] - Q[63] != 0x82000000 )
			continue;

		if (!bit(QM1,26) || bit(QM1,27) || bit(QM1,32) != bit(QM2,32) ) 
        	continue;

        if( (QM1 ^ QM1_1) != 0x86000000)
			continue;

		Q [64] = II(Q [60], Q [63], Q [62], Q [61], m [ 9], 21, 0xeb86d391);
		Q1[64] = II(Q1[60], Q1[63], Q1[62], Q1[61], m1[ 9], 21, 0xeb86d391);

		QM0_1 = QM0 + Q1[64];
		QM0   = QM0 + Q [64];

		if( Q1[64] - Q[64] != 0x82000000 )
			continue;
				
		if( (QM0 ^ QM0_1) != 0x82000000)
			continue;

		if( bit(QM0,6) || bit(QM0,26) || bit(QM0,27) || bit(QM0,32) != bit(QM1,32) ) 
			continue;
        
		
		printf_bsdr(QM3_1, QM3);
		printf_bsdr(QM2_1, QM2);
		printf_bsdr(QM1_1, QM1);
		printf_bsdr(QM0_1, QM0);
		

        IV1 = QM3;
		IV2 = QM0;
		IV3 = QM1;
		IV4 = QM2;

		IV1_1 = QM3_1; 
		IV2_1 = QM0_1;
		IV3_1 = QM1_1;
		IV4_1 = QM2_1;

	    
/*
	    if(DEBUG){
	    	printf("Printing ∆Q[i] to check if it follows the differential path\n");
	    	printf("PRINTING BSDR: ∆Q[i]: \n");
	    	for(int i = 1; i < 35; i++){
	    		printf("∆Q[%2d]:  ",i);
	    		printf_bsdr(Q1[i],Q[i]);
	    	}
	    	for(int i = 35; i < 65; i++){
	    		printf("δQ[%2d]:  ",i);
	    		printf_bsdr(Q1[i]-Q[i], 0x00000000);
	    	}
	    	printf("\n");
	    }*/

	    /*
	    if(DEBUG){
	    	printf("PRINTING Q[i], Q1[i]:\n");
	    	for(int i = 1; i < 65; i++){
	    		printf("Q [%2d]:  ",i);
	    		print_bin(Q[i]);
	    		printf("Q1[%2d]:  ",i);
	    		print_bin(Q1[i]);
	    		printf("\n");
	    	}
		}*/

	    /*
		if(DEBUG){
			printf("Printing message differences\n");
			for( int i = 0; i < 16; i++){
				printf("δm[%2d]: ",i);
				printf_bsdr( m1[i]-m[i], 0x00000000 );
				printf("\n");
			}
		} */


		
		
	    printf("Found it in 2^%2.2lf iterations\n\n", log(i)/log(2));


	    printf("M1:\n");
	    for(int i=0; i<16; i++){
	    	uint8_t *p;
	    	p=(uint8_t *)&m[i];
	    	printf("%2.2x%2.2x%2.2x%2.2x ", p[0], p[1], p[2], p[3]);	
	    }
	    printf("\n\n");

	    printf("M2:\n");
	    for(int i=0; i<16; i++){
	    	uint8_t *p;
	    	p=(uint8_t *)&m1[i];
	    	printf("%2.2x%2.2x%2.2x%2.2x ", p[0], p[1], p[2], p[3]);	
	    }
	    printf("\n\n");

	    write_block("m1_file", (void *)m);
	    write_block("m2_file", (void *)m1);

	    return 0;

	}
}





int block2(void){

	uint32_t Q[65], Q1[65], m[16], m1[16], QM0, QM1, QM2, QM3, QM0_1, QM1_1, QM2_1, QM3_1;


	//Initialization 
  	QM3 = IV1;  QM0 = IV2;
  	QM1 = IV3;  QM2 = IV4;

  	//Initialization 
  	QM3_1 = IV1_1;  QM0_1 = IV2_1;
  	QM1_1 = IV3_1;  QM2_1 = IV4_1;

	// Hex table
	// 0 = 0000  1 = 0001  2 = 0010  3 = 0011
	// 4 = 0100  5 = 0101  6 = 0110  7 = 0111
	// 8 = 1000  9 = 1001  A = 1010  B = 1011
	// C = 1100  D = 1101  E = 1110  F = 1111

  	uint64_t i = 1;
  	uint64_t j = 0;
  	int round50 = 0;
  	int round17 = 0;

	while(1){



/*		if(i%100000000 == 0){
		printf("\n");
		printf("Reach round17: %d\n", round17);
		printf("Reach round50: %d\n", round50);
		printf("%8.8lf\n", (double)(j)/round17*100);
		printf("Reach round 50 percent: %8.8lf\n\n", (double)(round50)/round17*100);
			return 0;
		}*/
		i++;



		//  Q[i] is contructed as shown in the following example:
		//  	
		// 	Conditions:
		//				. ---> no condition on Q[i]
		//				0 ---> Q[i] must be 0
		//				1 ---> Q[i] must be 1
		//				^ ---> Q[i] must be equal to Q[i-1]
		//
		//	If we have bitcondition 0000 .... 0000 0000 1111 1111 1111 ^^^^
		// 
		//  	1. First Q[i] is generated at random
		//		2. Set to 0 all bits of Q[i] with bitcondition different than .
		//			In the example Q[i] = Q[i] & 0x0f000000              ----> 0000 .... 0000 0000 0000 0000 0000 0000
		//		3. Add the 1's
		//			In the example Q[i] = Q[i] | 0x0000fff0              ----> 0000 .... 0000 0000 1111 1111 1111 0000
		//		4. Add the required bits of Q[i-1]
		// 			In the example Q[i] = Q[i] | ( Q[i-1] & 0x0000000f)  ----> 0000 .... 0000 0000 1111 1111 1111 ^^^^
		//		
		//
		// Q1[i] is constructed satisfying 	∆Q[i] = Q1[i] - Q[i] ==> Q1 = Q ^ ∆Q
		
		// Q[1]  = !... 010. ..1. ...0 .... 0... .10. .... 
	    Q [1]  = frandom() & 0x71fef4bf | 0x04200040;
	    Q1[1]  = Q[1] ^ 0x82000000;

	    m [ 0] = RR(Q [ 1] - QM0    ,  7) - F(QM0    , QM1    , QM2    ) - QM3     - 0xd76aa478;
	    m1[ 0] = RR(Q1[ 1] - QM0_1  ,  7) - F(QM0_1  , QM1_1  , QM2_1  ) - QM3_1   - 0xd76aa478;

	    if( m1[ 0] != m[ 0] )
	    	continue;  

	    // Q[2]  = ^^^^ 110. ..0^ ^^^0 1..^ 1... ^10. .00. 
	    Q [2]  = frandom() & 0x01c06759 | 0x0c008840 | Q[1] & 0xf01e1080;
	    Q1[2]  = Q[2] ^ 0x82000020;

	    m [ 1] = RR(Q [ 2] - Q [ 1], 12) - F(Q [ 1], QM0    , QM1    ) - QM2     - 0xe8c7b756;
	    m1[ 1] = RR(Q1[ 2] - Q1[ 1], 12) - F(Q1[ 1], QM0_1  , QM1_1  ) - QM2_1   - 0xe8c7b756;

	    if( m1[ 1] != m[ 1] )
	    	continue; 

	    // Q[3]  = ^011 111. ..01 1111 0..0 1..1 011^ ^11.
	    Q [3]  = frandom() & 0x01c06601 | 0x3e1f0966 | Q[2] & 0x80000018;
	    Q1[3]  = Q[3] ^ 0xfe3f18e0;

	    m [ 2] = RR(Q [ 3] - Q [ 2], 17) - F(Q [ 2], Q [ 1], QM0    ) - QM1     - 0x242070db;
	    m1[ 2] = RR(Q1[ 3] - Q1[ 2], 17) - F(Q1[ 2], Q1[ 1], QM0_1  ) - QM1_1   - 0x242070db;

	     printf("Round 4\n");

	    if( m1[ 2] != m[ 2] )
	    	continue; 

	    // Q[4]  = ^011 101. ..00 0100 ...0 0^^0 0001 000^ 
	    Q [4]  = frandom() & 0x01c0e000 | 0x3b040010 | Q[3] & 0x80000601;
	    Q1[4]  = Q[4] ^ 0x8600003e;

	    m [ 3] = RR(Q [ 4] - Q [ 3], 22) - F(Q [ 3], Q [ 2], Q [ 1]) - QM0     - 0xc1bdceee;
	    m1[ 3] = RR(Q1[ 4] - Q1[ 3], 22) - F(Q1[ 3], Q1[ 2], Q1[ 1]) - QM0_1   - 0xc1bdceee;

	     printf("Round 5\n");

	    if( m1[ 3] != m[ 3] )
	    	continue; 

	    //  Q[5]  = !100 10.. ..10 1111 ...0 1110 0101 0000   
	    // ∆Q[5]  = 0000 0000 0-++ ++++ ++++ ++++ ++00 0000
	    Q [5]  = frandom() & 0x03c0ee50 | 0x482f0e50;
	    Q1[5]  = Q[5] ^ 0x80001fc1;

	    m [ 4] = RR(Q [ 5] - Q [ 4],  7) - F(Q [ 4], Q [ 3], Q [ 2]) - Q [ 1] - 0xf57c0faf;
	    m1[ 4] = RR(Q1[ 5] - Q1[ 4],  7) - F(Q1[ 4], Q1[ 3], Q1[ 2]) - Q1[ 1] - 0xf57c0faf;

	    if( (m1[ 4] - m[ 4]) != 0x80000000)
	    	continue;
	
	    return 0;

	    //  Q[6]  = ^..0 010. 1.10 ..10 11.0 1100 0101 0110
	    // ∆Q[6]  = +000 0000 +000 0000 0000 0000 0-00 0000
	    Q [6]  = frandom() & 0x614c2000 | 0x080a2cc56 | Q[5] & 0x80000000;
	    Q1[6]  = Q[6] ^ 0x80330000;

	    m [ 5] = RR(Q [ 6] - Q [ 5], 12) - F(Q [ 5], Q [ 4], Q [ 3]) - Q [ 2] - 0x4787c62a;
	    m1[ 5] = RR(Q1[ 6] - Q1[ 5], 12) - F(Q1[ 5], Q1[ 4], Q1[ 3]) - Q1[ 2] - 0x4787c62a;

	    if( m1[ 5] != m[ 5] )
	    	continue;

	    //  Q[7]  = !..1 011^ 1.00 ..01 10.1 1110 00.. 0001 
	    // ∆Q[7]  = ++++ ++-- -000 0000 0000 -+++ ++-+ ++++
	    Q [7] = frandom() & 0x604c2030 | 0x16819e01 | Q[6] & 0x01000000;
	    Q1[7] = Q[7] ^ 0x980003c0;

	    m [ 6] = RR(Q [ 7] - Q [ 6], 17) - F(Q [ 6], Q [ 5], Q [ 4]) - Q [ 3] - 0xa8304613;
	    m1[ 6] = RR(Q1[ 7] - Q1[ 6], 17) - F(Q1[ 6], Q1[ 5], Q1[ 4]) - Q1[ 3] - 0xa8304613;

	    if( m1[ 6] != m[ 6] )
	    	continue;

	    //  Q[8]  = ^..0 0100 0.11 ..10 1... ..11 111. ..^0 
	    // ∆Q[8]  = 0000 0000 -00- +++- +000 0000 0000 000+	
	    Q [8]  = frandom() & 0x604c7c1c | 0x043283e0 | Q[7] & 0x80000002;
	    Q1[8]  = Q[8] ^ 0x87838000;

	    m [ 7] = RR(Q [ 8] - Q [ 7], 22) - F(Q [ 7], Q [ 6], Q [ 5]) - Q [ 4] - 0xfd469501;
	    m1[ 7] = RR(Q1[ 8] - Q1[ 7], 22) - F(Q1[ 7], Q1[ 6], Q1[ 5]) - Q1[ 4] - 0xfd469501;

	    if( m1[ 7] != m[ 7] )
	    	continue;

	    //  Q[9]  = ^..1 1100 0... ..01 0..^ ..01 110. ..01 
	    // ∆Q[9]  = -000 0000 0000 0000 0000 000- ++00 00+-
	    Q [9]  = frandom() & 0x607c6c1c | 0x1c0101c1 | Q[8] & 0x80001000;
	    Q1[9] = Q[9] ^ 0x800003c3;

	    m [ 8] = RR(Q [ 9] - Q [ 8],  7) - F(Q [ 8], Q [ 7], Q [ 6]) - Q [ 5] - 0x698098d8;
	    m1[ 8] = RR(Q1[ 9] - Q1[ 8],  7) - F(Q1[ 8], Q1[ 7], Q1[ 6]) - Q1[ 5] - 0x698098d8;

	    if( m1[8] != m[8] )
	    	continue;

	    //  Q[10] = ^... .111 1... 1011 1100 1.11 11.. ..00 
	    // ∆Q[10] = +000 0000 0000 0000 00+- 0000 0000 0000
	    Q [10] = frandom() & 0x7870043c | 0x078bcbc0 | Q[9] & 0x80000000;
	    Q1[10] = Q[10] ^ 0x80001000;

	    m [ 9] = RR(Q [10] - Q [ 9], 12) - F(Q [ 9], Q [ 8], Q [ 7]) - Q [ 6] - 0x8b44f7af;
	    m1[ 9] = RR(Q1[10] - Q1[ 9], 12) - F(Q1[ 9], Q1[ 8], Q1[ 7]) - Q1[ 6] - 0x8b44f7af;

	    if( m1[9] != m[9] )
	    	continue;

	    //  Q[11] = ^..0 0... .... 1101 1100 0.11 110. ..11 
	    // ∆Q[11] = ++00 0000 0000 0000 0000 0000 0000 0000
	    Q [11] = frandom() & 0x67f0041c | 0x000dc3c3 | Q[10] & 0x80000000;
	    Q1[11] = Q[11] ^ 0x80000000;

	    m [10] = RR(Q [11] - Q [10], 17) - F(Q [10], Q [ 9], Q [ 8]) - Q [ 7] - 0xffff5bb1;
	    m1[10] = RR(Q1[11] - Q1[10], 17) - F(Q1[10], Q1[ 9], Q1[ 8]) - Q1[ 7] - 0xffff5bb1; 

	    if( (m1[10] != m[10]) )
	    	continue;

	    //  Q[12] = ^^^0 0^^^ .... 1000 0001 .... 1... ....
	    // ∆Q[12] = +000 0000 0000 -+++ +++0 000- +000 0000
	    Q [12] = frandom() & 0x00f00f7f | 0x00081080 | Q[11] & 0xe7000000;
	    Q1[12] = Q[12] ^ 0x800fe080;

	    m [11] = RR(Q [12] - Q [11], 22) - F(Q [11], Q [10], Q [ 9]) - Q [ 8] - 0x895cd7be;
	    m1[11] = RR(Q1[12] - Q1[11], 22) - F(Q1[11], Q1[10], Q1[ 9]) - Q1[ 8] - 0x895cd7be;

	    if( (m[11] - m1[11]) != 0x00008000)
	    	continue;

	    //  Q[13] = !011 1111 0... 1111 111. .... 0... 1...
	    // ∆Q[13] = +000 00+- 0000 0000 0000 0000 0000 0000
	    Q [13] = frandom() & 0x00701f77 | 0x3f0fe008;
	    Q1[13] = Q[13] ^ 0xff000000;

	    m [12] = RR(Q [13] - Q [12],  7) - F(Q [12], Q [11], Q [10]) - Q [ 9] - 0x6b901122;
	    m1[12] = RR(Q1[13] - Q1[12],  7) - F(Q1[12], Q1[11], Q1[10]) - Q1[ 9] - 0x6b901122;

	    if( m1[12] != m[12] )
	    	continue;

	    //  Q[14] = ^100 0000 1... 1011 111. .... 1... 1... 
	    // ∆Q[14] = +000 0000 0000 0000 0000 0000 0000 0000
	    Q [14] = frandom() & 0x00701f77 | 0x408be088 | Q[12] & 0x80000000;
	    Q1[14] = Q[14] ^ 0x80000000;

	    m [13] = RR(Q [14] - Q [13], 12) - F(Q [13], Q [12], Q [11]) - Q [10] - 0xfd987193;
	    m1[13] = RR(Q1[14] - Q1[13], 12) - F(Q1[13], Q1[12], Q1[11]) - Q1[10] - 0xfd987193;

	    if( m1[13] != m[13] )
	    	continue;

	    //  Q[15] = 0111 1101 .... .... 00.. .... .... 0...
	    // ∆Q[15] = +000 0000 0000 0000 -000 0000 0000 +000
	    Q [15] = frandom() & 0x00ff3ff7 | 0x7d000000;
	    Q1[15] = Q[15] ^ 0x80008008;

	    m [14] = RR(Q [15] - Q [14], 17) - F(Q [14], Q [13], Q [12]) - Q [11] - 0xa679438e;
	    m1[14] = RR(Q1[15] - Q1[14], 17) - F(Q1[14], Q1[13], Q1[12]) - Q1[11] - 0xa679438e;

	    if( (m1[14] - m[14]) != 0x80000000)
	    	continue;

	    //  Q[16] = 0.10 .... .... ..!. .... .... .... .... 
	    // ∆Q[16] = +0-0 0000 0000 0000 0000 0000 0000 0000
	    if( bit(Q[15], 18) ){
	    	Q[16] = frandom() & 0x4ffdffff | 0x20000000;
	    }else{
	    	Q[16] = frandom() & 0x4ffdffff | 0x20020000;
	    } 
	    Q1[16] = Q[16] ^ 0xa0000000;

	    m [15] = RR(Q [16] - Q [15], 22) - F(Q [15], Q [14], Q [13]) - Q [12] - 0x49b40821; 
	    m1[15] = RR(Q1[16] - Q1[15], 22) - F(Q1[15], Q1[14], Q1[13]) - Q1[12] - 0x49b40821; 

	    if( (m1[15] - m[15]) != 0x00000000)
	    	continue;

	    return 0;
	

	    // For each Q[i]:
	    // 		the first  'if' checks the bitconditions
	    //		the second 'if' checks the differential path


		// Q[17] = 0!.. .... .... ..0. ^... .... .... ^... 
	    Q [17] = GG(Q [13], Q [16], Q [15], Q [14], m [ 1],  5, 0xf61e2562);
	    Q1[17] = GG(Q1[13], Q1[16], Q1[15], Q1[14], m1[ 1],  5, 0xf61e2562);

	    if ( bit(Q[17],32) || bit(Q[17],18) || bit(Q[17],16) != bit(Q[16],16) || bit(Q[17],4) != bit(Q[16],4) || bit(Q[17],31) == bit(Q[16],31) )
	    	continue;

	    // Q[18] = 0.^. .... .... ..1. .... .... .... ....
	    Q [18] = GG(Q [14], Q [17], Q [16], Q [15], m [ 6],  9, 0xc040b340);
	    Q1[18] = GG(Q1[14], Q1[17], Q1[16], Q1[15], m1[ 6],  9, 0xc040b340);

	    if ( bit(Q[18],32) || bit(Q[18],30) != bit(Q[17],30) || !bit(Q[18],18) )
	    	continue;

	    // Q[19] = 0... .... .... ..0. .... .... .... ....
	    Q [19] = GG(Q [15], Q [18], Q [17], Q [16], m [11], 14, 0x265e5a51);
	    Q1[19] = GG(Q1[15], Q1[18], Q1[17], Q1[16], m1[11], 14, 0x265e5a51);

	    if ( bit(Q[19],32) || bit(Q[19],18) )
	    	continue;

	    // Q[20] = 0... .... .... .... .... .... .... ....
	    Q [20] = GG(Q [16], Q [19], Q [18], Q [17], m [ 0], 20, 0xe9b6c7aa);
	    Q1[20] = GG(Q1[16], Q1[19], Q1[18], Q1[17], m1[ 0], 20, 0xe9b6c7aa);

	    if( bit(Q[20], 32) || bit(Q[20],19) != bit(Q[19],19) )
	    	continue;

	    // Q[21] = 0... .... .... ..^. .... .... .... ....
	    Q [21] = GG(Q [17], Q [20], Q [19], Q [18], m [ 5],  5, 0xd62f105d);
	    Q1[21] = GG(Q1[17], Q1[20], Q1[19], Q1[18], m1[ 5],  5, 0xd62f105d);

	    if ( bit(Q[21],32) || bit(Q[21],18) != bit(Q[20],18) )
	    	continue;

	    // Q[22] = 0... .... .... .... .... .... .... ....
	    Q [22] = GG(Q [18], Q [21], Q [20], Q [19], m [10],  9, 0x2441453);
	    Q1[22] = GG(Q1[18], Q1[21], Q1[20], Q1[19], m1[10],  9, 0x2441453);

	    if( bit(Q[22],32) )
	    	continue; 

	    // Q[23] = 0... .... .... .... .... .... .... ....
	    Q [23] = GG(Q [19], Q [22], Q [21], Q [20], m [15], 14, 0xd8a1e681);
	    Q1[23] = GG(Q1[19], Q1[22], Q1[21], Q1[20], m1[15], 14, 0xd8a1e681);

	    if( bit(Q[23],32) )
	    	continue; 

	    // Q[24] = 1... .... .... .... .... .... .... ....
	    Q [24] = GG(Q [20], Q [23], Q [22], Q [21], m [ 4], 20, 0xe7d3fbc8);
	    Q1[24] = GG(Q1[20], Q1[23], Q1[22], Q1[21], m1[ 4], 20, 0xe7d3fbc8); 

	    if( !bit(Q[24],32) )
	    	continue; 

	    // No bitconditions for Q[25]...Q[47]
	    Q [25] = GG(Q [21], Q [24], Q [23], Q [22], m [ 9],  5, 0x21e1cde6);
	    Q1[25] = GG(Q1[21], Q1[24], Q1[23], Q1[22], m1[ 9],  5, 0x21e1cde6);

	    if ( (Q[25] ^ Q1[25]) != 0x00000000 ) 
	    	continue;
	    
	    Q [26] = GG(Q [22], Q [25], Q [24], Q [23], m [14],  9, 0xc33707d6);
	    Q1[26] = GG(Q1[22], Q1[25], Q1[24], Q1[23], m1[14],  9, 0xc33707d6);

	    if ( (Q[26] ^ Q1[26]) != 0x00000000 ) 
	    	continue;

	    Q [27] = GG(Q [23], Q [26], Q [25], Q [24], m [ 3], 14, 0xf4d50d87);
	    Q1[27] = GG(Q1[23], Q1[26], Q1[25], Q1[24], m1[ 3], 14, 0xf4d50d87);

	    if ( (Q[27] ^ Q1[27]) != 0x00000000 ) 
	    	continue;

	    Q [28] = GG(Q [24], Q [27], Q [26], Q [25], m [ 8], 20, 0x455a14ed);
	    Q1[28] = GG(Q1[24], Q1[27], Q1[26], Q1[25], m1[ 8], 20, 0x455a14ed);

	    if ( (Q[28] ^ Q1[28]) != 0x00000000 ) 
	    	continue;

	    Q [29] = GG(Q [25], Q [28], Q [27], Q [26], m [13],  5, 0xa9e3e905);
	    Q1[29] = GG(Q1[25], Q1[28], Q1[27], Q1[26], m1[13],  5, 0xa9e3e905);

	    if ( (Q[29] ^ Q1[29]) != 0x00000000 ) 
	    	continue;

	    Q [30] = GG(Q [26], Q [29], Q [28], Q [27], m [ 2],  9, 0xfcefa3f8);
	    Q1[30] = GG(Q1[26], Q1[29], Q1[28], Q1[27], m1[ 2],  9, 0xfcefa3f8);

	    if ( (Q[30] ^ Q1[30]) != 0x00000000 ) 
	    	continue;

	    Q [31] = GG(Q [27], Q [30], Q [29], Q [28], m [ 7], 14, 0x676f02d9);
	    Q1[31] = GG(Q1[27], Q1[30], Q1[29], Q1[28], m1[ 7], 14, 0x676f02d9);

	    if ( (Q[31] ^ Q1[31]) != 0x00000000 ) 
	    	continue;

	    Q [32] = GG(Q [28], Q [31], Q [30], Q [29], m [12], 20, 0x8d2a4c8a);
	    Q1[32] = GG(Q1[28], Q1[31], Q1[30], Q1[29], m1[12], 20, 0x8d2a4c8a);

	    if ( (Q[32] ^ Q1[32]) != 0x00000000 ) 
	    	continue;


	    Q [33] = HH(Q [29], Q [32], Q [31], Q [30], m [ 5],  4, 0xfffa3942);
	    Q1[33] = HH(Q1[29], Q1[32], Q1[31], Q1[30], m1[ 5],  4, 0xfffa3942);

	    if ( (Q[33] ^ Q1[33]) != 0x00000000 ) 
	    	continue;

	    Q [34] = HH(Q [30], Q [33], Q [32], Q [31], m [ 8], 11, 0x8771f681);
	    Q1[34] = HH(Q1[30], Q1[33], Q1[32], Q1[31], m1[ 8], 11, 0x8771f681);

	    if ( (Q[34] ^ Q1[34]) != 0x00000000 ) 
	    	continue;

	    Q [35] = HH(Q [31], Q [34], Q [33], Q [32], m [11], 16, 0x6d9d6122);
	    Q1[35] = HH(Q1[31], Q1[34], Q1[33], Q1[32], m1[11], 16, 0x6d9d6122);

	    if ( (Q1[35] - Q[35]) != 0x80000000 ) 
	    	continue;

	    Q [36] = HH(Q [32], Q [35], Q [34], Q [33], m [14], 23, 0xfde5380c);
	    Q1[36] = HH(Q1[32], Q1[35], Q1[34], Q1[33], m1[14], 23, 0xfde5380c);

	    if ( (Q1[36] - Q[36]) != 0x80000000 ) 
	    	continue;

	    Q [37] = HH(Q [33], Q [36], Q [35], Q [34], m [ 1],  4, 0xa4beea44);
	    Q1[37] = HH(Q1[33], Q1[36], Q1[35], Q1[34], m1[ 1],  4, 0xa4beea44);

	    if ( (Q1[37] - Q[37]) != 0x80000000 ) 
	    	continue;

	    Q [38] = HH(Q [34], Q [37], Q [36], Q [35], m [ 4], 11, 0x4bdecfa9);
	    Q1[38] = HH(Q1[34], Q1[37], Q1[36], Q1[35], m1[ 4], 11, 0x4bdecfa9);

	    if ( (Q1[38] - Q[38]) != 0x80000000 ) 
	    	continue;

	    Q [39] = HH(Q [35], Q [38], Q [37], Q [36], m [ 7], 16, 0xf6bb4b60);
	    Q1[39] = HH(Q1[35], Q1[38], Q1[37], Q1[36], m1[ 7], 16, 0xf6bb4b60);

	    if ( (Q1[39] - Q[39]) != 0x80000000 ) 
	    	continue;

	    Q [40] = HH(Q [36], Q [39], Q [38], Q [37], m [10], 23, 0xbebfbc70);
	    Q1[40] = HH(Q1[36], Q1[39], Q1[38], Q1[37], m1[10], 23, 0xbebfbc70);
	    
	    if ( (Q1[40] - Q[40]) != 0x80000000 ) 
	    	continue;

	    Q [41] = HH(Q [37], Q [40], Q [39], Q [38], m [13],  4, 0x289b7ec6);
	    Q1[41] = HH(Q1[37], Q1[40], Q1[39], Q1[38], m1[13],  4, 0x289b7ec6);

	    if ( (Q1[41] - Q[41]) != 0x80000000 ) 
	    	continue;

	    Q [42] = HH(Q [38], Q [41], Q [40], Q [39], m [ 0], 11, 0xeaa127fa);
	    Q1[42] = HH(Q1[38], Q1[41], Q1[40], Q1[39], m1[ 0], 11, 0xeaa127fa);

	    if ( (Q1[42] - Q[42]) != 0x80000000 ) 
	    	continue;

	    Q [43] = HH(Q [39], Q [42], Q [41], Q [40], m [ 3], 16, 0xd4ef3085);
	    Q1[43] = HH(Q1[39], Q1[42], Q1[41], Q1[40], m1[ 3], 16, 0xd4ef3085);

	    if ( (Q1[43] - Q[43]) != 0x80000000 ) 
	    	continue;

	    Q [44] = HH(Q [40], Q [43], Q [42], Q [41], m [ 6], 23, 0x4881d05);
	    Q1[44] = HH(Q1[40], Q1[43], Q1[42], Q1[41], m1[ 6], 23, 0x4881d05);

	    if ( (Q1[44] - Q[44]) != 0x80000000 ) 
	    	continue;

	    Q [45] = HH(Q [41], Q [44], Q [43], Q [42], m [ 9],  4, 0xd9d4d039);
	    Q1[45] = HH(Q1[41], Q1[44], Q1[43], Q1[42], m1[ 9],  4, 0xd9d4d039);

	    if ( (Q1[45] - Q[45]) != 0x80000000 ) 
	    	continue;

	    Q [46] = HH(Q [42], Q [45], Q [44], Q [43], m [12], 11, 0xe6db99e5);
	    Q1[46] = HH(Q1[42], Q1[45], Q1[44], Q1[43], m1[12], 11, 0xe6db99e5);

	    if ( (Q1[46] - Q[46]) != 0x80000000 ) 
	    	continue;

	    Q [47] = HH(Q [43], Q [46], Q [45], Q [44], m [15], 16, 0x1fa27cf8);
	    Q1[47] = HH(Q1[43], Q1[46], Q1[45], Q1[44], m1[15], 16, 0x1fa27cf8);

	    if ( (Q1[47] - Q[47]) != 0x80000000 ) 
	    	continue;

	    // Q[48] = m... .... .... .... .... .... .... ....
	    Q [48] = HH(Q [44], Q [47], Q [46], Q [45], m [ 2], 23, 0xc4ac5665);
	    Q1[48] = HH(Q1[44], Q1[47], Q1[46], Q1[45], m1[ 2], 23, 0xc4ac5665);

	    if ( bit(Q[48],32) != bit(Q[46],32) )
	    	continue;

	    if ( (Q1[48] - Q[48]) != 0x80000000 )
	    	continue;

	    // Q[49] = m... .... .... .... .... .... .... ....
	    Q [49] = II(Q [45], Q [48], Q [47], Q [46], m [ 0],  6, 0xf4292244);
		Q1[49] = II(Q1[45], Q1[48], Q1[47], Q1[46], m1[ 0],  6, 0xf4292244);

		if ( bit(Q[49],32) != bit(Q[47],32) ) 
	    	continue;

		if ( (Q1[49] - Q[49]) != 0x80000000 ) 
	    	continue;

	    // Q[50] = #... .... .... .... .... .... .... ....
		Q [50] = II(Q [46], Q [49], Q [48], Q [47], m [ 7], 10, 0x432aff97);
		Q1[50] = II(Q1[46], Q1[49], Q1[48], Q1[47], m1[ 7], 10, 0x432aff97);

		round50++;

		if ( bit(Q[50],32) == bit(Q[48],32) )
			continue;

		if ( (Q1[50] - Q[50]) != 0x80000000 ) 
	    	continue;

	    // Q[51] = m... .... .... .... .... .... .... ....
		Q [51] = II(Q [47], Q [50], Q [49], Q [48], m [14], 15, 0xab9423a7);
		Q1[51] = II(Q1[47], Q1[50], Q1[49], Q1[48], m1[14], 15, 0xab9423a7);

		if ( bit(Q[51],32) != bit(Q[49],32) ) 
	    	continue;

		if ( (Q1[51] - Q[51]) != 0x80000000 ) 
	    	continue;
		
		// Q[52] = m... .... .... .... .... .... .... ....    
		Q [52] = II(Q [48], Q [51], Q [50], Q [49], m [ 5], 21, 0xfc93a039);
		Q1[52] = II(Q1[48], Q1[51], Q1[50], Q1[49], m1[ 5], 21, 0xfc93a039);

		if ( bit(Q[52],32) != bit(Q[50],32) ) 
	    	continue;

		if ( (Q1[52] - Q[52]) != 0x80000000 ) 
	    	continue;
	    
	    // Q[53] = m... .... .... .... .... .... .... ....
		Q [53] = II(Q [49], Q [52], Q [51], Q [50], m [12],  6, 0x655b59c3);
		Q1[53] = II(Q1[49], Q1[52], Q1[51], Q1[50], m1[12],  6, 0x655b59c3);

		if ( bit(Q[53],32) != bit(Q[51],32) ) 
	    	continue;

		if ( (Q1[53] - Q[53]) != 0x80000000 ) 
	    	continue;

	    // Q[54] = m... .... .... .... .... .... .... ....
		Q [54] = II(Q [50], Q [53], Q [52], Q [51], m [ 3], 10, 0x8f0ccc92);
		Q1[54] = II(Q1[50], Q1[53], Q1[52], Q1[51], m1[ 3], 10, 0x8f0ccc92);

		if ( bit(Q[54],32) != bit(Q[52],32) ) 
	    	continue;

		if ( (Q1[54] - Q[54]) != 0x80000000 ) 
	    	continue;
	    
	    // Q[55] = m... .... .... .... .... .... .... ....
		Q [55] = II(Q [51], Q [54], Q [53], Q [52], m [10], 15, 0xffeff47d);
		Q1[55] = II(Q1[51], Q1[54], Q1[53], Q1[52], m1[10], 15, 0xffeff47d);

		if ( bit(Q[55],32) != bit(Q[53],32) ) 
	    	continue;

		if ( (Q1[55] - Q[55]) != 0x80000000 ) 
	    	continue;

	    // Q[56] = m... .... .... .... .... .... .... ....
		Q [56] = II(Q [52], Q [55], Q [54], Q [53], m [ 1], 21, 0x85845dd1);
		Q1[56] = II(Q1[52], Q1[55], Q1[54], Q1[53], m1[ 1], 21, 0x85845dd1);

		if ( bit(Q[56],32) != bit(Q[54],32) ) 
	    	continue;

		if ( (Q1[56] - Q[56]) != 0x80000000 ) 
	    	continue;

	    // Q[57] = m... .... .... .... .... .... .... ....
		Q [57] = II(Q [53], Q [56], Q [55], Q [54], m [ 8],  6, 0x6fa87e4f);
		Q1[57] = II(Q1[53], Q1[56], Q1[55], Q1[54], m1[ 8],  6, 0x6fa87e4f);

		if ( bit(Q[57],32) != bit(Q[55],32) ) 
	    	continue;

		if ( (Q1[57] - Q[57]) != 0x80000000 ) 
	    	continue;

	    // Q[58] = m... .... .... .... .... .... .... ....
		Q [58] = II(Q [54], Q [57], Q [56], Q [55], m [15], 10, 0xfe2ce6e0);
		Q1[58] = II(Q1[54], Q1[57], Q1[56], Q1[55], m1[15], 10, 0xfe2ce6e0);

		if ( bit(Q[58],32) != bit(Q[56],32) ) 
	    	continue;

		if ( (Q1[58] - Q[58]) != 0x80000000 ) 
	    	continue;

	    // Q[59] = m... .... .... .... .... .... .... ....
		Q [59] = II(Q [55], Q [58], Q [57], Q [56], m [ 6], 15, 0xa3014314);
		Q1[59] = II(Q1[55], Q1[58], Q1[57], Q1[56], m1[ 6], 15, 0xa3014314);

		if ( bit(Q[59],32) != bit(Q[57],32) ) 
	    	continue;

		if ( (Q1[59] - Q[59]) != 0x80000000 ) 
	    	continue;
	    
	    // Q[60] = #... .... .... .... .... .... .... ....
		Q [60] = II(Q [56], Q [59], Q [58], Q [57], m [13],  21, 0x4e0811a1);
		Q1[60] = II(Q1[56], Q1[59], Q1[58], Q1[57], m1[13],  21, 0x4e0811a1);

		if ( bit(Q[60],32) == bit(Q[58],32) )
			continue;

		if ( (Q1[60] - Q[60]) != 0x80000000 ) 
	    	continue;

	    // Q[61] = m... .... .... .... .... .... .... ....
		Q [61] = II(Q [57], Q [60], Q [59], Q [58], m [ 4],  6, 0xf7537e82);
		Q1[61] = II(Q1[57], Q1[60], Q1[59], Q1[58], m1[ 4],  6, 0xf7537e82);

		if ( bit(Q[61],32) != bit(Q[59],32) ) 
	    	continue;

	    // Q[62] = m... .... .... .... .... .... .... ....
		Q [62] = II(Q [58], Q [61], Q [60], Q [59], m [11], 10, 0xbd3af235);
		Q1[62] = II(Q1[58], Q1[61], Q1[60], Q1[59], m1[11], 10, 0xbd3af235);

		if( bit(Q[62],32) != bit(Q[60],32) )
			continue;

		// Q[63] = m... .... .... .... .... .... .... ....
		Q [63] = II(Q [59], Q [62], Q [61], Q [60], m [ 2], 15, 0x2ad7d2bb);
		Q1[63] = II(Q1[59], Q1[62], Q1[61], Q1[60], m1[ 2], 15, 0x2ad7d2bb);		

		if( bit(Q[63],32) != bit(Q[61],32) )
			continue;

		Q [64] = II(Q [60], Q [63], Q [62], Q [61], m [ 9], 21, 0xeb86d391);
		Q1[64] = II(Q1[60], Q1[63], Q1[62], Q1[61], m1[ 9], 21, 0xeb86d391);


	} // End While



} // End function




int main (void){

	double cpu_time_used;
	clock_t start_t, end_t;

	start_t = clock();
	
	// Initializing seed for random number generator
	srand ( time(NULL) );
	seed32_1 = rand();
	seed32_2 = rand() % 162287;

	printf("Starting first block computation\n");
	printf("IV1 = %x\n", IV1 );
	printf("IV2 = %x\n", IV2 );
	printf("IV3 = %x\n", IV3 );
	printf("IV4 = %x\n", IV4 );
	block();

	end_t = clock();
	cpu_time_used = ((double) (end_t - start_t)) / CLOCKS_PER_SEC;
	printf("Block 1 found in %lf seconds\n", cpu_time_used  );

/*
	printf("IV1 = %x\n", IV1 );
	printf("IV2 = %x\n", IV2 );
	printf("IV3 = %x\n", IV3 );
	printf("IV4 = %x\n", IV4 );

	printf("IV1_1 = %x\n", IV1_1 );
	printf("IV2_1 = %x\n", IV2_1 );
	printf("IV3_1 = %x\n", IV3_1 );
	printf("IV4_1 = %x\n", IV4_1 );


	printf("Starting second block computation\n");
	block2();*/
	
	return 0;
}

