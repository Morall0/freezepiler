#!/bin/bash

# Colores para que se vea bonito
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Definimos los tests y sus resultados esperados según tus pruebas con GCC
# Formato: "NombreArchivo:ResultadoEsperado"
TESTS=(
    "testCompiler1.c:105"
    "testCompiler2.c:99"
    "testCompiler3.c:3"
    "testCompiler4.c:55"
    "testCompiler5.c:5"
    "testCompiler6.c:0"
    "testCompiler7.c:0"
    "testCompiler8.c:0"
    "testCompiler9.c:0"
    "testCompiler10.c:0"
)

echo -e "${CYAN}=========================================${NC}"
echo -e "${CYAN}    INICIANDO SUITE DE PRUEBAS AUTOMÁTICA    ${NC}"
echo -e "${CYAN}=========================================${NC}"

PASS_COUNT=0
TOTAL_TESTS=${#TESTS[@]}

for test_case in "${TESTS[@]}"; do
    # Separar el nombre del archivo y el resultado esperado
    FILE="${test_case%%:*}"
    EXPECTED="${test_case##*:}"
    SOURCE_PATH="../test/$FILE"

    echo -n "Probando $FILE... "

    # 1. Limpieza: Borrar ejecutable anterior para evitar falsos positivos
    if [ -f "./program" ]; then
        rm "./program" "./out.o"
    fi

    # 2. Ejecutar tu compilador (Silenciamos el stdout para limpiar la pantalla, pero dejamos stderr)
    ./main "$SOURCE_PATH" > /dev/null

    # 3. Verificar si tu compilador generó 'program'
    if [ ! -f "./program" ]; then
        echo -e "${RED}[ERROR DE COMPILACIÓN]${NC}"
        echo -e "   -> No se generó 'program'."
        continue
    fi

    # 4. Ejecutar el programa generado
    ./program
    EXIT_CODE=$?

    # 5. Comparar resultados
    if [ "$EXIT_CODE" -eq "$EXPECTED" ]; then
        echo -e "${GREEN} [PASÓ]${NC} (Retorno: $EXIT_CODE)"
        ((PASS_COUNT++))
    else
        echo -e "${RED}[FALLÓ]${NC}"
        echo -e "   -> Esperado: $EXPECTED"
        echo -e "   -> Obtenido: $EXIT_CODE"
    fi
done

echo -e "${CYAN}=========================================${NC}"
echo -e "Resultados Finales: ${GREEN}$PASS_COUNT${NC} de ${CYAN}$TOTAL_TESTS${NC} pruebas pasaron."

if [ "$PASS_COUNT" -eq "$TOTAL_TESTS" ]; then
    echo -e "${GREEN}EL COMPILADOR HA PASADO LAS PRUEBAS.${NC}"
else
    echo -e "${YELLOW}REVISAR LOS ERRORES.${NC}"
fi
