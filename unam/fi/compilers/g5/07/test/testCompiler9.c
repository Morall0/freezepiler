int test_expresiones_complejas(int a, int b, int c) {
    return (a + b) * c - (a / b) + (a % c) * (b << 2);// | (c & 0xFF);
}

// ===== MULTIPLES DECLARACIONES =====
int test_multiple_declarations(int x) {
    int a = x, b = x * 2, c = x * 3;
    int d = a + b + c;
    a += b;
    b -= c;
    c *= d;
    return a + b + c + d;
}

// ===== CONDICIONALES ANIDADAS =====
int test_condicionales_anidadas(int a, int b, int c) {
    if (a > b) {
        if (b > c) return 1;
        if (a > c) return 2;
        return 3;
    } else {
        if (a > c) return 4;
        if (b > c) return 5;
        return 6;
    }
}


int main() {
 
    if (test_expresiones_complejas(10, 5, 3) != 63) return 24;
    // Test 11
    if (test_multiple_declarations(2) != 88) return 25;

    // Test 12
    if (test_condicionales_anidadas(10, 5, 3) != 1) return 26;
    if (test_condicionales_anidadas(10, 3, 5) != 2) return 27;
    if (test_condicionales_anidadas(5, 10, 3) != 4) return 28;
    if (test_condicionales_anidadas(3, 10, 5) != 5) return 29;
    if (test_condicionales_anidadas(3, 5, 10) != 6) return 30;
  return 0;

}
