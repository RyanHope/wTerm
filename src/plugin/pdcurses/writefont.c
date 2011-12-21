#include "deffont.h"
#include <stdio.h>

FILE * fh;

void main(){

    int fsize, x;

    fh = fopen("out.bmp", "w");
    fsize = sizeof(deffont);
    for(x = 0; x < fsize; x++) {
        fwrite(deffont, fsize, 1, fh);
    }
    fclose(fh);
}
