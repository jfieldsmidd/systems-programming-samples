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
  char *p0 = malloc(5);

  write(1, "pointer_to_hex_le(p0) says: 0x", 30);
  write(1, pointer_to_hex_le(p0), sizeof(p0) * 2);
  write(1, "\n", 1);

  printf("printf(p0) says:            0x%016lx\n", (uint64_t) p0);

  free(p0);

  char *p1 = malloc(120);

  write(1, "pointer_to_hex_le(p1) says: 0x", 30);
  write(1, pointer_to_hex_le(p1), sizeof(p1) * 2);
  write(1, "\n", 1);

  printf("printf(p1) says:            0x%016lx\n", (uint64_t) p1);

  char *p2 = malloc(12);

  write(1, "pointer_to_hex_le(p2) says: 0x", 30);
  write(1, pointer_to_hex_le(p2), sizeof(p2) * 2);
  write(1, "\n", 1);

  printf("printf(p2) says:            0x%016lx\n", (uint64_t) p2);

  char *p3 = malloc(776);

  write(1, "pointer_to_hex_le(p3) says: 0x", 30);
  write(1, pointer_to_hex_le(p3), sizeof(p3) * 2);
  write(1, "\n", 1);

  printf("printf(p3) says:            0x%016lx\n", (uint64_t) p3);

  char *p4 = malloc(112);

  write(1, "pointer_to_hex_le(p4) says: 0x", 30);
  write(1, pointer_to_hex_le(p4), sizeof(p4) * 2);
  write(1, "\n", 1);

  printf("printf(p4) says:            0x%016lx\n", (uint64_t) p4);

  char *p5 = malloc(1336);

  write(1, "pointer_to_hex_le(p5) says: 0x", 30);
  write(1, pointer_to_hex_le(p5), sizeof(p5) * 2);
  write(1, "\n", 1);

  printf("printf(p5) says:            0x%016lx\n", (uint64_t) p5);

  char *p6 = malloc(216);

  write(1, "pointer_to_hex_le(p6) says: 0x", 30);
  write(1, pointer_to_hex_le(p6), sizeof(p6) * 2);
  write(1, "\n", 1);

  printf("printf(p6) says:            0x%016lx\n", (uint64_t) p6);

  char *p7 = malloc(432);

  write(1, "pointer_to_hex_le(p7) says: 0x", 30);
  write(1, pointer_to_hex_le(p7), sizeof(p7) * 2);
  write(1, "\n", 1);

  printf("printf(p7) says:            0x%016lx\n", (uint64_t) p7);

  char *p8 = malloc(104);

  write(1, "pointer_to_hex_le(p8) says: 0x", 30);
  write(1, pointer_to_hex_le(p8), sizeof(p8) * 2);
  write(1, "\n", 1);

  printf("printf(p8) says:            0x%016lx\n", (uint64_t) p8);

  char *p9 = malloc(88);

  write(1, "pointer_to_hex_le(p9) says: 0x", 30);
  write(1, pointer_to_hex_le(p9), sizeof(p9) * 2);
  write(1, "\n", 1);

  printf("printf(p9) says:            0x%016lx\n", (uint64_t) p9);

  char *q1 = malloc(120);

  write(1, "pointer_to_hex_le(q1) says: 0x", 30);
  write(1, pointer_to_hex_le(q1), sizeof(q1) * 2);
  write(1, "\n", 1);

  printf("printf(q1) says:            0x%016lx\n", (uint64_t) q1);

  char *q2 = malloc(168);

  write(1, "pointer_to_hex_le(q2) says: 0x", 30);
  write(1, pointer_to_hex_le(q2), sizeof(q2) * 2);
  write(1, "\n", 1);

  printf("printf(q2) says:            0x%016lx\n", (uint64_t) q2);

  char *xx = malloc(20000);

  write(1, "pointer_to_hex_le(xx) says: 0x", 30);
  write(1, pointer_to_hex_le(xx), sizeof(xx) * 2);
  write(1, "\n", 1);

  printf("printf(xx) says:            0x%016lx\n", (uint64_t) xx);
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
