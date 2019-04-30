#include <assert.h>
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unit-test.h"

char *CuStrAlloc(int size);
char *CuStrCopy(const char *old);

#define CU_ALLOC(TYPE) ((TYPE *) malloc(sizeof(TYPE)))

#define HUGE_STRING_LEN 8192
#define STRING_MAX 256
#define STRING_INC 256

struct CuStringInternal {
    int length;
    int size;
    char *buffer;
};

char *CuStringC(CuString *str)
{
    if (str)
        return str->buffer;
    return NULL;
}

struct CuTestInternal {
    const char *name;
    TestFunction function;
    int failed;
    int ran;
    const char *message;
    jmp_buf *jumpBuf;
};

char *CuStrAlloc(int size)
{
    char *newStr = (char *) malloc(sizeof(char) * (size));
    return newStr;
}

char *CuStrCopy(const char *old)
{
    int len = strlen(old);
    char *newStr = CuStrAlloc(len + 1);
    strcpy(newStr, old);
    return newStr;
}

void CuStringInit(CuString *str)
{
    str->length = 0;
    str->size = STRING_MAX;
    str->buffer = (char *) malloc(sizeof(char) * str->size);
    str->buffer[0] = '\0';
}

CuString *CuStringNew(void)
{
    CuString *str = (CuString *) malloc(sizeof(CuString));
    str->length = 0;
    str->size = STRING_MAX;
    str->buffer = (char *) malloc(sizeof(char) * str->size);
    str->buffer[0] = '\0';
    return str;
}

void CuStringResize(CuString *str, int newSize)
{
    str->buffer = (char *) realloc(str->buffer, sizeof(char) * newSize);
    str->size = newSize;
}

void CuStringAppend(CuString *str, const char *text)
{
    if (!text)
        text = "NULL";

    int length = strlen(text);
    if (str->length + length + 1 >= str->size)
        CuStringResize(str, str->length + length + 1 + STRING_INC);
    str->length += length;
    strcat(str->buffer, text);
}

void CuStringAppendChar(CuString *str, char ch)
{
    char text[2] = {ch, '\0'};
    CuStringAppend(str, text);
}

void CuStringAppendFormat(CuString *str, const char *format, ...)
{
    va_list argp;
    char buf[HUGE_STRING_LEN];
    va_start(argp, format);
    vsprintf(buf, format, argp);
    va_end(argp);
    CuStringAppend(str, buf);
}

void CuStringInsert(CuString *str, const char *text, int pos)
{
    int length = strlen(text);
    if (pos > str->length)
        pos = str->length;
    if (str->length + length + 1 >= str->size)
        CuStringResize(str, str->length + length + 1 + STRING_INC);
    memmove(str->buffer + pos + length, str->buffer + pos,
            (str->length - pos) + 1);
    str->length += length;
    memcpy(str->buffer + pos, text, length);
}

void CuTestInit(CuTest *t, const char *name, TestFunction function)
{
    t->name = CuStrCopy(name);
    t->failed = 0;
    t->ran = 0;
    t->message = NULL;
    t->function = function;
    t->jumpBuf = NULL;
}

CuTest *CuTestNew(const char *name, TestFunction function)
{
    CuTest *tc = CU_ALLOC(CuTest);
    CuTestInit(tc, name, function);
    return tc;
}

void CuTestRun(CuTest *tc)
{
    printf(" running %s\n", tc->name);

    jmp_buf buf;
    tc->jumpBuf = &buf;
    if (setjmp(buf) == 0) {
        tc->ran = 1;
        (tc->function)(tc);
    }
    tc->jumpBuf = 0;
}

static void CuFailInternal(CuTest *tc,
                           const char *file,
                           int line,
                           CuString *string)
{
    char buf[HUGE_STRING_LEN];

    sprintf(buf, "%s:%d: ", file, line);
    CuStringInsert(string, buf, 0);

    tc->failed = 1;
    tc->message = string->buffer;
    if (tc->jumpBuf != 0)
        longjmp(*(tc->jumpBuf), 0);
}

void CuFail_Line(CuTest *tc,
                 const char *file,
                 int line,
                 const char *message2,
                 const char *message)
{
    CuString string;

    CuStringInit(&string);
    if (message2) {
        CuStringAppend(&string, message2);
        CuStringAppend(&string, ": ");
    }
    CuStringAppend(&string, message);
    CuFailInternal(tc, file, line, &string);
}

void CuAssert_Line(CuTest *tc,
                   const char *file,
                   int line,
                   const char *message,
                   int condition)
{
    if (condition)
        return;
    CuFail_Line(tc, file, line, NULL, message);
}

void CuSuiteInit(CuSuite *testSuite)
{
    testSuite->count = 0;
    testSuite->failCount = 0;
}

CuSuite *CuSuiteNew(void)
{
    CuSuite *testSuite = CU_ALLOC(CuSuite);
    CuSuiteInit(testSuite);
    return testSuite;
}

void CuSuiteAdd(CuSuite *testSuite, CuTest *testCase)
{
    assert(testSuite->count < MAX_TEST_CASES);
    testSuite->list[testSuite->count] = testCase;
    testSuite->count++;
}

void CuSuiteAddSuite(CuSuite *testSuite, CuSuite *testSuite2)
{
    for (int i = 0; i < testSuite2->count; ++i) {
        CuTest *testCase = testSuite2->list[i];
        CuSuiteAdd(testSuite, testCase);
    }
}

void CuSuiteRun(CuSuite *testSuite)
{
    for (int i = 0; i < testSuite->count; ++i) {
        CuTest *testCase = testSuite->list[i];
        CuTestRun(testCase);
        if (testCase->failed) {
            testSuite->failCount += 1;
        }
    }
}

void CuSuiteSummary(CuSuite *testSuite, CuString *summary)
{
    for (int i = 0; i < testSuite->count; ++i) {
        CuTest *testCase = testSuite->list[i];
        CuStringAppend(summary, testCase->failed ? "F" : ".");
    }
    CuStringAppend(summary, "\n\n");
}

void CuSuiteDetails(CuSuite *testSuite, CuString *details)
{
    if (testSuite->failCount == 0) {
        int passCount = testSuite->count - testSuite->failCount;
        const char *testWord = passCount == 1 ? "test" : "tests";
        CuStringAppendFormat(details, "OK (%d %s)\n", passCount, testWord);
        return;
    }

    if (testSuite->failCount == 1)
        CuStringAppend(details, "There was 1 failure:\n");
    else
        CuStringAppendFormat(details, "There were %d failures:\n",
                             testSuite->failCount);

    for (int i = 0, failCount = 0; i < testSuite->count; ++i) {
        CuTest *testCase = testSuite->list[i];
        if (testCase->failed) {
            failCount++;
            CuStringAppendFormat(details, "%d) %s: %s\n", failCount,
                                 testCase->name, testCase->message);
        }
    }
    CuStringAppend(details, "\n!!!FAILURES!!!\n");

    CuStringAppendFormat(details, "Runs: %d ", testSuite->count);
    CuStringAppendFormat(details, "Passes: %d ",
                         testSuite->count - testSuite->failCount);
    CuStringAppendFormat(details, "Fails: %d\n", testSuite->failCount);
}
