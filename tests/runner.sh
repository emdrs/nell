#!/bin/bash

nell="./build/nell"
passed=0
failed=0

for dir in tests/cases/*/; do
    name=$(basename "$dir")
    input="$dir/input.nell"

    [ -f "$input" ] || continue

    if [ -f "$dir/expected_error.txt" ]; then
        output=$("$nell" "$input" 2>&1)
        expected_err=$(head -1 "$dir/expected_error.txt")
        rm -f out.c
        if echo "$output" | grep -qF "$expected_err"; then
            echo "PASSOU: $name"
            passed=$((passed+1))
        else
            echo "FALHOU: $name (erro esperado nao encontrado)"
            failed=$((failed+1))
        fi
        continue
    fi

    output=$("$nell" "$input" 2>&1)
    rc=$?

    if [ ! -f out.c ]; then
        echo "FALHOU: $name (compilador falhou)"
        [ -n "$output" ] && echo "$output"
        failed=$((failed+1))
        rm -f out.c
        continue
    fi

    if [ ! -f "$dir/expected.c" ]; then
        echo "PULA: $name (sem expected.c)"
        rm -f out.c
        continue
    fi

    if diff -q "$dir/expected.c" out.c >/dev/null 2>&1; then
        echo "PASSOU: $name"
        passed=$((passed+1))
    else
        echo "FALHOU: $name"
        diff -u "$dir/expected.c" out.c
        failed=$((failed+1))
    fi

    rm -f out.c
done

echo "---"
echo "$passed passaram, $failed falharam"
[ $failed -gt 0 ] && exit 1
