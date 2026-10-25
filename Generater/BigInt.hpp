// BigInt.hpp —— 任意精度有符号大整数（基 2^32，小端存储）
//
// 这是“大数计算过程混淆器”的运行时核心：生成器（Generater）和它产出的
// 自验证程序都依赖这一个头文件。为保证可被 MSVC v143 和 g++/clang 一致编译，
// 只使用标准 C++17，不含任何平台相关代码。
//
// 设计约束：混淆器只处理非负整数，并保证所有中间值非负，因此位运算
// （AND/OR/XOR/移位）直接定义在非负数的二进制补码幅值上，语义清晰且与
// 生成代码的计算完全一致。
#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <random>
#include <stdexcept>

class BigInt {
public:
    std::vector<uint32_t> mag; // 幅值，小端，无前导 0（空 = 0）
    bool neg = false;          // 符号；0 恒为非负

    BigInt() = default;

    BigInt(long long v) {
        unsigned long long u;
        if (v < 0) { neg = true; u = (unsigned long long)(-(v + 1)) + 1ULL; }
        else       { u = (unsigned long long)v; }
        while (u) { mag.push_back((uint32_t)(u & 0xFFFFFFFFULL)); u >>= 32; }
        trim();
    }

    explicit BigInt(const std::string& s) { *this = fromDec(s); }

    // ---- 基本状态 ----
    void trim() {
        while (!mag.empty() && mag.back() == 0) mag.pop_back();
        if (mag.empty()) neg = false;
    }
    bool isZero() const { return mag.empty(); }
    BigInt abs() const { BigInt r = *this; r.neg = false; return r; }

    int bitLength() const {
        if (mag.empty()) return 0;
        uint32_t top = mag.back(); int b = 0;
        while (top) { ++b; top >>= 1; }
        return (int)(mag.size() - 1) * 32 + b;
    }
    bool testBit(int i) const {
        if (i < 0) return false;
        size_t w = (size_t)i / 32, o = (size_t)i % 32;
        if (w >= mag.size()) return false;
        return ((mag[w] >> o) & 1u) != 0;
    }

    // ---- 幅值级辅助 ----
    static int cmpMag(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
        if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
        for (size_t i = a.size(); i-- > 0;)
            if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
        return 0;
    }
    static std::vector<uint32_t> addMag(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
        std::vector<uint32_t> r; uint64_t carry = 0;
        size_t n = a.size() > b.size() ? a.size() : b.size();
        for (size_t i = 0; i < n || carry; ++i) {
            uint64_t s = carry;
            if (i < a.size()) s += a[i];
            if (i < b.size()) s += b[i];
            r.push_back((uint32_t)(s & 0xFFFFFFFFULL));
            carry = s >> 32;
        }
        return r;
    }
    static std::vector<uint32_t> subMag(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
        // 需 a >= b
        std::vector<uint32_t> r; int64_t borrow = 0;
        for (size_t i = 0; i < a.size(); ++i) {
            int64_t s = (int64_t)a[i] - borrow - (i < b.size() ? (int64_t)b[i] : 0);
            if (s < 0) { s += (int64_t)1 << 32; borrow = 1; } else borrow = 0;
            r.push_back((uint32_t)s);
        }
        while (!r.empty() && r.back() == 0) r.pop_back();
        return r;
    }

    // ---- 比较 ----
    int cmp(const BigInt& o) const {
        if (neg != o.neg) return neg ? -1 : 1;
        int c = cmpMag(mag, o.mag);
        return neg ? -c : c;
    }
    bool operator==(const BigInt& o) const { return cmp(o) == 0; }
    bool operator!=(const BigInt& o) const { return cmp(o) != 0; }
    bool operator< (const BigInt& o) const { return cmp(o) <  0; }
    bool operator<=(const BigInt& o) const { return cmp(o) <= 0; }
    bool operator> (const BigInt& o) const { return cmp(o) >  0; }
    bool operator>=(const BigInt& o) const { return cmp(o) >= 0; }

    // ---- 加减乘 ----
    BigInt operator-() const { BigInt r = *this; if (!r.isZero()) r.neg = !r.neg; return r; }

    BigInt operator+(const BigInt& o) const {
        BigInt r;
        if (neg == o.neg) { r.mag = addMag(mag, o.mag); r.neg = neg; }
        else {
            int c = cmpMag(mag, o.mag);
            if (c == 0) return BigInt();
            if (c > 0) { r.mag = subMag(mag, o.mag); r.neg = neg; }
            else       { r.mag = subMag(o.mag, mag); r.neg = o.neg; }
        }
        r.trim(); return r;
    }
    BigInt operator-(const BigInt& o) const { return *this + (-o); }

    BigInt operator*(const BigInt& o) const {
        if (isZero() || o.isZero()) return BigInt();
        std::vector<uint32_t> r(mag.size() + o.mag.size(), 0);
        for (size_t i = 0; i < mag.size(); ++i) {
            uint64_t carry = 0;
            for (size_t j = 0; j < o.mag.size(); ++j) {
                uint64_t cur = (uint64_t)r[i + j] + (uint64_t)mag[i] * o.mag[j] + carry;
                r[i + j] = (uint32_t)(cur & 0xFFFFFFFFULL);
                carry = cur >> 32;
            }
            size_t k = i + o.mag.size();
            while (carry && k < r.size()) {
                uint64_t cur = (uint64_t)r[k] + carry;
                r[k] = (uint32_t)(cur & 0xFFFFFFFFULL);
                carry = cur >> 32; ++k;
            }
        }
        BigInt res; res.mag = r; res.neg = (neg != o.neg); res.trim(); return res;
    }

    // ---- 移位（非负逻辑移位）----
    BigInt shl(uint64_t n) const {
        if (isZero() || n == 0) return *this;
        uint64_t w = n / 32, b = n % 32;
        std::vector<uint32_t> r(mag.size() + w + 1, 0);
        for (size_t i = 0; i < mag.size(); ++i) {
            uint64_t v = (uint64_t)mag[i] << b;
            r[i + w]     |= (uint32_t)(v & 0xFFFFFFFFULL);
            r[i + w + 1] |= (uint32_t)(v >> 32);
        }
        BigInt res; res.mag = r; res.neg = neg; res.trim(); return res;
    }
    BigInt shr(uint64_t n) const {
        uint64_t w = n / 32, b = n % 32;
        if (n == 0) return *this;
        if (w >= mag.size()) return BigInt();
        std::vector<uint32_t> r(mag.size() - w, 0);
        for (size_t i = 0; i < r.size(); ++i) {
            uint64_t v = mag[i + w] >> b;
            if (b && i + w + 1 < mag.size())
                v |= (uint64_t)mag[i + w + 1] << (32 - b);
            r[i] = (uint32_t)(v & 0xFFFFFFFFULL);
        }
        BigInt res; res.mag = r; res.neg = neg; res.trim(); return res;
    }

    // ---- 位运算（要求两操作数非负）----
    BigInt bitop(const BigInt& o, int kind) const { // 0=and 1=or 2=xor
        size_t n = mag.size() > o.mag.size() ? mag.size() : o.mag.size();
        std::vector<uint32_t> r(n, 0);
        for (size_t i = 0; i < n; ++i) {
            uint32_t x = i < mag.size()   ? mag[i]   : 0;
            uint32_t y = i < o.mag.size() ? o.mag[i] : 0;
            r[i] = kind == 0 ? (x & y) : kind == 1 ? (x | y) : (x ^ y);
        }
        BigInt res; res.mag = r; res.neg = false; res.trim(); return res;
    }
    BigInt operator&(const BigInt& o) const { return bitop(o, 0); }
    BigInt operator|(const BigInt& o) const { return bitop(o, 1); }
    BigInt operator^(const BigInt& o) const { return bitop(o, 2); }

    // ---- 除法 / 取模（二进制长除法，操作数非负，除数非零）----
    static void divmod(const BigInt& A, const BigInt& B, BigInt& Q, BigInt& R) {
        if (B.isZero()) throw std::runtime_error("division by zero");
        Q = BigInt(); R = BigInt();
        int n = A.bitLength();
        for (int i = n - 1; i >= 0; --i) {
            R = R.shl(1);
            if (A.testBit(i)) { if (R.mag.empty()) R.mag.push_back(1u); else R.mag[0] |= 1u; }
            if (cmpMag(R.mag, B.mag) >= 0) {
                R.mag = subMag(R.mag, B.mag); R.trim();
                size_t w = (size_t)i / 32;
                if (w >= Q.mag.size()) Q.mag.resize(w + 1, 0);
                Q.mag[w] |= (1u << ((size_t)i % 32));
            }
        }
        Q.trim(); R.trim();
    }
    BigInt operator/(const BigInt& o) const {
        BigInt q, r; divmod(abs(), o.abs(), q, r);
        q.neg = (neg != o.neg) && !q.isZero(); return q;
    }
    BigInt operator%(const BigInt& o) const {
        BigInt q, r; divmod(abs(), o.abs(), q, r);
        r.neg = neg && !r.isZero(); return r;
    }

    // ---- 小数值原地运算（用于十进制 I/O）----
    void mul_small(uint32_t m) {
        uint64_t carry = 0;
        for (size_t i = 0; i < mag.size(); ++i) {
            uint64_t cur = (uint64_t)mag[i] * m + carry;
            mag[i] = (uint32_t)(cur & 0xFFFFFFFFULL); carry = cur >> 32;
        }
        while (carry) { mag.push_back((uint32_t)(carry & 0xFFFFFFFFULL)); carry >>= 32; }
        trim();
    }
    void add_small(uint32_t a) {
        uint64_t carry = a;
        for (size_t i = 0; i < mag.size() && carry; ++i) {
            uint64_t cur = (uint64_t)mag[i] + carry;
            mag[i] = (uint32_t)(cur & 0xFFFFFFFFULL); carry = cur >> 32;
        }
        while (carry) { mag.push_back((uint32_t)(carry & 0xFFFFFFFFULL)); carry >>= 32; }
    }
    uint32_t divmod_small(uint32_t d) { // 原地除以 d，返回余数（非负）
        uint64_t rem = 0;
        for (size_t i = mag.size(); i-- > 0;) {
            uint64_t cur = (rem << 32) | mag[i];
            mag[i] = (uint32_t)(cur / d); rem = cur % d;
        }
        trim(); return (uint32_t)rem;
    }

    // ---- 十进制 I/O ----
    std::string toDecString() const {
        if (isZero()) return "0";
        BigInt t = abs();
        std::vector<uint32_t> chunks;
        while (!t.isZero()) chunks.push_back(t.divmod_small(1000000000u));
        std::string s = (neg ? "-" : "") + std::to_string(chunks.back());
        for (size_t i = chunks.size() - 1; i-- > 0;) {
            std::string c = std::to_string(chunks[i]);
            s += std::string(9 - c.size(), '0') + c;
        }
        return s;
    }
    static BigInt fromDec(const std::string& s) {
        BigInt r; size_t i = 0; bool neg = false;
        if (i < s.size() && (s[i] == '+' || s[i] == '-')) { neg = (s[i] == '-'); ++i; }
        if (i >= s.size()) throw std::runtime_error("empty number");
        for (; i < s.size(); ++i) {
            if (s[i] < '0' || s[i] > '9') throw std::runtime_error("invalid decimal digit");
            r.mul_small(10); r.add_small((uint32_t)(s[i] - '0'));
        }
        r.neg = neg && !r.isZero(); return r;
    }

    // ---- 随机（生成器使用，拒绝采样保证均匀）----
    static BigInt randomBits(std::mt19937_64& rng, int bits) {
        if (bits <= 0) return BigInt();
        int words = (bits + 31) / 32;
        BigInt r; r.mag.resize(words);
        for (auto& w : r.mag) w = (uint32_t)(rng() & 0xFFFFFFFFULL);
        int top = bits % 32;
        if (top) r.mag.back() &= ((1u << top) - 1u);
        r.trim(); return r;
    }
    static BigInt randomBelowOrEqual(std::mt19937_64& rng, const BigInt& n) { // [0, n]
        if (n.isZero()) return BigInt();
        int bl = n.bitLength();
        for (;;) { BigInt r = randomBits(rng, bl); if (r <= n) return r; }
    }
};
