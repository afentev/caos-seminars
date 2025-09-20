#include <stdio.h>

int get_sign_bit(float number) {
  char* byte = (char*)&number;
  return (byte[3] & 0b10000000) >> 7;
}

int main() {
  printf("%d\n", get_sign_bit(1));
  printf("%d\n", get_sign_bit(-1));
}
