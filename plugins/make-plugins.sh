#!/bin/bash
set -x
for FILE in *.c; do
    base=$(echo $FILE | cut -d. -f1)
    gcc -shared -o ${base}.dylib -I../ -I../http-parser/ -I../libev/include/ ../martin.dylib ${base}.c
done
