#include <stdio.h>
#include <stdlib.h>

int main(void) {
  puts("before invalid access");
  int *p = NULL;
  *p = 0;
  puts("after invalid access");
}
