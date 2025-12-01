// test_parte1.c
int test_bits(int a, int b) {
    int resultado = 0;
    resultado = a & b;
    resultado = resultado | a;
    resultado = resultado ^ b;
    resultado = resultado << 2;
    resultado = resultado >> 1;
    return resultado;
}
// test_parte2.c  
int main() {
    return 0;
}
