/*
	Name: TB程序解释器
	Author: 许东伟3190102201
	Date: 2020/04/08 13:01
	Description:
      *基本要求  --完善程序IO，新增输入(!)语法
                 --添加减法(-)语句 
                 --允许行内任意位置添加空格
                 --跳转的行号不存在将向后顺延，可不加END结束程序 
	  *新增功能  --添加乘(*)除(/)赋值(=)以及取模(%)运算
                 --支持变量之间的运算     如A+B
                 --支持条件语句变量之间比较      如IF A=B 
                 --自动检查语法，出现违规语法将报错并提示出错行数
    Demo:  --功能 求1-N(N为待输入的整数)内所有素数(TB程序实现) 
        2  N !
        4  A=2
        5  IF N=2 GO 80
        6  A?
        10 IF A = N GO 80
        12 A+1 
        14 I=2
        20 D = A
        22 D%I
        26 IF D = 0 GO 10
        28 I+1
        30 IF A = I GO 110
        32 GO 20
        80 END
        110 A ?
        112 GO 10
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define MAX_CHAR 20        //每行最多字符数 
#define MAX_INDEXS 100     //最多行数
#define MAX_LINES 3000     //最大行号
#define MAX_VAR	26         //变量个数 
#define MAX_DGT 4          //规定数字最多只能四位数
#define MAX_TIME 10        //设定程序最多执行0.01s(约执行十万次语句) 

#define DIGIT 10         //数字均为十进制
#define GAP_IF 6         //条件语句中到GO后数字大致在第几位 如:IFA=5GO10
#define OK 1
#define NINE '9'
#define ZERO '0'
#define BASIC 'A'
#define CHAR(c) ('A'<=(c)&&'Z'>=(c)? OK: 0) 

typedef struct _str {
	char *pString;             //存放字符
	int sLine;                 //存放行号
} str;

str m_Str[MAX_INDEXS];        //存放语句
int m_sIndexs;                //记录程序总行数
int m_sVar[MAX_VAR];          //变量储存值
FILE *fpI, *fpO;
clock_t _start, _pause;       //计时
/***过程向函数***/
void Ini();                     //初始化全局变量
void RdFile();                  //读取文件(read)
void Sort();                    //分类
int Check(int sIndex);          //检查
int Solve(int sIndex);          //解释语法
int Judge(int sIndex);          //条件语句
int LookUp(int sLine);          //转移语句（返回值是下一行是第几条语句） 
void WrtFile(int num);          //输出文件(write)
void Input(int *p);             //输入数值
/***工具向函数***/     //此类函数不必细研，了解其功能及接口即可 
int GetNum(char *p, int sGap);    //返回数字/变量的值
int Num(char *p);                 //返回数字的值 
int *GetVar(char *p, int sGap);   //返回变量的地址
int CntNum(char *p);              //返回数字的位数(count)
void RmChar(char *p);             //删除一个字符(remove)
void SwapStr(str *p, str *q);     //交换两个结构体
int _if(char *p);               //是条件语句：OK     否：0 
int _go(char *p);               //是转移语句：OK     否：0 
int _sign(char *p);             //含符号语句：OK     否：0  
void ShowErr(const char *pMsg, int sLine);  //展示错误信息并退出
int main() {
	/*初始化->读入->整理->检查语法->解释(loop)->结束*/
	Ini();
	RdFile();
	Sort();
	Check(0);
	int sIndex;
	for(sIndex=0; sIndex<m_sIndexs; sIndex++)
        sIndex = Solve(sIndex);
}
/*初始化*/
void Ini() {
	int sIndex;
	for (sIndex=0; MAX_VAR>sIndex; sIndex++)
		m_sVar[sIndex] = 0;
	_start = clock();
}
/*读入语句*/
void RdFile() {
	fpI=fopen("input.txt", "r"), fpO=fopen("output.txt", "w");
	if (NULL==fpI)              //文件不存在或打开失败 
		ShowErr("Open fail or file doesn't exist", -1);
	//读取内容 
	int sIndex;
	for (sIndex=0; 0==feof(fpI); sIndex++) {                         
		m_Str[sIndex].pString = (char*)malloc(MAX_CHAR);
		fscanf(fpI, "%d", &m_Str[sIndex].sLine);
		fgets(m_Str[sIndex].pString, MAX_CHAR, fpI);
	}
	fclose(fpI);
	sIndex -= (0==m_Str[sIndex-1].sLine)? 1: 0;    //处理最后一行是空行的情况 
	m_sIndexs = sIndex;
}
/*整理语句*/
void Sort() {
	//冒泡排序
	int sIndex, sLarger;
	for (sLarger=m_sIndexs-1; 0<sLarger; sLarger--)
		for (sIndex=0; sLarger>sIndex; sIndex++)
			if (m_Str[sIndex].sLine>m_Str[sIndex+1].sLine)
				SwapStr(&m_Str[sIndex], &m_Str[sIndex+1]); //交换结构 
	//去除空格和回车
	for (sIndex=0; sIndex<m_sIndexs; sIndex++) {
		int sActive;          //所在字符串的位置
		for (sActive=0; '\0'!=m_Str[sIndex].pString[sActive]; sActive++)
            if (' '==m_Str[sIndex].pString[sActive]
			   ||'\n'==m_Str[sIndex].pString[sActive])
               RmChar(&m_Str[sIndex].pString[sActive--]);  //删除该字符 
    }
}
/*检查语法*/
int Check(int sIndex) {
    char *p = m_Str[sIndex].pString;
    if (0==strcmp(p, "END")|| _go(p)|| _sign(p)|| _if(p))  //是其中一种语句即可 
        return (sIndex+1==m_sIndexs)? 0 : Check(sIndex+1);
    ShowErr("There are some problems with grammer", m_Str[sIndex].sLine);
}
/*解释语句*/ 
int Solve(int sIndex) {
	char cSec = m_Str[sIndex].pString[1];  //second char
	char *pStr = m_Str[sIndex].pString;    //current string
    if ('+'==cSec)                                       //加法语句
        *GetVar(pStr, 0) += GetNum(pStr, 2);
	else if ('-'==cSec)                                  //减法语句
        *GetVar(pStr, 0) -= GetNum(pStr, 2);
	else if ('*'==cSec)                                  //乘法语句
        *GetVar(pStr, 0) *= GetNum(pStr, 2);
	else if ('/'==cSec)                                  //除法语句
        *GetVar(pStr, 0) /= GetNum(pStr, 2);
	else if ('%'==cSec)                                  //取模语句
        *GetVar(pStr, 0) %= GetNum(pStr, 2);
	else if ('='==cSec)                                  //赋值语句
        *GetVar(pStr, 0)  = GetNum(pStr, 2);
	else if ('O'==cSec)                                  //转移语句
        sIndex = LookUp( Num(pStr) );  
	else if ('F'==cSec)                                  //条件语句
        sIndex = Judge(sIndex);
	else if ('?'==cSec)                                  //输出语句
        WrtFile( *GetVar(pStr, 0) );
    else if ('!'==cSec)                                  //输入语句
        Input( GetVar(pStr, 0) );
	else if ('N'==cSec)                                  //结束语句
        sIndex = m_sIndexs;
    return sIndex;
}
/*输出语句*/
void WrtFile(int num) {
	fprintf(fpO, "%d", num);
}
/*输入语句*/ 
void Input(int *p){
	_pause = clock();
	printf("input '%c':", p-m_sVar+BASIC);
	scanf("%d", p);
	_start += clock()-_pause;
}
/*条件语句*/
int Judge(int sIndex) {
    char *p = m_Str[sIndex].pString;
    int sNum = CHAR(*(p+4))? *GetVar(p, 4): Num(p);
    if (*GetVar(p, 2)==sNum)                 //条件为真
        return LookUp(Num( p+GAP_IF+CntNum(p+4) ));    
    else                                     //条件为伪
        return sIndex;
}
/*转移语句*/
int LookUp(int sLine) {
	if (MAX_TIME<clock()-_start)               //检查是否死循环 
	    ShowErr("Endless loop",-1); 
    int sIndex = sLine>m_Str[m_sIndexs-1].sLine? m_sIndexs: 0;       //正常为零 
    while (m_sIndexs!=sIndex&&m_Str[sIndex].sLine<sLine)  //寻找行号所对应的语句
        sIndex++;
	return sIndex-1;   //返回语句在内存的位置 
}
/************以下函数均为工具型函数，便于过程函数直接调用**************/ 
/*读出数字*/
int Num(char *p) {
    int result;
    while (ZERO>*p || NINE<*p)          //找到数字首位
        p++;
    for (result=0; ZERO<=*p && NINE>=*p; p++)
        result = result*DIGIT + *p - ZERO;
    return result;
}
int _sign(char *p) {
    if (CHAR(*p)&&('!'==*(p+1)||'?'==*(p+1))&&'\0'==*(p+2))
        return OK;
    if (CHAR(*p)&&(CHAR(*(p+2))||(0<CntNum(p+2)&&MAX_DGT>=CntNum(p+2))) )
        if ('+'==*(p+1)||'-'==*(p+1)||'*'==*(p+1)
           ||'/'==*(p+1)||'%'==*(p+1)||'='==*(p+1))
            return OK;
    return 0;
}
int _if(char *p) {
	if (0==strncmp(p, "IF", 2)&& CHAR(*(p+2))&& '='==*(p+3)
        &&(CHAR(*(p+4))||(0<CntNum(p+4)&& MAX_DGT>=CntNum(p+4)) ))
        return _go( p+4+(CHAR(*(p+4))?1:CntNum(p+4)) );
    return 0;
}
int _go(char *p) {
    if (0==strncmp(p, "GO", 2)&& 0<CntNum(p+2)&& MAX_DGT>=CntNum(p+2))
        return OK;
    return 0;
}
int CntNum(char *p) {/*返回数字的位数*/ 
    return (ZERO<=*p&&NINE>=*p)? CntNum(p+1)+OK: 0;
}
void SwapStr(str *p, str *q) {/*交换结构体*/
	str stTemp = *q;
	*q = *p;
	*p = stTemp;
}
void RmChar(char *p) {/*删除首个字符*/
	for (; '\0'!=*p; p++)
		*p = *(p+1);
}
int GetNum(char *p, int sGap) {/*返回变量或数字的值*/ 
	if ('A'<=p[sGap] && 'Z'>=p[sGap])
		return *GetVar(p, sGap);
	else
		return Num(p);
}
int *GetVar(char *p, int sGap) {/*获取变量地址*/ 
	return m_sVar+p[sGap]-BASIC;
}
void ShowErr(const char *pMsg, int sLine) {/*报错*/
    fclose(fpO);
    fpO = fopen("output.txt", "w");
    fprintf(fpO,"-1");
	printf("\n[error] %s\n\tlies in line: %d\n", pMsg, sLine);
	system("pause");
	exit(0);
}
