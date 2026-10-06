/*
 * 测试条款：C99 7.23.3.1  The asctime function
 *
 * 预期行为：
 *   正向测试：包含 <time.h>，调用 asctime()，验证其返回的字符串格式为
 *             "Sun Sep 16 01:03:52 1973\n\0" 形式（即 "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n"），
 *             返回非空指针，字符串长度 25 个可见字符 + '\n' + '\0' = 26 字节。
 *   负向测试：违反约束的代码（如参数类型错误、未包含头文件、对返回的 const 数据赋值等）
 *             应导致编译报错。
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 函数原型：char *asctime(const struct tm *timeptr);
 *     验证头文件 <time.h> 中声明了该函数，且参数为 const struct tm *，
 *     返回类型为 char *。通过取函数指针类型来静态验证。 */
static char *(*asctime_proto_check)(const struct tm *) = asctime;

/* [2] 描述：将 timeptr 指向的分解时间转换为如下形式的字符串：
 *     "Sun Sep 16 01:03:52 1973\n\0"
 *     使用与给定算法等价的算法。
 *     这里构造与标准示例完全一致的时间结构，验证输出字符串。 */
static void test_asctime_format(void)
{
    struct tm t;
    char *s;

    memset(&t, 0, sizeof t);
    t.tm_wday  = 0;    /* Sun */
    t.tm_mon   = 8;    /* Sep */
    t.tm_mday  = 16;
    t.tm_hour  = 1;
    t.tm_min   = 3;
    t.tm_sec   = 52;
    t.tm_year  = 73;   /* 1900 + 73 = 1973 */

    s = asctime(&t);

    /* [3] 返回指向字符串的指针，非空 */
    assert(s != NULL);

    /* 验证格式与内容：与标准给出的形式完全一致 */
    assert(strcmp(s, "Sun Sep 16 01:03:52 1973\n") == 0);

    /* 验证字符串长度：25 个可见字符 + '\n' = 26 个字符，再加 '\0' 共 27 字节 */
    assert(strlen(s) == 26);
    assert(s[25] == '\n');
    assert(s[26] == '\0');
}

/* [2] 验证算法中 %.3s 对星期名和月份名的截断行为，
 *     以及 %3d 对 tm_mday 的宽度填充行为。 */
static void test_asctime_padding_and_names(void)
{
    struct tm t;
    char *s;

    memset(&t, 0, sizeof t);
    t.tm_wday  = 3;    /* Wed */
    t.tm_mon   = 0;    /* Jan */
    t.tm_mday  = 5;    /* 应被 %3d 填充为 "  5" */
    t.tm_hour  = 0;
    t.tm_min   = 0;
    t.tm_sec   = 0;
    t.tm_year  = 100;  /* 1900 + 100 = 2000 */

    s = asctime(&t);
    assert(s != NULL);
    assert(strcmp(s, "Wed Jan  5 00:00:00 2000\n") == 0);
}

/* [2] 验证所有星期名与月份名的映射（wday_name[7][3], mon_name[12][3]）。 */
static void test_asctime_all_names(void)
{
    static const char *wday[] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    static const char *mon[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    int i, j;

    for (i = 0; i < 7; i++) {
        for (j = 0; j < 12; j++) {
            struct tm t;
            char *s;
            char expect[64];

            memset(&t, 0, sizeof t);
            t.tm_wday = i;
            t.tm_mon  = j;
            t.tm_mday = 1;
            t.tm_hour = 2;
            t.tm_min  = 3;
            t.tm_sec  = 4;
            t.tm_year = 0;   /* 1900 */

            s = asctime(&t);
            assert(s != NULL);

            sprintf(expect, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
                    wday[i], mon[j], t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec,
                    1900 + t.tm_year);
            assert(strcmp(s, expect) == 0);
        }
    }
}

/* [2] 验证 tm_year 与 1900 相加的语义（例如 tm_year = 0 表示 1900）。 */
static void test_asctime_year_offset(void)
{
    struct tm t;
    char *s;

    memset(&t, 0, sizeof t);
    t.tm_wday = 0;
    t.tm_mon  = 0;
    t.tm_mday = 1;
    t.tm_hour = 0;
    t.tm_min  = 0;
    t.tm_sec  = 0;
    t.tm_year = 0;    /* 1900 */

    s = asctime(&t);
    assert(s != NULL);
    assert(strcmp(s, "Sun Jan  1 00:00:00 1900\n") == 0);
}

/* [3] 验证返回的指针指向一个以 '\0' 结尾的字符串，且可被读取。 */
static void test_asctime_return_string(void)
{
    struct tm t;
    char *s;
    size_t n;

    memset(&t, 0, sizeof t);
    t.tm_wday = 6;    /* Sat */
    t.tm_mon  = 11;   /* Dec */
    t.tm_mday = 31;
    t.tm_hour = 23;
    t.tm_min  = 59;
    t.tm_sec  = 59;
    t.tm_year = 99;   /* 1999 */

    s = asctime(&t);
    assert(s != NULL);

    /* 逐字符读取直到 '\0'，确认字符串合法 */
    n = 0;
    while (s[n] != '\0') {
        n++;
    }
    assert(n == 26);
    assert(strcmp(s, "Sat Dec 31 23:59:59 1999\n") == 0);
}

int main(void)
{
    /* 静态验证函数原型 [1] */
    (void)asctime_proto_check;

    test_asctime_format();          /* [2] 标准示例 */
    test_asctime_padding_and_names(); /* [2] 填充与名称 */
    test_asctime_all_names();       /* [2] 全部星期/月份名 */
    test_asctime_year_offset();     /* [2] 年份偏移 */
    test_asctime_return_string();   /* [3] 返回字符串 */

    printf("All positive tests for C99 7.23.3.1 asctime passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「asctime 的参数类型为 const struct tm *」：
 * 传入 int 类型实参，gcc -std=c99 应报错（参数类型不兼容）。 */
void neg_wrong_arg_type(void)
{
    int x = 0;
    char *s = asctime(x);   /* 错误：int 不能转换为 const struct tm * */
    (void)s;
}

/* 违反约束「asctime 的参数类型为 const struct tm *」：
 * 传入 char * 类型实参，gcc -std=c99 应报错。 */
void neg_wrong_arg_type2(void)
{
    char buf[10];
    char *s = asctime(buf); /* 错误：char * 不能转换为 const struct tm * */
    (void)s;
}

/* 违反约束「asctime 的返回类型为 char *」：
 * 将返回值赋给不兼容的指针类型（如 int *），gcc -std=c99 应报错。 */
void neg_wrong_return_type(void)
{
    struct tm t;
    int *p = asctime(&t);   /* 错误：char * 不能转换为 int * */
    (void)p;
}

/* 违反约束「调用函数前必须有声明」：
 * 未包含 <time.h> 且未声明 asctime，直接调用，
 * 在 C99 中隐式函数声明已被移除，gcc -std=c99 应报错。 */
void neg_no_declaration(void)
{
    struct tm t;
    char *s = asctime(&t);  /* 错误：asctime 未声明 */
    (void)s;
}

/* 违反约束「asctime 的参数为指向 const struct tm 的指针」：
 * 传入指向不完整/不兼容结构体类型的指针，gcc -std=c99 应报错。 */
struct not_tm { int a; };
void neg_incompatible_struct_ptr(void)
{
    struct not_tm nt;
    char *s = asctime(&nt); /* 错误：struct not_tm * 与 const struct tm * 不兼容 */
    (void)s;
}

/* 违反约束「asctime 返回 char *，其指向的字符串不应被修改」：
 * 虽然标准未明确禁止修改返回的静态缓冲区，但若将返回值赋给
 * const char * 后再尝试写入，属于对只读对象的修改，应报错。
 * 这里演示对字符串字面量/只读对象的写入。 */
void neg_write_to_const(void)
{
    const char *s = "abc";
    s[0] = 'x';   /* 错误：对 const 限定对象赋值 */
}

#endif /* 负向测试结束 */