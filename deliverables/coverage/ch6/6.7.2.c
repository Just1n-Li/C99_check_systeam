/*
 * 验证 C99 6.7.2 Type specifiers
 * 预期行为：
 *   正向测试——编译并运行通过，assert 全部成功；
 *   负向测试——编译报错（违反 6.7.2 约束）。
 */
#include <assert.h>
#include <stdio.h>
#include <complex.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 语法：覆盖 type-specifier 列表中的每一种基本形式 */
void test_syntax_basic(void)
{
    void       *vp = NULL;   (void)vp;
    char        c = 'A';     (void)c;
    short       s = 1;       (void)s;
    int         i = 1;       (void)i;
    long        l = 1;       (void)l;
    float       f = 1.0f;    (void)f;
    double      d = 1.0;     (void)d;
    _Bool       b = 1;       (void)b;
    float       _Complex fc = 1.0f + I;       (void)fc;
    double      _Complex dc = 1.0  + I;       (void)dc;
    long double _Complex ldc = 1.0L + I;      (void)ldc;
    printf("[1] basic type-specifiers: OK\n");
}

/* [2] 约束/语义：合法的逗号分隔集合（同一行多组）逐一验证 */
void test_valid_sets(void)
{
    /* char 系列 */
    char           c1 = 0;  signed char c2 = 0;  unsigned char c3 = 0;
    /* short 系列 */
    short          s1 = 0;  signed short s2 = 0;  short int s3 = 0;
    signed short int s4 = 0;  unsigned short s5 = 0;  unsigned short int s6 = 0;
    /* int 系列 */
    int            i1 = 0;  signed i2 = 0;  signed int i3 = 0;
    unsigned       i4 = 0;  unsigned int i5 = 0;
    /* long 系列 */
    long           l1 = 0;  signed long l2 = 0;  long int l3 = 0;
    signed long int l4 = 0;  unsigned long l5 = 0;  unsigned long int l6 = 0;
    /* long long 系列 */
    long long      ll1 = 0;  signed long long ll2 = 0;  long long int ll3 = 0;
    signed long long int ll4 = 0;  unsigned long long ll5 = 0;
    unsigned long long int ll6 = 0;
    /* float / double / long double */
    float          f1 = 0;  double f2 = 0;  long double f3 = 0;
    /* _Bool */
    _Bool          bb = 0;
    /* _Complex 系列 */
    float       _Complex fc = 0;  double _Complex dc = 0;  long double _Complex ldc = 0;

    (void)c1;(void)c2;(void)c3;(void)s1;(void)s2;(void)s3;(void)s4;(void)s5;(void)s6;
    (void)i1;(void)i2;(void)i3;(void)i4;(void)i5;
    (void)l1;(void)l2;(void)l3;(void)l4;(void)l5;(void)l6;
    (void)ll1;(void)ll2;(void)ll3;(void)ll4;(void)ll5;(void)ll6;
    (void)f1;(void)f2;(void)f3;(void)bb;(void)fc;(void)dc;(void)ldc;
    printf("[2] all valid sets: OK\n");
}

/* [2] 语义：type specifiers may occur in any order, possibly intermixed
       with the other declaration specifiers */
void test_any_order(void)
{
    int long unsigned       a = 1;   /* == unsigned long int */
    long unsigned int       b = 1;   /* == unsigned long int */
    unsigned int long       c = 1;   /* == unsigned long int */
    int signed              d = 1;   /* == signed int */
    long long unsigned      e = 1;   /* == unsigned long long int */
    unsigned long long int  g = 1;   /* == unsigned long long int */

    /* 与 storage-class specifier / type qualifier 混合排列 */
    static unsigned long    x1 = 1;
    long static unsigned    x2 = 1;  /* storage-class 可出现在中间 */
    const int volatile      y1 = 0;
    volatile const unsigned y2 = 0;

    assert(sizeof(a) == sizeof(unsigned long));
    assert(sizeof(b) == sizeof(unsigned long));
    assert(sizeof(c) == sizeof(unsigned long));
    assert(sizeof(d) == sizeof(int));
    assert(sizeof(e) == sizeof(unsigned long long));
    assert(sizeof(g) == sizeof(unsigned long long));
    assert(sizeof(x1) == sizeof(x2));
    (void)y1; (void)y2;
    printf("[2] any order / intermixed: OK\n");
}

/* [5] 语义：Each of the comma-separated sets designates the same type */
void test_same_type(void)
{
    assert(sizeof(short)              == sizeof(signed short));
    assert(sizeof(short)              == sizeof(short int));
    assert(sizeof(short)              == sizeof(signed short int));
    assert(sizeof(unsigned short)     == sizeof(unsigned short int));

    assert(sizeof(int)                == sizeof(signed));
    assert(sizeof(int)                == sizeof(signed int));
    assert(sizeof(unsigned)           == sizeof(unsigned int));

    assert(sizeof(long)               == sizeof(signed long));
    assert(sizeof(long)               == sizeof(long int));
    assert(sizeof(long)               == sizeof(signed long int));
    assert(sizeof(unsigned long)      == sizeof(unsigned long int));

    assert(sizeof(long long)          == sizeof(signed long long));
    assert(sizeof(long long)          == sizeof(long long int));
    assert(sizeof(long long)          == sizeof(signed long long int));
    assert(sizeof(unsigned long long) == sizeof(unsigned long long int));
    printf("[5] comma-separated sets designate same type: OK\n");
}

/* [5] 语义：bit-fields — it is implementation-defined whether int
       designates signed int or unsigned int */
struct BitField {
    int bf : 4;
};

void test_bitfield_int(void)
{
    struct BitField s;
    s.bf = -1;
    /* 实现定义：int 位域可能等同 signed int 或 unsigned int */
    if (s.bf < 0)
        printf("[5] bit-field int == signed int (impl-defined): OK\n");
    else
        printf("[5] bit-field int == unsigned int (impl-defined): OK\n");
}

/* [4] 语义：struct / union / enum / typedef 作为 type-specifier */
struct Point { int x, y; };
union  Bag   { int i; float f; };
enum   Color { RED, GREEN, BLUE };
typedef int MyInt;

void test_aggregate_typedef_specifiers(void)
{
    struct Point p = { 3, 4 };
    union  Bag   u;   u.i = 42;
    enum   Color  cl = GREEN;
    MyInt        mi  = 7;

    assert(p.x == 3 && p.y == 4);
    assert(u.i == 42);
    assert(cl  == GREEN);
    assert(mi  == 7);
    printf("[4] struct/union/enum/typedef as type-specifier: OK\n");
}

/* [2] 约束：struct declaration 的 specifier-qualifier list 中
       也必须至少有一个 type specifier — 正向验证 */
struct WithMember {
    int member;          /* specifier-qualifier list 含 type specifier */
    unsigned bit : 2;    /* 同上 */
};

/* [2] 约束：type name 中也必须至少有一个 type specifier — 正向验证 */
void test_type_name_ok(void)
{
    int   *pi = (int *)0;
    size_t sz = sizeof(unsigned long);
    (void)pi; (void)sz;
    printf("[2] type name / struct decl with type specifier: OK\n");
}

int main(void)
{
    test_syntax_basic();
    test_valid_sets();
    test_any_order();
    test_same_type();
    test_bitfield_int();
    test_aggregate_typedef_specifiers();
    test_type_name_ok();
    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 6.7.2 约束，应编译报错 ========== */
#if 0

/* [2] 违反约束「至少一个 type specifier」：
       声明的 declaration specifiers 中没有任何 type specifier。
       gcc -std=c99 -pedantic 应报 "type specifier missing" 类似错误 */
static x = 1;

/* [2] 违反约束「至少一个 type specifier」：
       struct 声明的 specifier-qualifier list 中没有 type specifier。
       gcc -std=c99 -pedantic 应报错 */
struct BadStruct {
    member;
};

/* [2] 违反约束「至少一个 type specifier」：
       type name 中没有 type specifier。
       gcc -std=c99 -pedantic 应报错 */
void f(void) {
    int *p = (*)0;
}

/* [2] 违反约束「Each list of type specifiers shall be one of the
       following sets」：long float 不在允许的集合中。
       gcc -std=c99 应报 "long float" 不合法 */
long float lf = 1.0;

/* [2] 违反约束：short double 不在允许的集合中。
       gcc -std=c99 应报错 */
short double sd = 1.0;

/* [2] 违反约束：signed float 不在允许的集合中
       （float 已隐含 signed，不允许再加 signed）。
       gcc -std=c99 应报错 */
signed float sf = 1.0;

/* [2] 违反约束：unsigned double 不在允许的集合中。
       gcc -std=c99 应报错 */
unsigned double ud = 1.0;

/* [2] 违反约束：long long long — 三个 long 不在允许的集合中。
       gcc -std=c99 应报错 */
long long long lll = 1;

/* [2] 违反约束：short long 不在允许的集合中。
       gcc -std=c99 应报错 */
short long sl = 1;

/* [2] 违反约束：double int 不在允许的集合中。
       gcc -std=c99 应报错 */
double int di = 1;

/* [2] 违反约束：_Bool long 不在允许的集合中。
       gcc -std=c99 应报错 */
_Bool long bl = 1;

#endif