#ifndef CTESTTOOL_H
#define CTESTTOOL_H
#include<string>
#include<windows.h>

class CTestTool
{
public:
    CTestTool();
    static void Dump(const BYTE* Data, size_t nSize);
};

#endif // CTESTTOOL_H
