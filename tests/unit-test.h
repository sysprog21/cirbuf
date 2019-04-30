#ifndef CU_TEST_H
#define CU_TEST_H

#include <setjmp.h>
#include <stdarg.h>

/* CuString */
typedef struct CuStringInternal CuString;
CuString *CuStringNew(void);
char *CuStringC(CuString *str);

typedef struct CuTestInternal CuTest;
typedef void (*TestFunction)(CuTest *);

void CuTestInit(CuTest *t, const char *name, TestFunction function);
CuTest *CuTestNew(const char *name, TestFunction function);
void CuTestRun(CuTest *tc);

/* Internal versions of assert functions -- use the public versions */
void CuFail_Line(CuTest *tc,
                 const char *file,
                 int line,
                 const char *message2,
                 const char *message);
void CuAssert_Line(CuTest *tc,
                   const char *file,
                   int line,
                   const char *message,
                   int condition);

/* public assert functions */

#define CuFail(tc, ms) CuFail_Line((tc), __FILE__, __LINE__, NULL, (ms))
#define CuAssert(tc, ms, cond) \
    CuAssert_Line((tc), __FILE__, __LINE__, (ms), (cond))
#define CuAssertTrue(tc, cond) \
    CuAssert_Line((tc), __FILE__, __LINE__, "assert failed", (cond))

/* CuSuite */

#define MAX_TEST_CASES 1024

#define SUITE_ADD_TEST(SUITE, TEST) CuSuiteAdd(SUITE, CuTestNew(#TEST, TEST))

typedef struct {
    int count;
    CuTest *list[MAX_TEST_CASES];
    int failCount;
} CuSuite;

void CuSuiteInit(CuSuite *testSuite);
CuSuite *CuSuiteNew(void);
void CuSuiteAdd(CuSuite *testSuite, CuTest *testCase);
void CuSuiteAddSuite(CuSuite *testSuite, CuSuite *testSuite2);
void CuSuiteRun(CuSuite *testSuite);
void CuSuiteSummary(CuSuite *testSuite, CuString *summary);
void CuSuiteDetails(CuSuite *testSuite, CuString *details);

#endif /* CU_TEST_H */
