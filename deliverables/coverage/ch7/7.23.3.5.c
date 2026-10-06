/*
 * 测试目标：C99 7.23.3.5  The strftime function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：以下代码违反 C99 约束，应编译报错（统一放在 #if 0 中，
 *             因此本文件整体仍可正常编译运行）。
 *
 * 覆盖段落：
 *   [1] 函数原型（restrict 限定、返回 size_t、参数类型）
 *   [2] 描述：普通字符原样拷贝、% 转换说明、maxsize 限制、终止空字符拷贝
 *   [3] 各转换说明符（%a %A %b %B %c %C %d %D %e %F %g %G %h %H %I %j
 *       %m %M %n %p %r %R %S %t %T %u %U %V %w %W %x %X %y %Y %z %Z %%）
 *   [4] E / O 修饰符（若本地化不支持则忽略修饰符）
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 函数原型检查：把 strftime 赋给一个函数指针，验证签名兼容。
 *     参数为 (char * restrict, size_t, const char * restrict,
 *              const struct tm * restrict)，返回 size_t。 */
static size_t (*fp_strftime)(char * restrict, size_t,
                             const char * restrict,
                             const struct tm * restrict) = strftime;

/* 构造一个确定的 broken-down time：
 *   1997-01-01 是星期三（tm_wday = 3），tm_yday = 0。
 *   使用固定值以便断言可预测的数值型转换说明符。 */
static struct tm make_tm(void)
{
    struct tm t;
    memset(&t, 0, sizeof t);
    t.tm_year = 97;   /* 1997 年 */
    t.tm_mon  = 0;    /* 一月 */
    t.tm_mday = 1;    /* 1 日 */
    t.tm_hour = 13;   /* 13 时 */
    t.tm_min  = 5;    /* 5 分 */
    t.tm_sec  = 9;    /* 9 秒 */
    t.tm_wday = 3;    /* 星期三 */
    t.tm_yday = 0;    /* 一年中的第 0 天 */
    t.tm_isdst = 0;
    return t;
}

/* 辅助：调用 strftime 并返回结果字符串（静态缓冲） */
static const char *fmt(const char *format, const struct tm *tp)
{
    static char buf[256];
    size_t n = strftime(buf, sizeof buf, format, tp);
    assert(n < sizeof buf);
    return buf;
}

int main(void)
{
    struct tm t = make_tm();

    /* [1] 通过函数指针调用，验证原型可用 */
    {
        char buf[64];
        size_t n = fp_strftime(buf, sizeof buf, "%Y", &t);
        assert(n == 4);
        assert(strcmp(buf, "1997") == 0);
    }

    /* [2] 普通多字节字符（含终止空字符）原样拷贝 */
    {
        char buf[64];
        size_t n = strftime(buf, sizeof buf, "hello world", &t);
        assert(n == 11);
        assert(strcmp(buf, "hello world") == 0);
        assert(buf[11] == '\0');   /* 终止空字符被拷贝 */
    }

    /* [2] 空格式串：只写入终止空字符，返回 0 */
    {
        char buf[8];
        memset(buf, 'X', sizeof buf);
        size_t n = strftime(buf, sizeof buf, "", &t);
        assert(n == 0);
        assert(buf[0] == '\0');
    }

    /* [2] maxsize 限制：结果放不下时返回 0，且不写超过 maxsize 个字符。
     *     这里格式串 "abcdef" 需要 7 字节（含 '\0'），maxsize = 4 放不下。 */
    {
        char buf[8];
        memset(buf, 'X', sizeof buf);
        size_t n = strftime(buf, 4, "abcdef", &t);
        assert(n == 0);
        /* 未定义具体写入内容，但不应越界；此处仅检查未越界（buf[4..7] 仍为 'X'） */
        assert(buf[4] == 'X' && buf[5] == 'X' && buf[6] == 'X' && buf[7] == 'X');
    }

    /* [2] maxsize 恰好容纳（含终止空字符）时成功 */
    {
        char buf[7];
        size_t n = strftime(buf, 7, "abcdef", &t);
        assert(n == 6);
        assert(strcmp(buf, "abcdef") == 0);
    }

    /* [3] %% 被替换为 % */
    assert(strcmp(fmt("%%", &t), "%") == 0);

    /* [3] %n 换行、%t 水平制表符 */
    assert(strcmp(fmt("%n", &t), "\n") == 0);
    assert(strcmp(fmt("%t", &t), "\t") == 0);

    /* [3] 数值型转换说明符（使用固定 tm 值，结果可预测） */
    assert(strcmp(fmt("%Y", &t), "1997") == 0);   /* [tm_year] */
    assert(strcmp(fmt("%y", &t), "97")   == 0);   /* [tm_year] 后两位 */
    assert(strcmp(fmt("%C", &t), "19")   == 0);   /* [tm_year] 年/100 */
    assert(strcmp(fmt("%m", &t), "01")   == 0);   /* [tm_mon] 月份 01-12 */
    assert(strcmp(fmt("%d", &t), "01")   == 0);   /* [tm_mday] 01-31 */
    assert(strcmp(fmt("%e", &t), " 1")   == 0);   /* [tm_mday] 单位数前导空格 */
    assert(strcmp(fmt("%H", &t), "13")   == 0);   /* [tm_hour] 24 小时制 */
    assert(strcmp(fmt("%I", &t), "01")   == 0);   /* [tm_hour] 12 小时制 */
    assert(strcmp(fmt("%M", &t), "05")   == 0);   /* [tm_min] */
    assert(strcmp(fmt("%S", &t), "09")   == 0);   /* [tm_sec] */
    assert(strcmp(fmt("%j", &t), "001")  == 0);   /* [tm_yday] 001-366 */
    assert(strcmp(fmt("%w", &t), "3")    == 0);   /* [tm_wday] 星期日=0 */
    assert(strcmp(fmt("%u", &t), "3")    == 0);   /* [tm_wday] 星期一=1 */

    /* [3] 复合等价说明符 */
    assert(strcmp(fmt("%D", &t), "01/01/97") == 0);      /* %m/%d/%y */
    assert(strcmp(fmt("%F", &t), "1997-01-01") == 0);    /* %Y-%m-%d */
    assert(strcmp(fmt("%R", &t), "13:05") == 0);         /* %H:%M */
    assert(strcmp(fmt("%T", &t), "13:05:09") == 0);      /* %H:%M:%S */
    assert(strcmp(fmt("%h", &t), fmt("%b", &t)) == 0);   /* %h 等价 %b */

    /* [3] 周数说明符：只验证格式（两位十进制数），不依赖具体周数算法 */
    {
        const char *sU = fmt("%U", &t);
        const char *sW = fmt("%W", &t);
        const char *sV = fmt("%V", &t);
        const char *sg = fmt("%g", &t);
        const char *sG = fmt("%G", &t);
        assert(strlen(sU) == 2 && sU[0] >= '0' && sU[0] <= '9');
        assert(strlen(sW) == 2 && sW[0] >= '0' && sW[0] <= '9');
        assert(strlen(sV) == 2 && sV[0] >= '0' && sV[0] <= '9');
        assert(strlen(sg) == 2 && sg[0] >= '0' && sg[0] <= '9');
        assert(strlen(sG) == 4);   /* 例如 1997 */
    }

    /* [3] 本地化相关说明符：只验证调用成功（返回非零长度），
     *     具体文本依赖 LC_TIME，不做内容断言。 */
    {
        const char *specs[] = { "%a", "%A", "%b", "%B", "%c", "%p",
                                "%r", "%x", "%X", "%Z", "%z" };
        size_t i;
        for (i = 0; i < sizeof specs / sizeof specs[0]; ++i) {
            char buf[256];
            size_t n = strftime(buf, sizeof buf, specs[i], &t);
            /* %z / %Z 在无法确定时区时可能为空，其余应非空 */
            if (strcmp(specs[i], "%z") != 0 && strcmp(specs[i], "%Z") != 0) {
                assert(n > 0);
            }
        }
    }

    /* [4] E / O 修饰符：若本地化不支持替代格式，修饰符被忽略，
     *     即 %Ec 与 %c 结果相同、%Od 与 %d 结果相同等。
     *     这里只验证调用成功且不崩溃，并检查 %O 数值说明符仍产生数字。 */
    {
        char buf[256];
        size_t n;
        n = strftime(buf, sizeof buf, "%Ec", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%EC", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%Ex", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%EX", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%Ey", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%EY", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%Od", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%Oe", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%OH", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%OI", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%Om", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%OM", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%OS", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%Ou", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%OU", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%OV", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%Ow", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%OW", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%Oy", &t); assert(n > 0);
        n = strftime(buf, sizeof buf, "%OY", &t); assert(n > 0);
    }

    /* [3] 混合普通字符与转换说明符 */
    assert(strcmp(fmt("Year=%Y, Month=%m", &t), "Year=1997, Month=01") == 0);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「strftime 的第一个参数类型为 char * restrict」：
 * 传入 const char *（丢弃 const 限定），gcc -std=c99 应报错
 * （discards qualifiers / incompatible pointer type）。 */
{
    const char buf[64];
    struct tm t;
    strftime(buf, sizeof buf, "%Y", &t);   /* 错误：s 应为 char * */
}

/* 违反约束「strftime 的第三个参数类型为 const char * restrict」：
 * 传入 int，gcc -std=c99 应报错（incompatible type for argument）。 */
{
    char buf[64];
    struct tm t;
    strftime(buf, sizeof buf, 42, &t);     /* 错误：format 应为 const char * */
}

/* 违反约束「strftime 的第四个参数类型为 const struct tm * restrict」：
 * 传入 struct tm（非指针），gcc -std=c99 应报错。 */
{
    char buf[64];
    struct tm t;
    strftime(buf, sizeof buf, "%Y", t);    /* 错误：timeptr 应为指针 */
}

/* 违反约束「strftime 的第二个参数类型为 size_t」：
 * 传入 struct tm *，gcc -std=c99 应报错。 */
{
    char buf[64];
    struct tm t;
    strftime(buf, &t, "%Y", &t);           /* 错误：maxsize 应为 size_t */
}

/* 违反约束「strftime 返回 size_t」：
 * 把返回值赋给 struct tm，gcc -std=c99 应报错。 */
{
    char buf[64];
    struct tm t, r;
    r = strftime(buf, sizeof buf, "%Y", &t);  /* 错误：size_t 不能赋给 struct tm */
}

/* 违反约束「调用 strftime 需包含 <time.h> 声明」：
 * 若未包含 <time.h>，C99 下隐式函数声明为约束违反（C99 移除隐式声明），
 * gcc -std=c99 应报错（implicit declaration of function 'strftime'）。
 * 注意：本文件已包含 <time.h>，此处仅示意。 */
{
    char buf[64];
    struct tm t;
    strftime(buf, sizeof buf, "%Y", &t);   /* 若缺少 <time.h> 则报错 */
}

#endif /* 负向测试结束 */