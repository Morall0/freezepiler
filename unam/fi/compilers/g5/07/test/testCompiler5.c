int funcion2 (int x, int y) {
  int iter = 0;
  if (x < 10) {
      iter = 10;
  }
  else {
      iter = 5;
  }

  int acumulado = 0;
  int i = 0;
  for (i = 1; i <= iter; i++) {
     acumulado += i;

  } 

  return acumulado - y;
}



int main () {
   int x = funcion2(4, 50);
   printf("Test 5: %i", funcion2(4,50));
   return x;
}
