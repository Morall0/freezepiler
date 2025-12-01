int main() {
    int j = 0;
    int var = 0;
    for (j = 0; j <= 10; j += 1) {
        var += j;
    }
    printf("Test 4: %i", var);
    return var; // Expect 55
}
