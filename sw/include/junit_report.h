/**
 * @file        TimerSrv_priv.h
 * @brief       This file implements the timerSrv provider
 * @author      Bob.Xu
 * @date        2015-10-15
 * @copyright   Tymphany Ltd.
 */
#ifndef JUNIT_REPORT_H
#define JUNIT_REPORT_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct testCasePassReport
{
    char testCaseName[30];
}testCasePassReport;

typedef struct testCaseFailReport
{
    char testCaseName[30];
    char failMessage[30];
}testCaseFailReport;

void addPassTestCase(testCasePassReport * testCase);
void addFailTestCase(testCaseFailReport * testCase);
void InitJunitXmlReport();
void InittestCasePass(testCasePassReport * testCase);
void InittestCaseFail(testCaseFailReport * testCase);
char* makeJunitXmlReport(char *testName);

#endif /* JUNIT_REPORT_H */

