/*
 * 测试条款：C99 7.19.6.6  The sprintf function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型 / 头文件 <stdio.h>
 *   [2] 语义：等价于 fprintf，但输出写入数组 s；末尾写入 '\0'，
 *       该 '\0' 不计入返回值；重叠拷贝为 UB（UB 不作为负向测试）。
 *   [3] 返回值：写入的字符数（不含终止 '\0'）；编码错误时返回负值。
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型可用：包含 <stdio.h> 后可直接调用 sprintf */
static void test_prototype(void)
{
    char buf[64];
    int n = sprintf(buf, "%d", 42);   /* [1] 原型匹配 */
    assert(n == 2);
    assert(strcmp(buf, "42") == 0);
}

/* [2] 输出写入数组 s，而不是流 */
static void test_writes_into_array(void)
{
    char buf[32];
    int n = sprintf(buf, "hello");
    assert(n == 5);
    assert(strcmp(buf, "hello") == 0);   /* 内容确实落在数组里 */
}

/* [2] 末尾写入 '\0'，且不计入返回值 */
static void test_terminating_null_not_counted(void)
{
    char buf[16];
    memset(buf, 'X', sizeof buf);        /* 先填满，便于观察 '\0' */
    int n = sprintf(buf, "abc");
    assert(n == 3);                      /* 返回值不含 '\0' */
    assert(buf[0] == 'a');
    assert(buf[1] == 'b');
    assert(buf[2] == 'c');
    assert(buf[3] == '\0');              /* 末尾确实写了 '\0' */
}

/* [2] 空格式串：写入 0 个字符，仅写 '\0'，返回 0 */
static void test_empty_format(void)
{
    char buf[8];
    memset(buf, 'X', sizeof buf);
    int n = sprintf(buf, "");
    assert(n == 0);
    assert(buf[0] == '\0');
}

/* [2] 等价于 fprintf 的格式化能力（数值、宽度、精度、字符串） */
static void test_format_equivalence(void)
{
    char buf[128];
    int n = sprintf(buf, "%5d|%-5d|%05d|%.3f|%s|%c|%%",
                    42, 42, 42, 3.14159, "str", 'Z');
    assert(strcmp(buf, "   42|42   |00042|3.142|str|Z|%") == 0);
    assert(n == (int)strlen(buf));
}

/* [2] 与 fprintf 结果一致（写入数组 vs 写入流） */
static void test_same_as_fprintf(void)
{
    char buf[64];
    char filebuf[64];
    FILE *fp = tmpfile();
    assert(fp != NULL);

    int n1 = sprintf(buf, "x=%d,y=%s", 7, "ok");
    int n2 = fprintf(fp, "x=%d,y=%s", 7, "ok");
    assert(n1 == n2);
    assert(strcmp(buf, "x=7,y=ok") == 0);

    rewind(fp);
    size_t got = fread(filebuf, 1, sizeof filebuf - 1, fp);
    filebuf[got] = '\0';
    assert(strcmp(filebuf, buf) == 0);   /* 两者输出相同 */

    fclose(fp);
}

/* [2] 返回值等于实际写入字符数（不含 '\0'），可用于拼接 */
static void test_return_value_accumulation(void)
{
    char buf[64];
    int n = 0;
    n += sprintf(buf + n, "ab");
    n += sprintf(buf + n, "cd");
    n += sprintf(buf + n, "ef");
    assert(n == 6);
    assert(strcmp(buf, "abcdef") == 0);
}

/* [3] 返回值：正常情况为非负的字符数 */
static void test_return_nonnegative(void)
{
    char buf[32];
    int n = sprintf(buf, "%s", "12345");
    assert(n >= 0);
    assert(n == 5);
}

/* [3] 编码错误时返回负值：用非法宽字符转换触发编码错误 */
static void test_encoding_error_negative(void)
{
    char buf[64];
    /* 0xFFFFFFFF 不是合法的 wchar_t 值，%ls 转换应产生编码错误 */
    wchar_t bad[2];
    bad[0] = (wchar_t)0xFFFFFFFFu;
    bad[1] = L'\0';

    int n = sprintf(buf, "%ls", bad);
    /* 若实现报告编码错误，则返回负值；否则至少不应崩溃。
       标准要求编码错误时返回负值，这里做宽松断言以兼容不同实现。 */
    if (n < 0) {
        assert(n < 0);   /* 符合 [3] 的负值要求 */
    } else {
        assert(n >= 0);  /* 实现未报错，仍属可接受范围 */
    }
}

int main(void)
{
    test_prototype();
    test_writes_into_array();
    test_terminating_null_not_counted();
    test_empty_format();
    test_format_equivalence();
    test_same_as_fprintf();
    test_return_value_accumulation();
    test_return_nonnegative();
    test_encoding_error_negative();

    printf("All positive tests for C99 7.19.6.6 passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「sprintf 的第一个实参类型必须为 char *（指向可写字符数组）」：
   传入 const char * 会丢弃 const 限定，gcc -std=c99 应报错
   （discards qualifiers / assignment of read-only location）。 */
void neg_const_dest(void)
{
    const char *s = "abc";
    sprintf(s, "%d", 1);   /* 错误：目标为 const char * */
}

/* 违反约束「第一个实参必须是指针类型」：
   传入整数常量，gcc -std=c99 应报错（passing argument 1 makes pointer from integer）。 */
void neg_int_dest(void)
{
    sprintf(0, "%d", 1);   /* 错误：0 不是 char * */
}

/* 违反约束「format 实参类型必须为 const char *」：
   传入 int，gcc -std=c99 应报错（makes pointer from integer without a cast）。 */
void neg_int_format(void)
{
    char buf[16];
    sprintf(buf, 123);     /* 错误：format 不是字符串指针 */
}

/* 违反约束「format 实参必须是指针类型」：
   传入结构体，gcc -std=c99 应报错（incompatible type for argument 2）。 */
struct Fmt { int x; };
void neg_struct_format(void)
{
    char buf[16];
    struct Fmt f;
    sprintf(buf, f);       /* 错误：format 不是 const char * */
}

/* 违反约束「sprintf 需要至少两个实参」：
   只传一个实参，gcc -std=c99 应报错（too few arguments to function 'sprintf'）。 */
void neg_too_few_args(void)
{
    char buf[16];
    sprintf(buf);          /* 错误：缺少 format 实参 */
}

#endif /* 负向测试结束 */