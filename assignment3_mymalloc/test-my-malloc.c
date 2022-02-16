/*
 *
 * test-my-malloc.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

char *pointer_to_hex_le(void *);

int
main(int argc, char *argv[])
{
   int *p = malloc(sizeof(int));

   write(1, "pointer_to_hex_le says: 0x", 26);
   write(1, pointer_to_hex_le(p), sizeof(p) * 2);
   write(1, "\n", 1);

   printf("printf says:            0x%016lx\n", (uint64_t) p);
}

char *
pointer_to_hex_le(void *ptr)
{
   static char hex[sizeof(ptr) * 2 + 1];
   char hex_chars[] = "0123456789abcdef";
   int i;
   uint8_t nibble;

   uint64_t mask = 0xf;
   uint64_t shift = 0;

   for(i=sizeof(ptr) * 2 - 1; i>=0; i-=1) {
       nibble = ((uint64_t) ptr & mask) >> shift;
       hex[i] = hex_chars[nibble];

       mask = mask << 4;
       shift += 4;
   }
   hex[sizeof(ptr) * 2] = '\0';

   return hex;
}
