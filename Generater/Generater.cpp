// Generater.cpp —— 大数计算过程混淆器 / 命令行入口
//
// 用法:
//   Generater [target] [depth] [seed] [outfile]
//     target  : 十进制非负大整数（任意位数），默认示例见下
//     depth   : 递归分解深度（表达式规模随之增长），默认 6
//     seed    : 随机种子（相同种子 + 相同参数 => 完全可复现），默认 0xC0FFEE
//     outfile : 输出的自验证 C++ 程序文件名，默认 obfuscated_output.cpp
//
// 行为:
//   1. 把 target 分解为一棵恒等运算树（构造上保证求值 == target）；
//   2. 打印该混淆表达式，并对树做一次独立求值自检；
//   3. 写出一份可直接编译运行的自验证程序，其源码中不含 target 的字面量。
#include "BigInt.hpp"
#include "Obfuscator.hpp"
#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char** argv) {
    std::string target = "123456789012345678901234567890";
    int depth = 6;
    unsigned long long seed = 0xC0FFEEULL;
    std::string outfile = "obfuscated_output.cpp";

    try {
        if (argc > 1) target = argv[1];
        if (argc > 2) depth = std::stoi(argv[2]);
        if (argc > 3) seed = std::stoull(argv[3]);
        if (argc > 4) outfile = argv[4];

        if (depth < 0) depth = 0;

        BigInt T(target);
        if (T.neg) { std::cerr << "错误：目标必须是非负整数\n"; return 1; }

        Obfuscator ob(seed);
        std::unique_ptr<Node> tree = ob.build(T, depth);
        BigInt self = tree->eval();

        std::cout << "==== 大数计算过程混淆器 ====\n";
        std::cout << "目标  target : " << T.toDecString() << "  (" << T.bitLength() << " bits)\n";
        std::cout << "深度  depth  : " << depth << "    种子 seed : " << seed << "\n";
        std::cout << "节点  nodes  : " << tree->countNodes() << "\n";
        std::cout << "自检  verify : " << self.toDecString()
                  << (self == T ? "   [OK 恒等成立]" : "   [MISMATCH 不应发生]") << "\n\n";

        std::string expr = tree->toCode();
        std::cout << "混淆表达式 expression:\n" << expr << "\n\n";

        std::ofstream f(outfile);
        if (!f) { std::cerr << "错误：无法写出文件 " << outfile << "\n"; return 1; }
        f << ob.emitProgram(expr, T);
        f.close();
        std::cout << "已生成自验证程序: " << outfile
                  << "  （与 BigInt.hpp 同目录下 g++ -std=c++17 编译即可运行）\n";

        return self == T ? 0 : 2;
    } catch (const std::exception& e) {
        std::cerr << "异常: " << e.what() << "\n";
        return 1;
    }
}
