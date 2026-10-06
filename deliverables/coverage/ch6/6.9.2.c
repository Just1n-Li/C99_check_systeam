/*
 * 验证 C99 6.9.2 External object definitions
 * 正向测试：应能编译并运行通过，验证语义 [1][2][3] 及 EXAMPLE [4][5]
 * 负向测试：违反约束 [3]（内部链接的暂定定义不得有不完整类型），应编译报错
 */

#include <assert.h>
#include <stdio.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 文件作用域带初始化器的声明是外部定义 */
int def_i1 = 1;          /* 定义，外部链接 */
static int def_i2 = 2;   /* 定义，内部链接 */
extern int def_i3 = 3;   /* 定义，外部链接（extern + initializer = definition） */

/* [2] 暂定定义：文件作用域，无初始化器，无存储类说明符或 static */
int td_i4;               /* 暂定定义，外部链接 */
static int td_i5;        /* 暂定定义，内部链接 */

/* [2] 多个暂定定义且无外部定义 -> 行为如同翻译单元末尾初始化为 0 */
int td_multi;            /* 暂定定义 */
int td_multi;            /* 再次暂定定义，合法 */
int td_multi;            /* 第三次暂定定义，合法 */

/* [2] 暂定定义后跟外部定义，暂定定义被合并 */
int td_then_def;         /* 暂定定义 */
int td_then_def = 42;    /* 外部定义 */

/* [2] 复合类型：暂定定义可以逐步完善类型（外部链接允许不完整类型） */
int td_composite[];      /* 暂定定义，不完整类型 */
int td_composite[5];     /* 暂定定义，现在完整了 */

/* [3] 暂定定义具有内部链接时，类型必须是完整类型（正向：完整类型） */
static int td_internal_ok;  /* 完整类型，内部链接，符合约束 */

/* [4] EXAMPLE 1 中的有效情况 */
int def_i1;              /* [4] 有效暂定定义，引用之前的 def_i1 */
int def_i3;              /* [4] 有效暂定定义，引用之前的 def_i3 */
int td_i4;               /* [4] 有效暂定定义，引用之前的 td_i4 */

extern int def_i1;       /* [4] 引用之前的 def_i1，外部链接 */
extern int def_i3;       /* [4] 引用之前的 def_i3，外部链接 */
extern int td_i4;        /* [4] 引用之前的 td_i4，外部链接 */
extern int td_i5;        /* [4] 引用之前的 td_i5，内部链接 */

/* [5] EXAMPLE 2: int i[]; 在翻译单元末尾仍不完整，
   隐式初始化为一个元素，值为 0 */
int example2_arr[];

void test_positive(void)
{
    /* [1] 外部定义验证 */
    assert(def_i1 == 1);
    assert(def_i2 == 2);
    assert(def_i3 == 3);

    /* [2] 多个暂定定义无外部定义 -> 初始化为 0 */
    assert(td_multi == 0);

    /* [2] 暂定定义后跟外部定义 */
    assert(td_then_def == 42);

    /* [2] 暂定定义（无初始化器）初始化为 0 */
    assert(td_i4 == 0);
    assert(td_i5 == 0);

    /* [2] 复合类型验证 */
    assert(sizeof(td_composite) == sizeof(int) * 5);
    for (int i = 0; i < 5; i++) {
        assert(td_composite[i] == 0);
    }

    /* [3] 内部链接完整类型暂定定义，初始化为 0 */
    assert(td_internal_ok == 0);

    /* [5] EXAMPLE 2: 数组隐式初始化为一个元素，值为 0 */
    assert(sizeof(example2_arr) == sizeof(int));
    assert(example2_arr[0] == 0);

    printf("All positive tests passed.\n");
}

int main(void)
{
    test_positive();
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0

/* [3] 违反约束：暂定定义具有内部链接（static），但声明的类型是不完整类型（未指定大小的数组）
   gcc -std=c99 应报错，例如 "error: storage size of 'bad_arr' isn't known" */
static int bad_arr[];

/* [3] 违反约束：暂定定义具有内部链接（static），但声明的类型是不完整类型（不完整结构体）
   gcc -std=c99 应报错，例如 "error: storage size of 'bad_struct' isn't known" */
struct IncompleteType;
static struct IncompleteType bad_struct;

#endif