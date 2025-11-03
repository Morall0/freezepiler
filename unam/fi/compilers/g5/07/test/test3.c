#include <stdio.h>

int main() {
    printf("Hello World!");
    int a;
    int b;
    int c, d;
    a = 1;
    b = 2;
    c = a + b;
    d = a + b * c;
    printf("%d + %d = %d", a, b, c);
    printf("%d + %d * %d = %d", a, b, c, d);
    return 0;
}

