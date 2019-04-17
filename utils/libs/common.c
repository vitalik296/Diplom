//
// Created by Zaharchenko on 17.04.2019.
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "common.h"

void throw(void) {
    perror(strerror(errno));
    exit(EXIT_FAILURE);
}


