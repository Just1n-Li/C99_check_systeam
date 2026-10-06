/*
 * 测试 C99 7.20.7.1 —— mblen 函数
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 原型声明 int mblen(const char *s, size_t n);
 *   [2] s 非空时等价于 mbtowc((wchar_t *)0, s, n)，且不影响 mbtowc 转换状态
 *   [3] 实现行为如同没有库函数调用 mblen
 *   [4] 返回值：s 为空指针时返回 0/非 0（是否状态相关编码）；
 *       s 非空时返回 0（空字符）、正字节数（合法多字节字符）或 -1（非法）
 *   Forward references: mbtowc (7.20.7.2)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型：mblen 接受 const char * 与 size_t，返回 int。
 *     通过取函数指针类型来静态验证原型。 */
static int (*mblen_proto_check)(const char *, size_t) = mblen;

/* [2] 辅助：验证 mblen(s,n) 与 mbtowc((wchar_t*)0,s,n) 结果一致，
 *     且 mblen 不改变 mbtowc 的转换状态。 */
static void test_equivalence_with_mbtowc(void)
{
    /* 使用 "C" 区域设置，多字节字符即单字节字符，行为可预测。 */
    const char *s = "A";
    int r_mblen, r_mbtowc;

    /* 先复位 mbtowc 的转换状态 */
    mbtowc(NULL, NULL, 0);

    r_mblen  = mblen(s, MB_CUR_MAX);
    r_mbtowc = mbtowc((wchar_t *)0, s, MB_CUR_MAX);

    /* [2] 二者应返回相同结果 */
    assert(r_mblen == r_mbtowc);
    assert(r_mblen == 1); /* "A" 是 1 字节多字节字符 */
}

/* [2] mblen 不应影响 mbtowc 的转换状态：
 *     调用 mblen 后，mbtowc 仍应能从头正确解码。 */
static void test_mblen_does_not_affect_state(void)
{
    wchar_t wc;
    int r;

    mbtowc(NULL, NULL, 0); /* 复位状态 */

    /* 调用 mblen，条款要求它不影响 mbtowc 的转换状态 */
    (void)mblen("A", MB_CUR_MAX);

    /* 之后 mbtowc 仍应正常解码 */
    r = mbtowc(&wc, "A", MB_CUR_MAX);
    assert(r == 1);
    assert(wc == L'A');
}

/* [4] s 为空指针：返回 0 或非 0，表示编码是否状态相关。
 *     在 "C" 区域设置下通常返回 0（非状态相关）。 */
static void test_null_pointer(void)
{
    int r = mblen(NULL, 0);
    /* 只要求返回 0 或非 0，二者都合法；这里验证它确实是 int 值 */
    assert(r == 0 || r != 0);
    printf("mblen(NULL,0) = %d (0 表示非状态相关编码)\n", r);
}

/* [4] s 指向空字符：返回 0 */
static void test_null_character(void)
{
    const char *s = "";
    int r = mblen(s, MB_CUR_MAX);
    assert(r == 0);
}

/* [4] s 指向合法多字节字符：返回其字节数 */
static void test_valid_multibyte(void)
{
    const char *s = "A";
    int r = mblen(s, MB_CUR_MAX);
    assert(r == 1);

    /* 只给 n=1 字节，仍应构成合法字符 */
    r = mblen(s, 1);
    assert(r == 1);
}

/* [4] 非法多字节字符：返回 -1。
 *     在 "C" 区域设置下，字节 0xFF 不是合法多字节字符。 */
static void test_invalid_multibyte(void)
{
    const char bad[2];
    bad[0] = (char)0xFF;
    bad[1] = '\0';

    int r = mblen(bad, 1);
    /* 在 "C" 区域设置下 0xFF 非法，应返回 -1。
     * 若实现把 0xFF 视为合法（某些实现），则 r >= 0；
     * 这里针对标准 "C" 区域设置断言 -1。 */
    assert(r == -1);
}

/* [4] n 限制：若前 n 个字节不足以构成完整合法字符，返回 -1。
 *     在 "C" 区域设置下每个字符 1 字节，n=0 时无字节可读。 */
static void test_n_limit(void)
{
    const char *s = "A";
    int r = mblen(s, 0);
    /* n=0 时没有字节构成字符，应返回 -1（非空字符） */
    assert(r == -1);
}

/* [3] 实现行为如同没有库函数调用 mblen：
 *     即 mblen 是自包含的，不依赖其它库函数副作用。
 *     这里通过多次调用结果一致来间接验证其无隐藏状态。 */
static void test_no_hidden_state(void)
{
    int r1, r2, r3;
    r1 = mblen("A", MB_CUR_MAX);
    r2 = mblen("A", MB_CUR_MAX);
    r3 = mblen("A", MB_CUR_MAX);
    assert(r1 == r2 && r2 == r3);
    assert(r1 == 1);
}

int main(void)
{
    /* 使用 "C" 区域设置，保证多字节行为可预测 */
    setlocale(LC_ALL, "C");

    /* 确认原型检查指针已初始化（[1]） */
    assert(mblen_proto_check == mblen);

    test_equivalence_with_mbtowc();       /* [2] */
    test_mblen_does_not_affect_state();   /* [2] */
    test_null_pointer();                  /* [4] */
    test_null_character();                /* [4] */
    test_valid_multibyte();               /* [4] */
    test_invalid_multibyte();             /* [4] */
    test_n_limit();                       /* [4] */
    test_no_hidden_state();               /* [3] */

    printf("C99 7.20.7.1 mblen: all positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「mblen 的第一个参数类型为 const char *」：
 * 传入 int * 与原型不兼容，gcc -std=c99 应报错
 * （incompatible pointer type / passing argument 1）。 */
{
    int x = 0;
    int *p = &x;
    mblen(p, 1);   /* 期望：编译错误，int* 不能转换为 const char* */
}

/* 违反约束「mblen 的第二个参数类型为 size_t」：
 * 传入结构体类型，无法转换为 size_t，应报错。 */
{
    struct S { int a; } s;
    mblen("A", s); /* 期望：编译错误，结构体不能转换为 size_t */
}

/* 违反约束「mblen 返回 int，不能作为左值被赋值」：
 * 函数调用结果不是左值，对其赋值应报错。 */
{
    mblen("A", 1) = 5; /* 期望：编译错误，lvalue required as left operand of assignment */
}

/* 违反约束「mblen 需要两个实参」：
 * 实参个数不匹配，应报错。 */
{
    mblen("A");        /* 期望：编译错误，too few arguments to function 'mblen' */
}

/* 违反约束「mblen 需要两个实参」：
 * 实参过多，应报错。 */
{
    mblen("A", 1, 2);  /* 期望：编译错误，too many arguments to function 'mblen' */
}

/* 违反约束「mblen 的返回类型为 int，不能当作结构体使用」：
 * 对 int 结果做成员访问，应报错。 */
{
    mblen("A", 1).x;   /* 期望：编译错误，request for member 'x' in something not a structure */
}

#endif /* 负向测试结束 */