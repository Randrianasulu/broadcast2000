#ifndef BYTEORDER_H
#define BYTEORDER_H

#include "sizes.h"

inline int get_byte_order()
{                // 1 if little endian
	FOUR byteordertest;
	int byteorder;

	byteordertest = 0x00000001;
	byteorder = *((unsigned char *)&byteordertest);
	return byteorder;
}

#define SWAP_ITERATE \
					byte1 = buffer1[i]; \
      				byte2 = buffer2[i]; \
      				buffer1[i] = byte2; \
      				buffer2[i] = byte1; \
					i += 2;
      							                                         
#define SWAP_24BIT_ITERATE \
					byte1 = buffer1[i]; \
      				byte2 = buffer2[i]; \
      				byte3 = buffer3[i]; \
      				buffer1[i] = byte3; \
      				buffer2[i] = byte2; \
      				buffer3[i] = byte1; \
      				i += 3;

#define SWAP_32BIT_ITERATE \
					byte1 = buffer1[i]; \
      				byte2 = buffer2[i]; \
      				byte3 = buffer3[i]; \
      				byte4 = buffer4[i]; \
      				buffer1[i] = byte4; \
      				buffer2[i] = byte1; \
      				buffer3[i] = byte2; \
      				buffer4[i] = byte3; \
      				i += 4;

inline int swap_bytes(int wordsize, unsigned char *buffer, long len)
{
	 unsigned char byte1, byte2, byte3, byte4;
	 unsigned char *buffer1 = buffer;
	 unsigned char *buffer2 = buffer + 1;
	 unsigned char *buffer3 = buffer + 2;
	 unsigned char *buffer4 = buffer + 3;

	 long i = 0, j = 0, k = 0;

//printf("swap bytes\n");

	switch(wordsize)
	{
		case 1:
			return 0;
			break;
		
		case 2:
  			len -= 8;
  			while(i < len){
    			SWAP_ITERATE
    			SWAP_ITERATE
    			SWAP_ITERATE
    			SWAP_ITERATE
  			}

  			len += 8;
  			while(i < len){
    			SWAP_ITERATE
  			}
			return 0;
			break;

		case 3:
  			len -= 12;
  			while(i < len){
    			SWAP_24BIT_ITERATE
    			SWAP_24BIT_ITERATE
    			SWAP_24BIT_ITERATE
    			SWAP_24BIT_ITERATE
  			}

  			len += 12;
  			while(i < len){
    			SWAP_24BIT_ITERATE
  			}
			return 0;
			break;

		case 4:
			len -= 16;
			while(i < len)
			{
				SWAP_32BIT_ITERATE
				SWAP_32BIT_ITERATE
				SWAP_32BIT_ITERATE
				SWAP_32BIT_ITERATE
			}

			len += 16;
			while(i < len)
			{
				SWAP_32BIT_ITERATE
			}
			return 0;
			break;
	}
	return 1;
}

#endif
