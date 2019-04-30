#!/bin/bash
# Automatically generate single AllTests file for C-based unit test.

FILES=$1

echo \
'
/* This is auto-generated code. */
#include <stdio.h>
#include "unit-test.h"

'

cat $FILES | grep '^void Test' | 
    sed -e 's/(.*$//' \
        -e 's/$/(CuTest*);/' \
        -e 's/^/extern /'

echo \
'
void RunAllTests(void) {
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();

'
cat $FILES | grep '^void Test' | 
    sed -e 's/^void //' \
        -e 's/(.*$//' \
        -e 's/^/    SUITE_ADD_TEST(suite, /' \
        -e 's/$/);/'

echo \
'
    CuSuiteRun(suite);
    CuSuiteDetails(suite, output);
    printf("%s\n", CuStringC(output));
}

int main() {
    RunAllTests();
    return 0;
}
'
