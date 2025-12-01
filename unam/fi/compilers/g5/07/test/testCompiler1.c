int funcion1 (int x, int y) {
  if (x < 10) {
    y = y * 2;
  }
  return x * y;
}


int main() {
  int lol = funcion1(20, 5);
  printf("Test 1: %i", lol+5);
  return lol + 5;
}

