// test_comprehensive.c - Test suite completo para el compilador

// ===== FUNCIONES MATEMÁTICAS BÁSICAS =====
int suma(int a, int b) {
    return a + b;
}

int resta(int a, int b) {
    return a - b;
}

int multiplicacion(int a, int b) {
    return a * b;
}

int division(int a, int b) {
    return a / b;
}

int modulo(int a, int b) {
    return a % b;
}

// ===== FUNCIONES CON CONDICIONALES =====
int absoluto(int x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

int maximo(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int minimo(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}

// ===== FUNCIONES CON BUCLES =====
int factorial(int n) {
    int resultado = 1;
    int i = 1;
    for (i = 1; i <= n; i++) {
        resultado = resultado * i;
    }
    return resultado;
}

int suma_arreglo(int n) {
    int total = 0;
    int i = 0;
    for (i = 0; i <= n; i++) {
        total = total + i;
    }
    return total;
}

int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    
    int a = 0;
    int b = 1;
    int temp = 0;
    int i = 2;
    
    for (i = 2; i <= n; i++) {
        temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

// ===== FUNCIONES RECURSIVAS =====
int factorial_recursivo(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial_recursivo(n - 1);
}

int suma_recursiva(int n) {
    if (n <= 0) {
        return 0;
    }
    return n + suma_recursiva(n - 1);
}

// ===== FUNCIONES CON OPERACIONES COMPLEJAS =====
int operaciones_complejas(int x, int y) {
    int resultado = 0;
    
    // Expresiones anidadas
    resultado = (x + y) * (x - y) + (x * y) / 2;
    
    // Múltiples asignaciones
    int temp = resultado;
    resultado = resultado + x * y;
    temp = temp - resultado;
    
    return resultado + temp;
}

// ===== FUNCIÓN MAIN DE PRUEBA MODIFICADA =====
int main() {
    // Test 1: Operaciones básicas
    if (suma(5, 3) != 8) return 1;
    if (resta(10, 4) != 6) return 2;
    if (multiplicacion(3, 4) != 12) return 3;
    if (division(15, 3) != 5) return 4;
    if (modulo(10, 3) != 1) return 5;
    
    // Test 2: Condicionales
    if (absoluto(-5) != 5) return 6;
    if (absoluto(5) != 5) return 7;
    if (maximo(3, 7) != 7) return 8;
    if (minimo(3, 7) != 3) return 9;
    
    // Test 3: Bucles
    if (factorial(5) != 120) return 10;
    if (suma_arreglo(5) != 15) return 11;
    if (fibonacci(6) != 8) return 12;
    
    // Test 4: Recursividad
    if (factorial_recursivo(5) != 120) return 13;
    if (suma_recursiva(5) != 15) return 14;
    
    // Test 5: Operaciones complejas
    if (operaciones_complejas(4, 3) != 13) return 15;
    
    // Si todos los tests pasan, retorna 0
    return 0;
}


