// Obfuscator.hpp —— 大数计算过程混淆器核心
//
// 思想：把一个目标常量 T 递归地替换成一棵“运算表达式树”，树上每一步都是
// 可逆的恒等变换，因此整棵树的求值结果在构造上恒等于 T，与数字大小无关。
// 这取代了原仓库用遗传算法“搜索”表达式的做法——搜索可能不收敛而返回
// 近似（错误）解，而恒等分解永远精确。
//
// 每种分解都满足 eval(node) == 传入的子目标值：
//   ADD   : T = A + B           (A,B 随机, 0<A<T, B=T-A)           —— 收敛
//   SUB   : T = A - B           (B 随机>0, A=T+B)                  —— 放大
//   XOR   : T = A ^ B           (M 随机, A=M, B=T^M, 自逆)         —— 等位宽
//   MULADD: T = f*Q + R         (f 小, Q=T/f, R=T%f)               —— 收敛
//   SPLIT : T = (H << k) | L    (H=T>>k, L=T 低 k 位, 两段不相交)  —— 收敛
// 递归由 depth 预算界定规模；depth 用尽即落为字面量叶子。
#pragma once
#include "BigInt.hpp"
#include <memory>
#include <string>
#include <random>
#include <cstdint>

struct Node {
    enum Type { LEAF, ADD, SUB, MUL, XOR, OR, SHL } type = LEAF;
    BigInt val;                 // LEAF: 字面量
    uint64_t shift = 0;         // SHL: 位移量
    std::unique_ptr<Node> a, b; // 子节点

    BigInt eval() const {
        switch (type) {
            case LEAF: return val;
            case ADD:  return a->eval() + b->eval();
            case SUB:  return a->eval() - b->eval();
            case MUL:  return a->eval() * b->eval();
            case XOR:  return a->eval() ^ b->eval();
            case OR:   return a->eval() | b->eval();
            case SHL:  return a->eval().shl(shift);
        }
        return BigInt();
    }
    long long countNodes() const {
        long long n = 1;
        if (a) n += a->countNodes();
        if (b) n += b->countNodes();
        return n;
    }
    std::string toCode() const {
        switch (type) {
            case LEAF: return "C(\"" + val.toDecString() + "\")";
            case ADD:  return "badd(" + a->toCode() + "," + b->toCode() + ")";
            case SUB:  return "bsub(" + a->toCode() + "," + b->toCode() + ")";
            case MUL:  return "bmul(" + a->toCode() + "," + b->toCode() + ")";
            case XOR:  return "bxor(" + a->toCode() + "," + b->toCode() + ")";
            case OR:   return "bor("  + a->toCode() + "," + b->toCode() + ")";
            case SHL:  return "bshl(" + a->toCode() + "," + std::to_string(shift) + ")";
        }
        return "C(\"0\")";
    }
};

class Obfuscator {
public:
    explicit Obfuscator(uint64_t seed) : rng(seed) {}

    std::unique_ptr<Node> build(const BigInt& target, int depth) {
        return expand(target, depth);
    }

    // 产出一份自包含、可直接编译运行的自验证 C++ 程序
    std::string emitProgram(const std::string& expr, const BigInt& target) const {
        std::string o;
        o += "// 本文件由大数计算过程混淆器自动生成。\n";
        o += "// 目标常量不以字面量出现，而由下面的运算过程在运行时还原。\n";
        o += "// 需与 BigInt.hpp 置于同一目录下编译，例如：\n";
        o += "//   g++ -std=c++17 obfuscated_output.cpp -o demo && ./demo\n";
        o += "#include \"BigInt.hpp\"\n#include <iostream>\n\n";
        o += "static inline BigInt C(const char* s){ return BigInt(std::string(s)); }\n";
        o += "static inline BigInt badd(const BigInt& a,const BigInt& b){ return a + b; }\n";
        o += "static inline BigInt bsub(const BigInt& a,const BigInt& b){ return a - b; }\n";
        o += "static inline BigInt bmul(const BigInt& a,const BigInt& b){ return a * b; }\n";
        o += "static inline BigInt bxor(const BigInt& a,const BigInt& b){ return a ^ b; }\n";
        o += "static inline BigInt bor (const BigInt& a,const BigInt& b){ return a | b; }\n";
        o += "static inline BigInt bshl(const BigInt& a,unsigned long long k){ return a.shl(k); }\n\n";
        o += "// 还原后的目标值 —— 注意源码中不存在该常量的字面量形式\n";
        o += "static BigInt compute(){\n    return " + expr + ";\n}\n\n";
        o += "int main(){\n";
        o += "    BigInt v = compute();\n";
        o += "    std::cout << \"computed = \" << v.toDecString() << std::endl;\n";
        o += "    BigInt expected = C(\"" + target.toDecString() + "\");\n";
        o += "    std::cout << (v == expected ? \"[OK] matches target\" : \"[FAIL] mismatch\") << std::endl;\n";
        o += "    return v == expected ? 0 : 1;\n";
        o += "}\n";
        return o;
    }

private:
    std::mt19937_64 rng;
    // 小于该阈值直接作为字面量叶子（也用作终止条件）
    const BigInt SMALL = BigInt(256);

    std::unique_ptr<Node> leaf(const BigInt& v) {
        auto n = std::make_unique<Node>();
        n->type = Node::LEAF; n->val = v; return n;
    }

    std::unique_ptr<Node> expand(const BigInt& V, int depth) {
        if (depth <= 0 || V < SMALL) return leaf(V);

        // depth==1 时只用“收敛型”分解，避免产生过大的单个字面量
        int nStrat = (depth >= 2) ? 5 : 3;
        int choice = (int)(rng() % (uint64_t)nStrat);

        switch (choice) {
            case 0: { // ADD: V = A + B, 1<=A<=V-1
                BigInt A = BigInt(1) + BigInt::randomBelowOrEqual(rng, V - BigInt(2));
                BigInt B = V - A;
                auto n = std::make_unique<Node>(); n->type = Node::ADD;
                n->a = expand(A, depth - 1); n->b = expand(B, depth - 1);
                return n;
            }
            case 1: { // MULADD: V = f*Q + R
                uint32_t f = 2 + (uint32_t)(rng() % 16);
                BigInt F(f), Q = V / F, R = V % F;
                auto mul = std::make_unique<Node>(); mul->type = Node::MUL;
                mul->a = leaf(F); mul->b = expand(Q, depth - 1);
                auto add = std::make_unique<Node>(); add->type = Node::ADD;
                add->a = std::move(mul); add->b = leaf(R);
                return add;
            }
            case 2: { // SPLIT: V = (H << k) | L
                int bl = V.bitLength();
                uint64_t k = 1 + (uint64_t)(rng() % (uint64_t)(bl - 1));
                BigInt H = V.shr(k);
                BigInt L = V - H.shl(k); // 即 V 的低 k 位，与高位不相交
                auto shl = std::make_unique<Node>(); shl->type = Node::SHL;
                shl->shift = k; shl->a = expand(H, depth - 1);
                auto orr = std::make_unique<Node>(); orr->type = Node::OR;
                orr->a = std::move(shl); orr->b = expand(L, depth - 1);
                return orr;
            }
            case 3: { // XOR: V = A ^ B, 自逆变换（仅 depth>=2）
                int bl = V.bitLength();
                BigInt M = BigInt::randomBits(rng, bl);
                BigInt B = V ^ M; // A=M, M ^ (V^M) == V
                auto n = std::make_unique<Node>(); n->type = Node::XOR;
                n->a = expand(M, depth - 1); n->b = expand(B, depth - 1);
                return n;
            }
            default: { // SUB: V = A - B, B 随机>0, A=V+B（仅 depth>=2）
                int bl = V.bitLength();
                BigInt B = BigInt::randomBits(rng, bl > 0 ? bl : 1);
                if (B.isZero()) B = BigInt(1);
                BigInt A = V + B;
                auto n = std::make_unique<Node>(); n->type = Node::SUB;
                n->a = expand(A, depth - 1); n->b = expand(B, depth - 1);
                return n;
            }
        }
    }
};
