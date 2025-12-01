// ===== OPERADORES LÓGICOS Y DE COMPARACIÓN =====
int test_logicos(int a, int b) {
    if (a == b && a != 0) return 1;
    if (a > b || a < 0) return 2;
    if ((a != b)) return 3;
    return 0;
}

int test_comparaciones(int a, int b) {
    int resultado = 0;
    if (a == b) resultado += 1;
    if (a != b) resultado += 2;
    if (a < b) resultado += 4;
    if (a <= b) resultado += 8;
    if (a > b) resultado += 16;
    if (a >= b) resultado += 32;
    return resultado;
}




// ===== OPERADORES DE ASIGNACIÓN COMPUESTA =====
int test_assign_compuestos(int a, int b) {
    int resultado = a;
    resultado += b;
    resultado -= a;
    resultado *= b;
    resultado /= 2;
    resultado %= 3;/*
    resultado &= 0xFF;
    resultado |= 0x0F;
    resultado ^= 0x55;
    resultado <<= 1;
    resultado >>= 2;*/
    return resultado;
}

// ===== INCREMENTO Y DECREMENTO ===== YA PASO
int test_inc_dec(int a) {
    int b = a;
    b++;
    //++b;
    b--;
    //--b;
    return b;
}


// ===== OPERADOR TERNARIO =====
int test_ternario(int a, int b) {
    return (a > b) ? a : b;
}

int test_ternario_anidado(int a, int b, int c) {
    return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}


// ===== BUCLES =====
int test_while(int n) {
    int i = 0;
    int suma = 0;
    while (i < n) {
        suma += i;
        i++;
    }
    return suma;
}
// HASTA ACA VAMOS


/*
// ===== SWITCH-CASE =====
int test_switch(int a) {
    int resultado = 0;
    switch (a) {
        case 1: resultado = 10; break;
        case 2: resultado = 20; break;
        case 3: resultado = 30; break;
        default: resultado = 0;
    }
    return resultado;
}

int test_switch_sin_break(int a) {
    int resultado = 0;
    switch (a) {
        case 1: resultado += 1;
        case 2: resultado += 2;
        case 3: resultado += 3;
        default: resultado += 4;
    }
    return resultado;
}

// ===== BREAK Y CONTINUE =====
int test_break_continue(int n) {
    int i;
    int suma = 0;
    for (i = 0; i < n; i++) {
        if (i % 2 == 0) continue;
        if (i > 10) break;
        suma += i;
    }
    return suma;
}
*/



// ===== MAIN =====
int main() {

    // Test 1
    if (test_logicos(5, 5) != 1) return 1;
    if (test_logicos(10, 5) != 2) return 2;
    if (test_logicos(0, 0) != 0) return 3;

    // Test 2
    if (test_comparaciones(5, 5) != (1 + 8 + 32)) return 4;
    if (test_comparaciones(3, 7) != (2 + 4 + 8)) return 5;
    if (test_comparaciones(7, 3) != (2 + 16 + 32)) return 6;



    // Test 4
    if (test_assign_compuestos(10, 20) != 2) return 8;

    // Test 5
    if (test_inc_dec(5) != 5) return 9;


    // Test 6
    if (test_ternario(10, 5) != 10) return 11;
    if (test_ternario(5, 10) != 10) return 12;
    if (test_ternario_anidado(10, 5, 8) != 10) return 13;
    if (test_ternario_anidado(5, 10, 8) != 10) return 14;
    if (test_ternario_anidado(5, 8, 10) != 10) return 15;

    // Test 7
    if (test_while(5) != 10) return 16;
/*
    // Test 8
    if (test_switch(1) != 10) return 18;
    if (test_switch(2) != 20) return 19;
    if (test_switch(3) != 30) return 20;
    if (test_switch(4) != 0) return 21;
    if (test_switch_sin_break(1) != (1+2+3+4)) return 22;

    // Test 9
    if (test_break_continue(20) != 25) return 23;

    */


    return 0; // Todo OK
}

