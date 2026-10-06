/*
 * 测试 C99 6.3.2.1 —— Lvalues, arrays, and function designators
 *
 * 预期行为：
 *   正向测试：程序应能编译（gcc -std=c99 -Wall）并运行通过（assert 全部成立）。
 *   负向测试：位于 #if 0 块内，每个片段故意违反 6.3.2.1 相关约束，
 *             若单独取出编译，编译器应报错。
 *
 * 覆盖段落：[1] lvalue / modifiable lvalue 定义
 *           [2] lvalue 转换（值转换、限定符去除、例外情形）
 *           [3] 数组到指针转换（例外情形、非左值结果）
 *           [4] 函数指示符到函数指针转换
 *           Footnote 53 / 54
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* ============================================================
 * 辅助类型与函数
 * ============================================================ */

struct Point { int x; int y; };
struct CPoint { const int x; int y; };   /* 含 const 成员 -> 非 modifiable lvalue */
union  U { int i; float f; };

/* 无原型函数：用于测试默认实参提升（[2] 相关，配合 6.5.2.2） */
int  no_proto_int();          /* 无原型 */
double no_proto_double();     /* 无原型 */

int  no_proto_int()  { return 42; }
double no_proto_double() { return 3.5; }

/* 返回结构体的函数：f().x 不是 lvalue */
struct Point make_point(int x, int y) { struct Point p; p.x = x; p.y = y; return p; }

/* 返回数组指针的函数 */
int (*ret_arr_ptr(void))[3];

static int g_arr[3] = { 10, 20, 30 };
int (*ret_arr_ptr(void))[3] { return &g_arr; }

/* 用于测试函数指示符转换 */
int add(int a, int b) { return a + b; }

/* ============================================================
 * 正向测试：以下代码应能编译并运行通过
 * ============================================================ */
int main(void)
{
    /* ---------- [1] lvalue 定义与 modifiable lvalue ---------- */

    /* [1] 标识符是 lvalue 的典型例子（Footnote 53） */
    int obj = 5;
    int *p = &obj;          /* & 作用于 lvalue */
    assert(*p == 5);        /* *E 是 lvalue，指向 E 所指对象（Footnote 53） */

    /* [1] modifiable lvalue：非数组、非不完整、非 const、结构体无 const 成员 */
    struct Point mp = { 1, 2 };
    mp.x = 10;              /* 可修改 */
    assert(mp.x == 10);

    /* [1] 非 modifiable lvalue：const 限定 */
    const int ci = 7;
    assert(ci == 7);        /* 读取 OK，赋值见负向测试 */

    /* [1] 非 modifiable lvalue：结构体含 const 成员 */
    struct CPoint cp = { 3, 4 };
    cp.y = 40;              /* y 非 const，可改 */
    assert(cp.y == 40);
    /* cp.x = 30;  <-- 见负向测试 */

    /* ---------- [2] lvalue 转换：值转换与限定符去除 ---------- */

    /* [2] 非例外情形下，非数组 lvalue 转换为所存值 */
    int a = 1, b = 2;
    int sum = a + b;        /* a、b 转换为值 */
    assert(sum == 3);

    /* [2] 限定类型 -> 值的类型为无限定版本 */
    const int cval = 99;
    int ival = cval;        /* cval 转换为 int 值（去 const） */
    assert(ival == 99);

    volatile int vval = 11;
    int vcopy = vval;       /* volatile 限定被去除 */
    assert(vcopy == 11);

    /* [2] 例外：sizeof 的操作数不转换 */
    assert(sizeof(obj) == sizeof(int));
    assert(sizeof g_arr == sizeof(int) * 3);   /* 数组不退化 */

    /* [2] 例外：一元 & 的操作数不转换 */
    int *pa = &obj;
    assert(pa == &obj);

    /* [2] 例外：++ / -- 的操作数不转换（保持 lvalue） */
    int inc = 0;
    ++inc;                  /* 前缀 ++ 作用于 lvalue */
    assert(inc == 1);
    inc++;                  /* 后缀 ++ */
    assert(inc == 2);
    --inc;
    assert(inc == 1);

    /* [2] 例外：. 运算符左操作数不转换 */
    struct Point sp = { 5, 6 };
    sp.x = 50;              /* sp 保持 lvalue，成员可赋值 */
    assert(sp.x == 50);

    /* [2] 例外：赋值运算符左操作数不转换 */
    int lhs = 0;
    lhs = 123;              /* lhs 保持 lvalue */
    assert(lhs == 123);

    /* ---------- [3] 数组到指针转换 ---------- */

    /* [3] 数组表达式转换为指向首元素的指针（非 lvalue） */
    int arr[4] = { 1, 2, 3, 4 };
    int *ap = arr;          /* arr 转换为 &arr[0] */
    assert(ap == &arr[0]);
    assert(*ap == 1);
    assert(ap[2] == 3);

    /* [3] 转换结果不是 lvalue：不能对它赋值（见负向测试） */

    /* [3] 例外：sizeof 操作数不转换 */
    assert(sizeof(arr) == sizeof(int) * 4);

    /* [3] 例外：一元 & 操作数不转换，得到指向数组的指针 */
    int (*parr)[4] = &arr;
    assert((*parr)[0] == 1);
    assert(sizeof(*parr) == sizeof(int) * 4);

    /* [3] 例外：字符串字面量用于初始化数组时不转换 */
    char str[] = "hello";   /* 字符串字面量初始化数组，不退化 */
    assert(sizeof(str) == 6);   /* 含 '\0' */
    assert(strcmp(str, "hello") == 0);

    /* [3] 字符串字面量在其他语境下转换为指针 */
    const char *sp2 = "world";
    assert(sp2[0] == 'w');

    /* [3] 多维数组：第一维转换为指针 */
    int mat[2][3] = { {1,2,3}, {4,5,6} };
    int (*pm)[3] = mat;     /* mat 转换为 &mat[0]，类型 int(*)[3] */
    assert(pm[0][0] == 1);
    assert(pm[1][2] == 6);

    /* ---------- [4] 函数指示符到函数指针转换 ---------- */

    /* [4] 函数指示符转换为函数指针 */
    int (*fp)(int, int) = add;
    assert(fp(3, 4) == 7);

    /* [4] 直接调用：函数指示符转换为函数指针后调用 */
    assert(add(10, 20) == 30);

    /* [4] 例外：一元 & 作用于函数指示符，得到函数指针 */
    int (*fp2)(int, int) = &add;
    assert((*fp2)(5, 6) == 11);
    assert(fp2(5, 6) == 11);

    /* [4] 例外：sizeof 的操作数不转换 —— 但 sizeof 函数指示符违反 6.5.3.4 约束，
     *     故此处不测试 sizeof(add)，见负向测试。 */

    /* ---------- Footnote 53 / 54 ---------- */

    /* Footnote 53：*E 是 lvalue，指定 E 所指对象 */
    int target = 77;
    int *pt = &target;
    *pt = 88;               /* *pt 是 lvalue，可赋值 */
    assert(target == 88);

    /* Footnote 54：sizeof 的操作数不发生函数指示符转换，
     * 因此 sizeof(add) 违反 6.5.3.4 约束（见负向测试）。 */

    /* ---------- 无原型函数调用的默认实参提升（配合 6.5.2.2） ---------- */

    /* 无原型函数：实参经默认提升后传递 */
    int r1 = no_proto_int();
    assert(r1 == 42);

    double r2 = no_proto_double();
    assert(r2 == 3.5);

    /* 无原型调用中 char/float 实参提升为 int/double */
    {
        char  ch = 'A';
        float fl = 2.5f;
        /* 无原型调用：ch 提升为 int，fl 提升为 double */
        int  ri = no_proto_int(ch);       /* ch -> int */
        double rd = no_proto_double(fl);  /* fl -> double */
        (void)ri; (void)rd;
    }

    /* ---------- 返回结构体的函数：f().x 不是 lvalue ---------- */

    /* f().x 是成员访问结果，但 f() 不是 lvalue，故 f().x 不是 lvalue */
    int val = make_point(3, 4).x;   /* 读取 OK */
    assert(val == 3);

    /* ---------- 返回数组指针的函数 ---------- */

    int (*rp)[3] = ret_arr_ptr();
    assert((*rp)[0] == 10);
    assert((*rp)[2] == 30);

    /* ---------- 条件/逗号表达式结果不是 lvalue ---------- */

    int x1 = 1, x2 = 2;
    int cond_val = (1 ? x1 : x2);   /* 条件表达式结果不是 lvalue，读取 OK */
    assert(cond_val == 1);

    int comma_val = (x1, x2);       /* 逗号表达式结果不是 lvalue，读取 OK */
    assert(comma_val == 2);

    /* ---------- 强制转换结果不是 lvalue ---------- */

    int cast_val = (int)3.7;        /* 强制转换结果不是 lvalue，读取 OK */
    assert(cast_val == 3);

    printf("All positive tests passed.\n");
    return 0;
}

/* ============================================================
 * 负向测试：以下代码违反 C99 约束，应编译报错
 * ============================================================ */
#if 0

/* ------------------------------------------------------------
 * 违反约束「modifiable lvalue 不能有 const 限定类型」（6.3.2.1 [1]）
 * 期望：gcc -std=c99 报错 "assignment of read-only variable 'ci'"
 * ------------------------------------------------------------ */
void neg_const_assign(void)
{
    const int ci = 7;
    ci = 8;                 /* 错误：ci 不是 modifiable lvalue */
}

/* ------------------------------------------------------------
 * 违反约束「结构体含 const 成员时不是 modifiable lvalue」（6.3.2.1 [1]）
 * 期望：gcc -std=c99 报错 "assignment of read-only member 'x'"
 * ------------------------------------------------------------ */
void neg_const_member_assign(void)
{
    struct CPoint cp = { 3, 4 };
    cp.x = 30;              /* 错误：cp.x 是 const 成员 */
}

/* ------------------------------------------------------------
 * 违反约束「数组不是 modifiable lvalue」（6.3.2.1 [1]）
 * 期望：gcc -std=c99 报错 "assignment to expression with array type"
 * ------------------------------------------------------------ */
void neg_array_assign(void)
{
    int a[3] = { 1, 2, 3 };
    int b[3] = { 4, 5, 6 };
    a = b;                  /* 错误：数组不是 modifiable lvalue */
}

/* ------------------------------------------------------------
 * 违反约束「数组到指针转换结果不是 lvalue」（6.3.2.1 [3]）
 * 期望：gcc -std=c99 报错 "lvalue required as left operand of assignment"
 * ------------------------------------------------------------ */
void neg_array_conv_not_lvalue(void)
{
    int arr[3] = { 1, 2, 3 };
    arr = 0;                /* 错误：arr 转换为指针，结果不是 lvalue */
}

/* ------------------------------------------------------------
 * 违反约束「强制转换结果不是 lvalue」（6.3.2.1 [2] 隐含）
 * 期望：gcc -std=c99 报错 "lvalue required as left operand of assignment"
 * ------------------------------------------------------------ */
void neg_cast_not_lvalue(void)
{
    int x = 1;
    (int)x = 5;             /* 错误：强制转换结果不是 lvalue */
}

/* ------------------------------------------------------------
 * 违反约束「条件表达式结果不是 lvalue」（6.3.2.1 [2] 隐含）
 * 期望：gcc -std=c99 报错 "lvalue required as left operand of assignment"
 * ------------------------------------------------------------ */
void neg_cond_not_lvalue(void)
{
    int a = 1, b = 2;
    (1 ? a : b) = 5;        /* 错误：条件表达式结果不是 lvalue */
}

/* ------------------------------------------------------------
 * 违反约束「逗号表达式结果不是 lvalue」（6.3.2.1 [2] 隐含）
 * 期望：gcc -std=c99 报错 "lvalue required as left operand of assignment"
 * ------------------------------------------------------------ */
void neg_comma_not_lvalue(void)
{
    int a = 1, b = 2;
    (a, b) = 5;             /* 错误：逗号表达式结果不是 lvalue */
}

/* ------------------------------------------------------------
 * 违反约束「函数返回结构体的成员不是 lvalue」（6.3.2.1 [2] 隐含）
 * 期望：gcc -std=c99 报错 "lvalue required as left operand of assignment"
 * ------------------------------------------------------------ */
void neg_func_ret_member_not_lvalue(void)
{
    make_point(1, 2).x = 5; /* 错误：f().x 不是 lvalue */
}

/* ------------------------------------------------------------
 * 违反约束「函数指示符不是 lvalue，不能赋值」（6.3.2.1 [4]）
 * 期望：gcc -std=c99 报错 "lvalue required as left operand of assignment"
 * ------------------------------------------------------------ */
void neg_func_designator_assign(void)
{
    add = 0;                /* 错误：函数指示符不是 lvalue */
}

/* ------------------------------------------------------------
 * 违反约束「sizeof 的操作数不能是函数指示符」（6.5.3.4，Footnote 54）
 * 期望：gcc -std=c99 报错 "invalid application of 'sizeof' to a function type"
 * ------------------------------------------------------------ */
void neg_sizeof_function(void)
{
    size_t s = sizeof(add); /* 错误：sizeof 不能作用于函数类型 */
    (void)s;
}

/* ------------------------------------------------------------
 * 违反约束「sizeof 的操作数不能是不完整类型」（6.5.3.4）
 * 期望：gcc -std=c99 报错 "invalid application of 'sizeof' to incomplete type"
 * ------------------------------------------------------------ */
struct Incomplete;
void neg_sizeof_incomplete(void)
{
    size_t s = sizeof(struct Incomplete);  /* 错误：不完整类型 */
    (void)s;
}

/* ------------------------------------------------------------
 * 违反约束「const 限定对象不能作为 ++/-- 操作数」（6.5.2.4 / 6.5.3.1）
 * 期望：gcc -std=c99 报错 "increment of read-only variable"
 * ------------------------------------------------------------ */
void neg_const_incdec(void)
{
    const int ci = 5;
    ci++;                   /* 错误：ci 不是 modifiable lvalue */
    ++ci;
}

/* ------------------------------------------------------------
 * 违反约束「数组不能作为 ++/-- 操作数」（6.5.2.4 / 6.5.3.1）
 * 期望：gcc -std=c99 报错 "wrong type argument to increment"
 * ------------------------------------------------------------ */
void neg_array_incdec(void)
{
    int arr[3] = { 1, 2, 3 };
    arr++;                  /* 错误：数组不是 modifiable lvalue */
}

#endif /* 负向测试结束 */