# MyLLVMPass

An out-of-tree **LLVM New Pass Manager** pass project built on Ubuntu (WSL 2).

## Overview

This repository demonstrates how to hook into LLVM's New Pass Manager (`opt`) to analyze and transform LLVM Intermediate Representation (IR), including:
* **Function & Opcode Analysis:** Inspecting basic blocks, instruction counts, and IR opcodes.
* **IR Transformation / Obfuscation:** Rewriting `add` instructions using `IRBuilder` and `replaceAllUsesWith`.

## Requirements

* LLVM (`llvm-dev`, `opt`)
* Clang
* CMake (3.20+) & Ninja

## Build & Run

    cmake -B build -G Ninja && cmake --build build
    clang -S -emit-llvm tests/test.c -o tests/test.ll
    opt -load-pass-plugin ./build/HelloWorldPass.so -passes="hello-world" -S tests/test.ll -o tests/test_modified.ll

## Verify Output

Compare the original and modified IR:

    diff tests/test.ll tests/test_modified.ll

Compile and run the modified IR:

    clang tests/test_modified.ll -o tests/test_modified && ./tests/test_modified
