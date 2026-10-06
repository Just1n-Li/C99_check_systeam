/*
 * 测试条款：C99 7.23.3.2  The ctime function
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（统一放在 #if 0 中，不影响本文件编译）。
 *
 * 覆盖段落：
 *   [1] 原型：char *ctime(const time_t *timer);
 *   [2] 语义：把 timer 指向的日历时间转换为本地时间字符串，
 *       等价于 asctime(localtime(timer))。
 *   [3] 返回值：返回 asctime 以该分解时间为参数所返回的指针。
 *   Forward references: localtime (7.23.3.4)
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 原型检查：ctime 接受 const time_t *，返回 char *。
 *     通过函数指针赋值来静态验证原型签名。 */
static char *(*ctime_proto)(const time_t *) = ctime;

int main(void)
{
    /* [1] 原型：参数为 const time_t *，返回 char * */
    time_t t;
    char *result;

    /* 构造一个确定的日历时间：1970-01-01 00:00:00 UTC 之后 0 秒。
     * 注意：ctime 使用本地时间，具体字符串依赖时区，因此我们只做
     * 结构性验证（非空、以 '\n' 结尾、长度合理），而不硬编码具体文本。 */
    t = (time_t)0;

    /* [2] 语义：ctime 等价于 asctime(localtime(timer))。
     *     分别调用两者，比较结果字符串是否一致。 */
    result = ctime(&t);
    assert(result != NULL);                 /* [3] 返回非空指针 */

    {
        struct tm *lt = localtime(&t);      /* Forward reference: localtime */
        char *expected;
        assert(lt != NULL);
        expected = asctime(lt);
        assert(expected != NULL);

        /* [2][3] ctime 的返回值应与 asctime(localtime(&t)) 的返回值内容一致 */
        assert(strcmp(result, expected) == 0);
    }

    /* [2] 结果是一个字符串：以 '\n' 结尾（asctime 的格式），且长度合理 */
    {
        size_t len = strlen(result);
        assert(len > 0);
        assert(result[len - 1] == '\n');
        /* asctime 格式固定为 26 字符（含结尾 '\n' 和 '\0' 前的内容） */
        assert(len == 25);
    }

    /* [3] 返回值就是 asctime 返回的指针（同一静态缓冲区）。
     *     再次调用 ctime 会覆盖同一缓冲区，因此两次调用返回同一地址。 */
    {
        char *r1 = ctime(&t);
        char *r2 = ctime(&t);
        assert(r1 == r2);                   /* 指向同一静态存储区 */
    }

    /* [1] 参数为 const time_t *：可以传入指向 const 的指针 */
    {
        const time_t ct = (time_t)1000000000;
        char *r = ctime(&ct);
        assert(r != NULL);
    }

    /* [1] 通过函数指针调用，验证原型可用 */
    {
        char *r = ctime_proto(&t);
        assert(r != NULL);
    }

    printf("All positive tests for C99 7.23.3.2 (ctime) passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* 违反约束「ctime 的参数类型为 const time_t *」：
 * 传入 time_t 值（而非指针），gcc -std=c99 应报错
 * （incompatible type for argument / passing argument makes pointer from integer）。 */
{
    time_t t = 0;
    char *p = ctime(t);          /* 错误：应为 const time_t *，此处传 time_t */
    (void)p;
}

/* 违反约束「ctime 的参数类型为 const time_t *」：
 * 传入 int * 而非 time_t *，类型不兼容，应报错。 */
{
    int x = 0;
    char *p = ctime(&x);         /* 错误：int * 与 const time_t * 不兼容 */
    (void)p;
}

/* 违反约束「ctime 返回 char *」：
 * 把返回值赋给不兼容的指针类型（如 int *），应报错。 */
{
    time_t t = 0;
    int *p = ctime(&t);          /* 错误：char * 赋给 int * 不兼容 */
    (void)p;
}

/* 违反约束「ctime 需要恰好一个参数」：
 * 调用时参数个数不匹配，应报错。 */
{
    time_t t = 0;
    char *p = ctime();           /* 错误：参数太少 */
    (void)p;
    p = ctime(&t, &t);           /* 错误：参数太多 */
    (void)p;
}

/* 违反约束「ctime 返回 char *，不是左值」：
 * 对函数调用结果赋值，应报错（lvalue required as left operand of assignment）。 */
{
    time_t t = 0;
    ctime(&t) = "x";             /* 错误：函数调用结果不是左值 */
}

#endif /* 负向测试结束 */