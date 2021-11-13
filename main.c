#include "pgm.h"

int main(int argc, char *argv[])
{

    char filename[30] = "lenaN.pgm";

    if (argc > 1)
    {
        strcpy(filename, argv[1]);
    }

    PGM *pgm = pgm_read(filename);

    PGM *median = filter_median(pgm, 9, "yes");
    PGM *sobel = filter_sobel(pgm, "yes");

    pgm_write(sobel, "test.pgm");

    pgm_free(median);
    pgm_free(sobel);
    pgm_free(pgm);


    return 0;
}