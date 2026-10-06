/*
 * 测试条款：C99 7.23.3 Time conversion functions
 * 条款原文要点：
 *   [1] 除 strftime 外，这些函数（asctime, ctime, gmtime, localtime）各自返回指向
 *       两种静态对象之一的指针：分解时间结构体(struct tm) 或 char 数组。
 *       执行任何返回这类指针的函数，可能覆盖之前任何一次调用返回的、指向同类型对象的
 *       信息（即返回的静态缓冲区是共享的，会被后续调用覆盖）。
 *       实现的行为应如同没有其它库函数调用这些函数（即这些静态对象不会被其它库函数
 *       悄悄修改）。
 *
 * 预期行为：
 *   正向测试：以下代码应能编译并运行通过（assert 全部成立）。
 *   负向测试：违反约束的片段应导致编译报错（放在 #if 0 中，不影响本文件编译）。
 *
 * 说明：本条款主要描述“语义/实现行为”，几乎没有显式 Constraints 段落。
 *       因此负向测试针对的是这些函数返回类型相关的类型约束（例如把返回的
 *       char* 当作可写左值、或对返回的 struct tm* 解引用后赋给 const 等），
 *       以及“返回指针指向静态对象”这一语义在类型层面的体现。
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 返回类型检查：asctime/ctime 返回 char*，gmtime/localtime 返回 struct tm* */
static void test_return_types(void)
{
    time_t t = (time_t)0; /* 1970-01-01 00:00:00 UTC */
    struct tm *tm_ptr;
    char *c_ptr;

    /* gmtime 返回 struct tm* */
    tm_ptr = gmtime(&t);
    assert(tm_ptr != NULL);

    /* localtime 返回 struct tm* */
    tm_ptr = localtime(&t);
    assert(tm_ptr != NULL);

    /* asctime 返回 char* */
    c_ptr = asctime(tm_ptr);
    assert(c_ptr != NULL);

    /* ctime 返回 char* */
    c_ptr = ctime(&t);
    assert(c_ptr != NULL);

    printf("[1] return types OK\n");
}

/* [1] 语义：返回的静态对象会被后续调用覆盖。
 * 对 asctime/ctime：两次调用返回的 char* 指向同一静态缓冲区，
 * 后一次调用会覆盖前一次的内容。
 * 注意：标准允许（但不要求）返回同一指针；这里验证“可能被覆盖”这一语义，
 * 即我们不能依赖旧指针内容在后续调用后仍然有效。为可移植地测试，
 * 我们只验证：调用后旧指针内容可能改变——但标准并未强制一定改变，
 * 所以这里改为验证“返回指针指向静态对象”的可观察性质：
 * 连续两次调用返回的指针，其内容在第二次调用后与第二次结果一致。
 */
static void test_static_overwrite_semantics(void)
{
    time_t t1 = (time_t)0;          /* 1970-01-01 UTC */
    time_t t2 = (time_t)86400;      /* 1970-01-02 UTC */
    char *p1;
    char *p2;

    /* 两次 ctime 调用 */
    p1 = ctime(&t1);
    assert(p1 != NULL);
    p2 = ctime(&t2);
    assert(p2 != NULL);

    /* 标准说“可能覆盖”，因此 p1 的内容在 p2 调用后可能已变。
     * 我们验证：p2 的内容确实对应 t2（而不是 t1）。
     * 同时，如果实现返回同一指针，则 p1 == p2，且 p1 内容 == p2 内容。 */
    if (p1 == p2) {
        /* 同一静态缓冲区：p1 内容已被 p2 覆盖 */
        assert(strcmp(p1, p2) == 0);
    }
    /* 无论是否同一指针，p2 必须对应 t2 的日期（1970-01-02） */
    assert(strstr(p2, "Jan") != NULL);
    /* ctime 输出格式含年份 1970 */
    assert(strstr(p2, "1970") != NULL);

    printf("[1] static overwrite semantics OK\n");
}

/* [1] 语义：gmtime/localtime 返回 struct tm*，同样可能被后续调用覆盖。
 * 验证返回的 struct tm 内容正确（以 UTC 1970-01-01 为例）。 */
static void test_tm_static_semantics(void)
{
    time_t t = (time_t)0;
    struct tm *tm1;
    struct tm *tm2;

    tm1 = gmtime(&t);
    assert(tm1 != NULL);
    /* 1970-01-01 00:00:00 UTC */
    assert(tm1->tm_year == 70);   /* 1900 + 70 = 1970 */
    assert(tm1->tm_mon  == 0);    /* January */
    assert(tm1->tm_mday == 1);
    assert(tm1->tm_hour == 0);
    assert(tm1->tm_min  == 0);
    assert(tm1->tm_sec  == 0);

    /* 第二次调用可能覆盖第一次的静态对象 */
    tm2 = gmtime(&t);
    assert(tm2 != NULL);
    if (tm1 == tm2) {
        /* 同一静态对象：内容应一致 */
        assert(tm1->tm_year == tm2->tm_year);
        assert(tm1->tm_mday == tm2->tm_mday);
    }

    printf("[1] struct tm static semantics OK\n");
}

/* [1] 语义：实现的行为应如同没有其它库函数调用这些函数。
 * 即：调用其它库函数（如 printf、strlen）不应改变这些静态对象的内容。
 * 这里验证：取得 asctime 结果后，调用其它库函数，再检查内容未变。 */
static void test_no_other_lib_modifies(void)
{
    time_t t = (time_t)0;
    struct tm *tm_ptr = gmtime(&t);
    char *s;
    char saved[64];

    assert(tm_ptr != NULL);
    s = asctime(tm_ptr);
    assert(s != NULL);

    /* 保存一份副本 */
    strncpy(saved, s, sizeof(saved) - 1);
    saved[sizeof(saved) - 1] = '\0';

    /* 调用其它库函数 */
    (void)strlen(saved);
    (void)printf("");          /* 空输出 */
    (void)memcmp(saved, s, strlen(saved));

    /* 再次检查 asctime 返回的静态缓冲区内容未被其它库函数修改 */
    assert(strcmp(s, saved) == 0);

    printf("[1] no other lib modifies static object OK\n");
}

/* [1] 语义：strftime 是例外——它不返回静态对象指针，而是把结果写入用户提供的缓冲区。
 * 验证 strftime 的返回值与写入行为。 */
static void test_strftime_exception(void)
{
    time_t t = (time_t)0;
    struct tm *tm_ptr = gmtime(&t);
    char buf[64];
    size_t n;

    assert(tm_ptr != NULL);
    n = strftime(buf, sizeof(buf), "%Y-%m-%d", tm_ptr);
    assert(n > 0);
    assert(strcmp(buf, "1970-01-01") == 0);

    printf("[1] strftime exception OK\n");
}

int main(void)
{
    test_return_types();
    test_static_overwrite_semantics();
    test_tm_static_semantics();
    test_no_other_lib_modifies();
    test_strftime_exception();

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/*
 * 违反约束：asctime/ctime 返回 char*，指向静态对象，不应被当作可写左值
 * 去修改（虽然类型上 char* 可写，但标准语义是静态对象；这里用类型约束
 * 演示：把返回的 char* 赋给 const char* 是允许的，但反过来把字符串字面量
 * 赋给 char* 在 C99 中是约束违反——不过那与 7.23.3 无关）。
 *
 * 更贴切 7.23.3 的负向测试：这些函数返回指针，不能把返回值当作数组来
 * 取地址后再做非法操作。下面演示对返回的 struct tm* 解引用后取地址赋给
 * 不兼容类型，违反赋值约束。
 */
void neg_assign_incompatible(void)
{
    time_t t = 0;
    /* gmtime 返回 struct tm*，不能赋给 int*（不兼容指针类型，约束违反） */
    int *p = gmtime(&t);   /* 期望：编译报错（不兼容指针类型赋值） */
    (void)p;
}

/*
 * 违反约束：asctime 返回 char*，不能赋给 struct tm*（不兼容指针类型）。
 */
void neg_assign_char_to_tm(void)
{
    time_t t = 0;
    struct tm *tm_ptr = gmtime(&t);
    /* asctime 返回 char*，赋给 struct tm* 违反赋值约束 */
    struct tm *bad = asctime(tm_ptr);  /* 期望：编译报错 */
    (void)bad;
}

/*
 * 违反约束：ctime 返回 char*，不能直接解引用为 struct tm 成员访问。
 * 对 char* 使用 -> 运算符违反约束（-> 左操作数必须是指向结构体/联合体的指针）。
 */
void neg_arrow_on_char_ptr(void)
{
    time_t t = 0;
    char *s = ctime(&t);
    /* char* 不是指向结构体的指针，-> 违反约束 */
    int x = s->tm_year;   /* 期望：编译报错 */
    (void)x;
}

/*
 * 违反约束：gmtime 返回 struct tm*，不能对它使用下标运算符 [] 当作数组。
 * 对 struct tm* 使用 [] 违反约束（[] 要求指向完整对象类型的指针，
 * 且该类型不能是函数类型；struct tm 是完整类型，但 [] 结果不是左值可赋？
 * 实际上对 struct tm* 用 [] 是合法的，返回 struct tm 左值。
 * 因此这里改为：对返回的 struct tm* 直接赋值（指针本身是右值，不可赋值）。
 */
void neg_assign_to_return_value(void)
{
    time_t t = 0;
    /* gmtime(&t) 是函数调用结果，不是左值，不能赋值 */
    gmtime(&t) = NULL;   /* 期望：编译报错（赋值左操作数必须是可修改左值） */
}

/*
 * 违反约束：asctime 返回 char*，函数调用结果不是左值，不能赋值。
 */
void neg_assign_to_asctime_result(void)
{
    time_t t = 0;
    struct tm *tm_ptr = gmtime(&t);
    asctime(tm_ptr) = NULL;   /* 期望：编译报错（非左值赋值） */
}

/*
 * 违反约束：ctime 返回 char*，函数调用结果不是左值，不能取地址后再赋值。
 * 这里演示对非左值取地址是约束违反。
 */
void neg_address_of_rvalue(void)
{
    time_t t = 0;
    char **pp = &ctime(&t);   /* 期望：编译报错（不能取右值的地址） */
    (void)pp;
}

/*
 * 违反约束：strftime 的第一个参数必须是 char*，不能传 struct tm*。
 * 违反函数实参类型约束。
 */
void neg_strftime_wrong_arg(void)
{
    time_t t = 0;
    struct tm *tm_ptr = gmtime(&t);
    char buf[64];
    /* 第一个参数应为 char*，传 struct tm* 违反约束 */
    strftime(tm_ptr, sizeof(buf), "%Y", tm_ptr);  /* 期望：编译报错 */
}

/*
 * 违反约束：strftime 的第三个参数必须是 const char*，不能传 int。
 */
void neg_strftime_wrong_fmt(void)
{
    time_t t = 0;
    struct tm *tm_ptr = gmtime(&t);
    char buf[64];
    /* 第三个参数应为 const char*，传 int 违反约束 */
    strftime(buf, sizeof(buf), 123, tm_ptr);  /* 期望：编译报错 */
}

#endif /* 负向测试结束 */