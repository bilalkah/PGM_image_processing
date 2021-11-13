#ifndef PGM_H
#define PGM_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <float.h>

// PGM data structure

typedef struct
{
    int width;
    int height;
    int max_val;
    char type[3];
    unsigned char **data;
} PGM;

// PGM file format read and write
char *check_pgm_type(char *filename);
PGM *pgm_read(char *filename);
void skip_comments(FILE *fp);
PGM *pgm_create(int width, int height, int max_val, char *type);
void pgm_write(PGM *pgm, char *filename);
void pgm_free(PGM *pgm);

// Filter functions
PGM *filter_median(PGM *img, int filter_size, char *padding);
PGM *filter_average(PGM *img, int filter_size, char *padding);
PGM *filter_sobel(PGM *img,char *padding);
unsigned char find_median(unsigned char **array, int i, int j, int size);
void mergeSort(unsigned char *arr, int left, int right);
void merge(unsigned char *arr, int left, int middle, int right);

#endif //PGM_H