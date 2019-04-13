#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void throw(void) {
    perror(strerror(errno));
    exit(EXIT_FAILURE);
}