#ifndef BINARY_H
#define BINARY_H

#include "sizes.h"
#include <stdio.h>

inline int putfourswap(FOUR number, FILE *file){
  fputc(number & 0xff, file);
  fputc((number & 0xff00) >> 8, file);
  fputc((number & 0xff0000) >> 16, file);
  fputc((number & 0xff000000) >> 24, file);
}

inline int putfour(FOUR number, FILE *file){
  fputc((number & 0xff000000) >> 24, file);
  fputc((number & 0xff0000) >> 16, file);
  fputc((number & 0xff00) >> 8, file);
  fputc(number & 0xff, file);
}

inline FOUR getfour(FILE *in){
  static FOUR number=0;

  number = (FOUR)fgetc(in) << 24;
  number += (FOUR)fgetc(in) << 16;
  number += (FOUR)fgetc(in) << 8;
  number += fgetc(in);
	return number;
}

inline FOUR getfourswap(FILE *in){
  static FOUR number=0;

  number = (FOUR)fgetc(in);
  number += (FOUR)fgetc(in) << 8;
  number += (FOUR)fgetc(in) << 16;
  number += fgetc(in) << 24;
	return number;
}

inline TWO gettwo(FILE *in){
  static TWO number=0;

  number = (FOUR)fgetc(in) << 8;
  number += fgetc(in);
	return number;
}

inline int puttwo(TWO number, FILE *file){
  fputc((number & 0xff00) >> 8, file);
  fputc(number & 0xff, file);
}

#endif
