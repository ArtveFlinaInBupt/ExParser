# ExParser

> Expression Parser – LL(1)

这是一份北京邮电大学计算机科学与技术专业 2021 级大三上（2023–2024 学年度秋季学期）《编译原理与技术》课程实验。

实验题目是对算术表达式的递归下降分析和 LL(1) 分析二选一，本项目选择了 LL(1) 分析。

算术表达式的定义十分 trivial：

- 包含数字、加法、减法、乘法、除法、括号
- 括号可以嵌套，具有最高优先级
- 除法和乘法具有相同优先级，高于加法和减法

更加详细的说明（文法定义）参见[报告](doc.pdf)。

## 概述

这是一个 LL(1) 语法分析器。它只在本次实验题目给定的算术表达式文法上进行了测试。

## 构建和运行

可以使用 CMake 和提供的 CMakeLists.txt 进行构建，也可以直接编译并链接 `src/` 目录下的所有 `.cpp` 文件。

构建后直接运行，根据提示向 stdin 输入算术表达式，并查看 stdout 的输出。

```shell
./ExParser
```

## 已知的问题

- 文法的起始符号 hardcoded 在了代码里。这个问题在后来的 ExParserR 中得到了解决。
- 输出实际上不够美观，代码中存在一定的耦合。这个问题在后来的 ExParserR 中得到了解决。
- 输入含有词法错误的表达式（如含有拉丁字母）时，程序会直接抛出异常（回显 Lex error）并退出。老实说我其实不觉得这是问题。
- 程序没有在更多文法上进行测试。

## 参见

- [ExParserR](https://github.com/ArtveFlinaInBupt/ExParserR)：一个 LR(1) 语法分析器。
