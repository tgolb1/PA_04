#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

typedef struct buffer_item {
    uint8_t data[30];       // Array to hold 30 bytes of data
    uint16_t cksum;         // Checksum
} BUFFER_ITEM;

#define BUFFER_SIZE 10 //Define the size of the buffer


#endif
