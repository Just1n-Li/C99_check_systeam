/*
 * 验证 C99 条款 6.2.3 (Name spaces of identifiers)
 * 预期行为：
 * 正向测试：能编译并运行通过，验证不同名字空间的标识符可以同名且互不干扰。
 * 负向测试：违反同一名字空间内标识符重名约束，应编译报错。
 */

#include <stdio.h>
#include <assert.h>

/* ========== 正向测试：以下代码应能编译并运行通过 ========== */

/* [1] 普通标识符 与 结构体标签 同名 */
int var_a = 1;
struct var_a { int x; }; /* struct tag 'var_a' 与普通标识符 'var_a' 在不同名字空间 */

/* [1] 脚注24：struct, union, enum 共享同一个 tag 名字空间，但与普通标识符独立 */
struct tag_b { int x; };
int tag_b = 2; /* 普通标识符 'tag_b' 与 struct tag 'tag_b' 不冲突 */

/* [1] 每个结构体或联合体都有其成员的独立名字空间 */
struct struct_c { int member; };
struct struct_d { double member; }; /* 'member' 在不同结构体中属于不同名字空间 */

/* [1] 结构体成员 与 普通标识符 同名 */
int member = 3;
struct struct_e { int member; };

int main(void) {
    /* [1] 标签名 与 普通标识符 同名 */
    int label = 5;
    /* label 名字空间和普通标识符名字空间独立，goto label 不会混淆 */
    if (label > 0) {
        label--;
        goto label;
    }
label:
    assert(label == 0);

    /* [1] 验证普通标识符与结构体标签同名 */
    struct var_a a;
    a.x = 10;
    assert(var_a == 1);
    assert(a.x == 10);

    /* [1] 验证不同结构体的同名成员互不干扰 */
    struct struct_c c;
    struct struct_d d;
    c.member = 20;
    d.member = 30.0;
    assert(c.member == 20);
    assert(d.member == 30.0);

    /* [1] 验证结构体成员与普通标识符同名 */
    struct struct_e e;
    e.member = 40;
    assert(member == 3);
    assert(e.member == 40);

    printf("All positive tests passed.\n");
    return 0;
}

/* ========== 负向测试：以下代码违反 C99 约束，应编译报错 ========== */
#if 0
/* [1] 违反约束：枚举常量属于普通标识符名字空间，与普通变量重名应报错 */
int ordinary_id = 1;
enum { ordinary_id = 2 }; /* gcc -std=c99 应报错：'ordinary_id' 重新定义为不同类型的符号 */

/* [1] 脚注24 违反约束：struct, union, enum 共享同一个 tag 名字空间，重名应报错 */
struct shared_tag { int a; };
union shared_tag { int b; }; /* gcc -std=c99 应报错：'shared_tag' 重新定义为不同类型的符号 */

/* [1] 违反约束：同一结构体内的成员名字空间内重名应报错 */
struct S {
    int dup_member;
    double dup_member; /* gcc -std=c99 应报错：重复成员 'dup_member' */
};

/* [1] 违反约束：同一函数内的 label 名字空间内重名应报错 */
void func_with_dup_labels(void) {
    goto my_label;
my_label:
    goto my_label;
my_label:; /* gcc -std=c99 应报错：重复标签 'my_label' */
}
#endif