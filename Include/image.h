#ifndef __IMAGE_H___
#define __IMAGE_H__

char* getBMPData(const char* filename);
char* greyScaleBMP(char* data, unsigned int height, unsigned int width);
unsigned int getBMPDimension(const char* filename, const char dim);

#endif