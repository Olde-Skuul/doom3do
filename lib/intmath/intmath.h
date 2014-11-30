/**********************************

	Calls to the integer math toolbox

**********************************/

#ifndef __BURGER__
#include <BURGER.h>
#endif

#ifndef __INTMATH__
#define __INTMATH__

#ifdef __cplusplus
extern "C" {
#endif

/* Error Codes */

#define IMBadInptParam 0x0B01	/* Bad input parameter */
#define IMIllegalChar 0x0B02	/* Illegal character in string */
#define IMOverflow 0x0B03		/* Integer or long integer overflow */
#define IMStrOverflow 0x0B04	/* String overflow */

/* Limit Values */

#define minLongint 0x80000000	/* Minimum negative signed long integer */
#define minFrac 0x80000000		/* Pinned value for negative Frac overflow */
#define minFixed 0x80000000		/* Pinned value for negative Fixed overflow */
#define minInt 0x8000			/* Minimum negative signed integer */
#define maxInt 0x7FFF			/* Maximum positive signed integer */
#define maxUInt 0xFFFFU			/* Maximum positive unsigned integer */
#define maxLongint 0x7FFFFFFFL	/* Maximum positive signed Longint */
#define maxFrac 0x7FFFFFFFL		/* Pinned value for positive Frac overflow */
#define maxFixed 0x7FFFFFFFL	/* Pinned value for positive Fixed overflow */
#define maxULong 0xFFFFFFFFUL	/* Maximum unsigned Long */

typedef struct {
	int quotient;		/* Quotient from IMSDivide */
	int remainder;		/* Remainder from IMSDivide */
} IntDivRec;

typedef struct {
	long quotient;	/* Quotient from IMLongDiv */
	long remainder;	/* Remainder from IMLongDiv */
} LongDivRec;

typedef struct {
	long lsResult;	/* Low 2 words of product */
	long msResult;	/* High 2 words of product */
} LongMulRec;

typedef struct {
	Word quotient;		/* Quotient from IMUDivide */
	Word remainder;		/* Remainder from IMUDivide */
} WordDivRec;

extern void IMBootInit(void);
extern void IMStartUp(void);
extern void IMShutDown(void);
extern Word IMVersion(void);
extern void IMReset(void);
extern Boolean IMIMStatus(void);
extern int IMDec2Int(char *,Word, Boolean);
extern long IMDec2Long(char *, Word, Boolean);
extern Frac Fix2Frac(Fixed);
extern long Fix2Long(Fixed);
extern void IMFix2X(Fixed,extended *);
extern Fixed IMFixATan2(long,long);
extern Fixed IMFixDiv(Fixed Numerator,Fixed Denominator);
extern Fixed IMFixMul(Fixed InputA,Fixed InputB);
extern Fixed IMFixRatio(int,int);
extern int IMFixRound(Fixed);
extern Fixed IMFrac2Fix(Frac);
extern void IMFrac2X(Frac, extended *);
extern Frac IMFracCos(Fixed);
extern Frac IMFracDiv(long,long);
extern Frac IMFracMul(Frac, Frac);
extern Frac IMFracSin(Fixed);
extern Frac IMFracSqrt(Frac);
extern Word IMHex2Int(char *, Word);
extern LongWord IMHex2Long(char *, Word);
extern LongWord IMHexIt(Word);
extern Word IMHiWord(LongWord Input);
extern void IMInt2Dec(int,char *,Word,Boolean);
extern void IMInt2Hex(Word, char *, Word);
extern void IMLong2Dec(long,char *, Word, Boolean);
extern Fixed IMLong2Fix(long);
extern void IMLong2Hex(LongWord,char *, Word);
extern LongDivRec IMLongDivide(long,long);
extern LongMulRec IMLongMul(long,long);
extern Word IMLoWord(LongWord Input);
extern LongWord IMMultiply(Word InputA,Word InputB);
extern Word IMSDivide(int Numerator,int Denominator,IntDivRec *Result);
extern Word IMUDivide(Word Numerator,Word Denominator,WordDivRec *Result);
extern Fixed IMX2Fix(extended *);
extern Frac IMX2Frac(extended *);

#ifdef __cplusplus
};
#endif
#endif
