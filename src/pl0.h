#include <stdio.h>

#define norw       11             // no. of reserved words - 保留字的个数
#define txmax      100            // length of identifier table
#define nmax       14             // max. no. of digits in numbers  十进制表示的数字的最大位数
#define al         10             // length of identifiers
#define amax       2047           // maximum address
#define levmax     3              // maximum depth of block nesting
#define cxmax      2000           // size of code array

#define nul	       0x1  //空？？？
#define ident      0x2  //标识符
#define number     0x4  //数字
#define plus       0x8
#define minus      0x10
#define times      0x20
#define slash      0x40
#define oddsym     0x80
#define eql        0x100
#define neq        0x200  //不等于
#define lss        0x400  //小于
#define leq        0x800  //小于等于
#define gtr        0x1000  //大于
#define geq        0x2000  //大于等于
#define lparen     0x4000
#define rparen     0x8000
#define comma      0x10000
#define semicolon  0x20000
#define period     0x40000
#define becomes    0x80000  //赋值
#define beginsym   0x100000
#define endsym     0x200000
#define ifsym      0x400000
#define thensym    0x800000
#define whilesym   0x1000000
#define dosym      0x2000000
#define callsym    0x4000000
#define constsym   0x8000000
#define varsym     0x10000000
#define procsym    0x20000000

enum object {
    constant, variable, proc
};

//最终编译出的字节码的关键字（类似汇编代码）
enum fct {
    lit, opr, lod, sto, cal, Int, jmp, jpc         // functions
};

typedef struct{
    enum fct f;		// function code
    long l; 		// level
    long a; 		// displacement address
} instruction;
/*  lit 0, a : load constant a
    opr 0, a : execute operation a
    lod l, a : load variable l, a
    sto l, a : store variable l, a
    cal l, a : call procedure a at level l
    Int 0, a : increment t-register by a
    jmp 0, a : jump to a
    jpc 0, a : jump conditional to a       */

char ch;               // last character read
unsigned long sym;     // last symbol read  上一次获取的符号是什么类型（具体的哪一个关键字、标识符、数字等）
char id[al+1];         // last identifier read - 上一次获取的标识符
long num;              // last number read
long cc;               // character count  当前已经分析的这一行代码的数量
long ll;               // line length
long kk, err;          // kk-当前分析的标识符等的长度
long cx;               // code allocation index

char line[81];         // a line of code - 一条代码语句
char a[al+1];          // 标识符（identifiers）数组
instruction code[cxmax+1];
char word[norw][al+1];   //保留字数组（reserved words）
unsigned long wsym[norw];  //十六进制表示的保留字
unsigned long ssym[256];  //符号数组，按照符号的ASCII码作为数组下标

char mnemonic[8][3+1];
unsigned long declbegsys, statbegsys, facbegsys;

struct{
    char name[al+1];
    enum object kind;
    long val;
    long level;
    long addr;
}table[txmax+1];

char infilename[80];  // input file name
FILE* infile;  //Input file descriptor

// the following variables for block
long dx;		// data allocation index
long lev;		// current depth of block nesting
long tx;		// current table index

// the following array space for interpreter
#define stacksize 50000
long s[stacksize];	// datastore
