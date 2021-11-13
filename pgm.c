#include "pgm.h"

/**
 * @brief Check if pgm format is P2 or P5 and return the reading mode
 * 
 * @param filename 
 * @return char* 
 */
char *check_pgm_type(char *filename)
{
    char type[3];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(EXIT_FAILURE);
    }
    fscanf(fp, "%2s", type);
    fclose(fp);
    if (strcmp(type, "P2") == 0)
    {
        return "r\0";
    }

    else if (strcmp(type, "P5") == 0)
    {
        return "rb";
    }

    return NULL;
}

/**
 * @brief Create a new pgm struct to hold the image data and return pointer to it
 *        Struct is allocated on the heap
 *        Calloc is used to allocate memory because Calloc initializes the memory to 0 for padding if needed
 * 
 * @param width 
 * @param height 
 * @param max_val 
 * @param type 
 * @return PGM* 
 */
PGM *pgm_create(int width, int height, int max_val, char *type)
{
    PGM *pgm = (PGM *)malloc(sizeof(PGM));
    pgm->width = width;
    pgm->height = height;
    pgm->max_val = max_val;
    strcpy(pgm->type, type);
    pgm->data = (unsigned char **)calloc(height, sizeof(unsigned char *));

    if (pgm->data == NULL)
    {
        free(pgm);
        fprintf(stderr, "Error: pgm_create() failed to allocate memory for pgm->data\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < height; i++)
    {
        pgm->data[i] = (unsigned char *)calloc(width, sizeof(unsigned char));
    }
    fprintf(stdout, "pgm_create() created a PGM image with width %d, height %d, max_val %d, type %s\n", width, height, max_val, type);
    return pgm;
}

/**
 * @brief Read a pgm image from a file and return a pointer to the image struct
 *        Check for PGM type is P2, reading mode is r and if PGM type is P5, reading mode is rb
 *        if PGM type is P2, data orientation is like this: data|whitespace|data|whitespace
 *          so fscanf reads data|whitespace and then data to fill the image struct
 *        if PGM type is P5, we can read the data with fread line by line 
 *        Single pixel data type is unsigned char (max value is 255)
 * 
 * @param filename 
 * @return PGM* 
 */
PGM *pgm_read(char *filename)
{
    char type[3];
    int max_val;
    int width, height, i;
    char reading_format[3];

    memcpy(reading_format, check_pgm_type(filename), 2);

    FILE *ptr = fopen(filename, reading_format);
    if (ptr == NULL)
    {
        fprintf(stderr, "Error: pgm_read() failed to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    skip_comments(ptr);
    fscanf(ptr, "%s\n", type);
    skip_comments(ptr);
    fscanf(ptr, "%d %d\n", &width, &height);
    skip_comments(ptr);
    fscanf(ptr, "%d\n", &max_val);

    PGM *pgm = pgm_create(width, height, max_val, type);
    switch (pgm->type[1])
    {
    case '2':
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                fscanf(ptr, "%hhu ", &pgm->data[i][j]);
            }
        }
        break;

    case '5':
        for (i = 0; i < pgm->height; i++)
        {
            fread(pgm->data[i], sizeof(unsigned char), pgm->width, ptr);
            if (ferror(ptr))
            {
                fprintf(stderr, "Error: pgm_read() failed to read from file %s\n", filename);
                exit(EXIT_FAILURE);
            }
        }
        break;

    default:
        fprintf(stderr, "Error: Unknown format\n");
        break;
    }

    fclose(ptr);
    return pgm;
}
/**
 * @brief There can be comments in the file, so we need to skip them
 *        Comments are lines starting with #
 *        Read char and check if it is #
 *        If it is #, skip the line
 *        If it is not #, seek back to the beginning of the line 
 * 
 * @param ptr 
 */
void skip_comments(FILE *ptr)
{
    int ch;
    char line[100];
    while ((ch = fgetc(ptr)) != EOF && isspace(ch))
    {
    };
    if (ch == '#')
    {
        fgets(line, sizeof(line), ptr);
        skip_comments(ptr);
    }
    else
    {
        fseek(ptr, -1, SEEK_CUR);
    }
}

/**
 * @brief Write a pgm image to a file
 *        Check for PGM type is P2, writing mode is w and if PGM type is P5, writing mode is wb
 *        if PGM type is P2, data orientation is like this: data|whitespace|data|whitespace
 *        so fprintf writes data|whitespace and then data to the file
 *        if PGM type is P5, we can write the data with fwrite line by line
 *        Single pixel data type is unsigned char (max value is 255)
 * 
 * @param pgm 
 * @param filename 
 */
void pgm_write(PGM *pgm, char *filename)
{
    char reading_format[3];
    if (strcmp(pgm->type, "P2") == 0)
    {
        strcpy(reading_format, "w\0");
    }
    else if (strcmp(pgm->type, "P5") == 0)
    {
        strcpy(reading_format, "wb");
    }
    else
    {
        fprintf(stderr, "Error: Unknown format\n");
        exit(EXIT_FAILURE);
    }

    FILE *ptr = fopen(filename, reading_format);
    if (ptr == NULL)
    {
        fprintf(stderr, "Error: pgm_write() failed to open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(ptr, "%s\n", pgm->type);
    fprintf(ptr, "%d %d\n", pgm->width, pgm->height);
    fprintf(ptr, "%d\n", pgm->max_val);

    switch (pgm->type[1])
    {
    case '2':
        for (int i = 0; i < pgm->height; i++)
        {
            for (int j = 0; j < pgm->width; j++)
            {
                fprintf(ptr, "%hhu ", pgm->data[i][j]);
            }
            fprintf(ptr, "\n");
        }
        break;

    case '5':
        for (int i = 0; i < pgm->height; i++)
        {
            fwrite(pgm->data[i], sizeof(unsigned char), pgm->width, ptr);
            if (ferror(ptr))
            {
                fprintf(stderr, "Error: pgm_write() failed to write to file %s\n", filename);
                exit(EXIT_FAILURE);
            }
        }
    default:
        break;
    }

    fclose(ptr);
}

/**
 * @brief Free the memory allocated for the image struct
 * 
 * @param pgm 
 */
void pgm_free(PGM *pgm)
{
    for (int i = 0; i < pgm->height; i++)
    {
        free(pgm->data[i]);
    }

    free(pgm->data);
    free(pgm);
    fprintf(stdout, "pgm_free() freed the PGM image\n");
}

PGM *filter_sobel(PGM *img, char *padding)
{
    PGM *filtered, *sobel_x, *sobel_y;
    int k, i, j;

    double x_max = DBL_MIN;
    double x_min = DBL_MAX;
    double y_max = DBL_MIN;
    double y_min = DBL_MAX;

    short x_sobel[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    short y_sobel[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    double **temp_x = (double **)malloc(sizeof(double *) * img->height - 2);
    double **temp_y = (double **)malloc(sizeof(double *) * img->height - 2);

    for (i = 0; i < img->height - 1; i++)
    {
        temp_x[i] = (double *)malloc(sizeof(double) * img->width - 2);
        temp_y[i] = (double *)malloc(sizeof(double) * img->width - 2);
    }

    if (strcmp(padding, "yes") == 0)
    {
        filtered = pgm_create(img->width, img->height, img->max_val, img->type);
        sobel_x = pgm_create(img->width, img->height, img->max_val, img->type);
        sobel_y = pgm_create(img->width, img->height, img->max_val, img->type);
        k = 1;
    }
    else
    {
        filtered = pgm_create(img->width - 2, img->height - 2, img->max_val, img->type);
        sobel_x = pgm_create(img->width - 2, img->height - 2, img->max_val, img->type);
        sobel_y = pgm_create(img->width - 2, img->height - 2, img->max_val, img->type);
        k = 0;
    }

    for (i = 0; i < img->height - 2; i++)
    {
        for (j = 0; j < img->width - 2; j++)
        {
            double x_sum = 0;
            double y_sum = 0;
            for (int m = 0; m < 3; m++)
            {
                for (int n = 0; n < 3; n++)
                {
                    x_sum += (double)img->data[i + m][j + n] * x_sobel[m][n];
                    y_sum += (double)img->data[i + m][j + n] * y_sobel[m][n];
                }
            }
            temp_x[i][j] = x_sum;
            temp_y[i][j] = y_sum;
            if (x_sum > x_max)
            {
                x_max = x_sum;
            }
            else if (x_sum < x_min)
            {
                x_min = x_sum;
            }

            if (y_sum > y_max)
            {
                y_max = y_sum;
            }
            else if (y_sum < y_min)
            {
                y_min = y_sum;
            }
        }
    }

    for (i = 0; i < img->height - 2; i++)
    {
        for (j = 0; j < img->width - 2; j++)
        {
            sobel_x->data[i + k][j + k] = (unsigned char)(temp_x[i][j] - x_min) * 255 / (x_max - x_min);
            sobel_y->data[i + k][j + k] = (unsigned char)(temp_y[i][j] - y_min) * 255 / (y_max - y_min);
        }
    }

    for (i = 0; i < img->height - 2; i++)
    {
        for (j = 0; j < img->width - 2; j++)
        {
            filtered->data[i + k][j + k] = (unsigned char)(sqrt(pow(sobel_x->data[i + k][j + k], 2) + pow(sobel_y->data[i + k][j + k], 2)));
        }
    }

    for (i = 0; i < img->height - 1; i++)
    {
        free(temp_x[i]);
        free(temp_y[i]);
    }
    free(temp_x);
    free(temp_y);
    pgm_free(sobel_x);
    pgm_free(sobel_y);

    return filtered;
}

/**
 * @brief Apply median filter to the image and return the filtered image
 *        Kernel size must be odd and greater than 1
 *        Median filter is applied to each pixel in the image and write the result to the new image
 *        Check if padding is needed. If needed allocate memory with same size and start filling axis from
 *          (1,1) to (width-1, height-1) else allocate memory with size of (width-1,height-1) start from 
 *          (0,0) to (width, height)
 * @param img 
 * @param filter_size 
 * @param padding 
 * @return PGM* 
 */
PGM *filter_median(PGM *img, int filter_size, char *padding)
{
    PGM *filtered;
    int i, j, k;
    int size = filter_size - 1;

    if (filter_size % 2 == 0 && filter_size > 1)
    {
        fprintf(stderr, "Error: filter_median() filter_size must be odd\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(padding, "yes") == 0)
    {
        filtered = pgm_create(img->width, img->height, img->max_val, img->type);
        k = size / 2;
    }
    else
    {
        filtered = pgm_create(img->width - size, img->height - size, img->max_val, img->type);
        k = 0;
    }

    for (i = 0; i < img->height - size; i++)
    {
        for (j = 0; j < img->width - size; j++)
        {
            filtered->data[i + k][j + k] = find_median(img->data, i, j, filter_size);
        }
    }
    return filtered;
}

/**
 * @brief Find the median of the pixels in the kernel and return it
 *        Uses the bubble sort algorithm to sort the pixels in the kernel
 *        First allocate array of size filter_size*filter_size and fill it with the pixels in the kernel
 *        Then sort the array
 *        Return the median of the array
 *        Free the array
 * 
 * @param array 
 * @param i 
 * @param j 
 * @param size 
 * @return unsigned char 
 */
unsigned char find_median(unsigned char **array, int i, int j, int size)
{
    unsigned char *arr = (unsigned char *)malloc(size * size * sizeof(unsigned char));
    unsigned median;

    for (int k = 0; k < size; k++)
    {
        for (int m = 0; m < size; m++)
        {
            arr[k * size + m] = array[i + k][j + m];
        }
    }

    mergeSort(arr, 0, size * size - 1);
    median = arr[size * size / 2];

    free(arr);
    return median;
}

/**
 * @brief Sort the array using the merge sort algorithm
 *        Divide the array in two halves until the array is of size 1
 *        Complexity: O(n*log(n)) where n is the size of the array
 * 
 * @param arr 
 * @param left 
 * @param right 
 */
void mergeSort(unsigned char *arr, int left, int right)
{
    if (left < right)
    {
        int middle = left + (right - left) / 2;

        mergeSort(arr, left, middle);
        mergeSort(arr, middle + 1, right);

        merge(arr, left, middle, right);
    }
}

/**
 * @brief Merge the two sorted arrays
 *        
 * @param arr 
 * @param left 
 * @param middle 
 * @param right 
 */
void merge(unsigned char *arr, int left, int middle, int right)
{
    int n1 = middle - left + 1;
    int n2 = right - middle;
    int i, j, k;

    unsigned char *L = (unsigned char *)malloc(n1 * sizeof(unsigned char));
    unsigned char *M = (unsigned char *)malloc(n2 * sizeof(unsigned char));

    for (i = 0; i < n1; i++)
    {
        L[i] = arr[left + i];
    }

    for (j = 0; j < n2; j++)
    {
        M[j] = arr[middle + 1 + j];
    }

    i = 0;
    j = 0;
    k = left;

    while (i < n1 && j < n2)
    {
        if (L[i] <= M[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = M[j];
            j++;
        }
        k++;
    }

    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2)
    {
        arr[k] = M[j];
        j++;
        k++;
    }

    free(L);
    free(M);
}

/**
 * @brief Apply average filter to the image and return the filtered image
 *        Kernel size must be odd and greater than 1
 *        Average filter is applied to each pixel in the image and write the result to the new image
 *        Check if padding is needed. If needed allocate memory with same size and start filling axis from
 *          (1,1) to (width-1, height-1) else allocate memory with size of (width-1,height-1) start from
 *          (0,0) to (width, height)
 *        
 * 
 * @param img 
 * @param filter_size 
 * @param padding 
 * @return PGM* 
 */
PGM *filter_average(PGM *img, int filter_size, char *padding)
{
    PGM *filtered;
    int i, j, k;
    int size = filter_size - 1;

    if (filter_size % 2 == 0 && filter_size > 1)
    {
        fprintf(stderr, "Error: filter_average() filter_size must be odd\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(padding, "yes") == 0)
    {
        filtered = pgm_create(img->width, img->height, img->max_val, img->type);
        k = size / 2;
    }
    else
    {
        filtered = pgm_create(img->width - size, img->height - size, img->max_val, img->type);
        k = 0;
    }

    for (i = 0; i < img->height - size; i++)
    {
        for (j = 0; j < img->width - size; j++)
        {
            double sum = 0;
            for (int m = 0; m < filter_size; m++)
            {
                for (int n = 0; n < filter_size; n++)
                {
                    sum += img->data[i + m][j + n];
                }
            }
            filtered->data[i + k][j + k] = (unsigned char)(sum / (filter_size * filter_size));
        }
    }

    return filtered;
}
