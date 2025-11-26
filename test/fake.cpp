extern "C" int fake_func(double *);

int fake_func(double *value) {
    if(value != nullptr) { *value = -*value; }
    return -1;
}
