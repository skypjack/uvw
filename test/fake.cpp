#include <stdio.h>

extern "C" int fake_func(double *);

int fake_func(double *d) {
    if(d) { *d = -*d; }
    return -42;
}
