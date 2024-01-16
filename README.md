# Piped

Piped is a single header library for C++ that features piping operator syntax similar to that of Circle compiler, but in pure C++ 20 standard, without any compiler extensions.

## Installation
Simply include the piped.hpp header file in your C++ project to start using the library.

## Usage
```c++
#include "piped.hpp"
#include <iostream>

using piped::$;

int main() {
    auto result = $[5] || (!$ * 2) || (!$ + 3) || $; // Pipe into $ at the end to extract result
    $[5] || (!$ * 2) || (!$ + 3); // Ignore result
    std::cout << result << std::endl; // Output: 13
    return 0;
}
```

For more examples, see tests.

## Cache Coherence

This version of the library relies on GCC 13.2 optimizations to eliminate unnecessary access to global variables.
### Assembly comparison
Assembly produced by **GCC 13.2** with `-O3` via [Compiler Explorer](https://godbolt.org/)
<table>
<tr>
<th>Handwritten</th>
<th>Piped</th>
</tr>
<tr>
<td>

```c++
int main(int argc, char** argv) {
    int res = (argc + 1) * 2;
    return res;
}
```

</td>
<td>

```c++
int main(int argc, char** argv) {
    int res = $[argc] || !$ + 1 || !$ * 2 || $;
    return res;
}
```

</td>
</tr>
<tr>
<td>

```asm
main:
        lea     eax, [rdi+2+rdi]
        ret
```

</td>
<td>

```asm
main:
        lea     eax, [rdi+2+rdi]
        ret
```

</td>
</tr>

<th>Handwritten</th>
<th>Piped</th>

<tr>
<td>

```c++
struct NonTrivial {
    NonTrivial(int v) {
        ptr = new int[1 + v]();
        ptr[0] = v;
    }

    NonTrivial(const NonTrivial& other) {
        ptr = new int[1 + other.get_value()];
        ptr[0] = other.get_value();
    }

    NonTrivial(NonTrivial&& other) {
        ptr = other.ptr;
        other.ptr = nullptr;
    }

    ~NonTrivial() {
        delete[] ptr;
    }

    NonTrivial operator+(int x) const {
        return NonTrivial(x + get_value());
    }

    int get_value() const {
        return ptr[0];
    }

    int* ptr = nullptr;
};
int main(int argc, char** argv) {
    NonTrivial initial(argc);
    auto step1 = initial + 1;
    auto step2 = std::move(step1) + 100;
    NonTrivial result = std::move(step2);
    return result.value;
}
```

</td>
<td>

```c++
struct NonTrivial {
    NonTrivial(int v) {
        ptr = new int[1 + v]();
        ptr[0] = v;
    }

    NonTrivial(const NonTrivial& other) {
        ptr = new int[1 + other.get_value()];
        ptr[0] = other.get_value();
    }

    NonTrivial(NonTrivial&& other) {
        ptr = other.ptr;
        other.ptr = nullptr;
    }

    ~NonTrivial() {
        delete[] ptr;
    }

    NonTrivial operator+(int x) const {
        return NonTrivial(x + get_value());
    }

    int get_value() const {
        return ptr[0];
    }

    int* ptr = nullptr;
};
int main(int argc, char** argv) {
    NonTrivial initial(argc);
    NonTrivial result = $[initial] || !$ + 1 || !$ + 100 || $;
    return result.value;
}
```

</td>
</tr>
<tr>
<td>

```asm
NonTrivial::NonTrivial(int) [base object constructor]:
        movabs  rax, 2305843009213693950
        push    r13
        push    r12
        lea     r12d, [rsi+1]
        push    rbp
        movsx   r12, r12d
        mov     rbp, rdi
        push    rbx
        mov     ebx, esi
        sub     rsp, 8
        mov     QWORD PTR [rdi], 0
        cmp     rax, r12
        jnb     .L2
        mov     rdi, -1
        call    operator new[](unsigned long)
        mov     rcx, rax
.L3:
        mov     QWORD PTR [rbp+0], rcx
        mov     DWORD PTR [rcx], ebx
        add     rsp, 8
        pop     rbx
        pop     rbp
        pop     r12
        pop     r13
        ret
.L2:
        lea     r13, [0+r12*4]
        mov     rdi, r13
        call    operator new[](unsigned long)
        mov     rcx, rax
        test    r12, r12
        je      .L3
        mov     rdx, r13
        xor     esi, esi
        mov     rdi, rax
        call    memset
        mov     rcx, rax
        jmp     .L3
main:
        push    r12
        mov     esi, edi
        push    rbp
        push    rbx
        sub     rsp, 32
        lea     rdi, [rsp+8]
        call    NonTrivial::NonTrivial(int) [complete object constructor]
        mov     rbx, QWORD PTR [rsp+8]
        lea     rdi, [rsp+16]
        mov     eax, DWORD PTR [rbx]
        lea     esi, [rax+1]
        call    NonTrivial::NonTrivial(int) [complete object constructor]
        mov     rbp, QWORD PTR [rsp+16]
        lea     rdi, [rsp+24]
        mov     eax, DWORD PTR [rbp+0]
        lea     esi, [rax+100]
        call    NonTrivial::NonTrivial(int) [complete object constructor]
        mov     rdi, QWORD PTR [rsp+24]
        mov     r12d, DWORD PTR [rdi]
        call    operator delete[](void*)
        mov     rdi, rbp
        call    operator delete[](void*)
        mov     rdi, rbx
        call    operator delete[](void*)
        add     rsp, 32
        mov     eax, r12d
        pop     rbx
        pop     rbp
        pop     r12
        ret
```

</td>
<td>

```diff
NonTrivial::NonTrivial(int) [base object constructor]:
        movabs  rax, 2305843009213693950
        push    r13
        push    r12
        lea     r12d, [rsi+1]
        push    rbp
        movsx   r12, r12d
        mov     rbp, rdi
        push    rbx
        mov     ebx, esi
        sub     rsp, 8
        mov     QWORD PTR [rdi], 0
        cmp     rax, r12
        jnb     .L2
        mov     rdi, -1
        call    operator new[](unsigned long)
        mov     rcx, rax
.L3:
        mov     QWORD PTR [rbp+0], rcx
        mov     DWORD PTR [rcx], ebx
        add     rsp, 8
        pop     rbx
        pop     rbp
        pop     r12
        pop     r13
        ret
.L2:
        lea     r13, [0+r12*4]
        mov     rdi, r13
        call    operator new[](unsigned long)
        mov     rcx, rax
        test    r12, r12
        je      .L3
        mov     rdx, r13
        xor     esi, esi
        mov     rdi, rax
        call    memset
        mov     rcx, rax
        jmp     .L3
main:
        push    r12
        mov     esi, edi
        push    rbp
        push    rbx
        sub     rsp, 32
        lea     rdi, [rsp+8]
        call    NonTrivial::NonTrivial(int) [complete object constructor]
        mov     rbx, QWORD PTR [rsp+8]
        lea     rdi, [rsp+16]
        mov     eax, DWORD PTR [rbx]
        lea     esi, [rax+1]
        call    NonTrivial::NonTrivial(int) [complete object constructor]
        mov     r12, QWORD PTR [rsp+16]
        lea     rdi, [rsp+24]
        mov     eax, DWORD PTR [r12]
        lea     esi, [rax+100]
        call    NonTrivial::NonTrivial(int) [complete object constructor]
        mov     rbp, QWORD PTR [rsp+24]
        mov     rdi, r12
        call    operator delete[](void*)
        mov     rdi, rbp
        mov     r12d, DWORD PTR [rbp+0]
        call    operator delete[](void*)
        mov     rdi, rbx
        call    operator delete[](void*)
        add     rsp, 32
        mov     eax, r12d
        pop     rbx
        pop     rbp
        pop     r12
        ret
```

</td>
</tr>
</table>

## Reference
For more information on the Circle compiler, visit [Circle Lang](https://www.circle-lang.org/).
