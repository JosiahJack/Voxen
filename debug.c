#include <stdio.h>
#include "debug.h"

void print_bytes_no_newline(int count) {
    printf("%d bytes | %f kb | %f Mb",count,(float)count / 1000.0f,(float)count / 1000000.0f);
}
