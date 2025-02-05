// Generater.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <Windows.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <tuple>
#include <string>
#include <algorithm>
#include <random>
#include <sstream>
#include <stack>
#include <cmath>
#include <functional>
#include <unordered_map>
// 实现自定义运算
long long int S_add(long long int a, long long int b)
{
    return a + b;
}

long long int S_sub(long long int a, long long int b)
{
    return a - b;
}

long long int S_mul(long long int a, long long int b)
{
    return a * b;
}

long long int S_div(long long int a, long long int b)
{
    return a / b;
}

long long int S_mod(long long int a, long long int b)
{
    return a % b;
}

long long int S_pow(long long int a, long long int b)
{
    return pow(a, b);
}

long long int S_and(long long int a, long long int b)
{
    return a & b;
}

long long int S_or(long long int a, long long int b)
{
    return a | b;
}

long long int S_xor(long long int a, long long int b)
{
    return a ^ b;
}


long long int S_lshift(long long int a, long long int b)
{
    return a << b;
}

long long int S_rshift(long long int a, long long int b)
{
    return a >> b;
}

std::string generateExpressionCode(const std::vector<long long int>& numbers, const std::vector<std::string>& ops) {
    std::string code = std::to_string(numbers[0]);
    for (size_t i = 1; i < numbers.size(); ++i) {
        if (ops[i - 1] == "+") {
            code = "S_add(" + code + "," + std::to_string(numbers[i]) + ")";
        }
        else if (ops[i - 1] == "-") {
            code = "S_sub(" + code + "," + std::to_string(numbers[i]) + ")";
        }
        else if (ops[i - 1] == "*") {
            code = "S_mul(" + code + "," + std::to_string(numbers[i]) + ")";
        }
        else if (ops[i - 1] == "/") {
            code = "S_div(" + code + "," + std::to_string(numbers[i]) + ")";
        }
        else if (ops[i - 1] == "%") {
            code = "S_mod(" + code + "," + std::to_string(numbers[i]) + ")";
        }
        else if (ops[i - 1] == "^") {
            code = "S_pow(" + code + "," + std::to_string(numbers[i]) + ")";
		}
		else if (ops[i - 1] == "x") {
			code = "S_xor(" + code + "," + std::to_string(numbers[i]) + ")";
        }
        else if(ops[i -1] == "a")
        {
            code = "S_and(" + code + "," + std::to_string(numbers[i]) + ")";
        }
        else if (ops[i - 1] == "o")
        {
            code = "S_or(" + code + "," + std::to_string(numbers[i]) + ")";
        }
        else if (ops[i - 1] == "r")
        {
			code = "S_rshift(" + code + "," + std::to_string(numbers[i]) + ")";
        }
        else if (ops[i - 1] == "l")
        {
			code = "S_lshift(" + code + "," + std::to_string(numbers[i]) + ")";
        }
    }
    return code;
}


void parseCode(const std::string& code, std::vector<long long int>& numbers, std::vector<std::string>& ops) {
    std::string temp = code;
    size_t pos = 0;
    std::string token;
    std::string delimiters = "(),";

    while ((pos = temp.find_first_of(delimiters)) != std::string::npos) {
        token = temp.substr(0, pos);
        if (!token.empty()) {
            if (token.find("S_") != std::string::npos) {
                if (token == "S_add") {
                    ops.push_back("+");
                }
                else if (token == "S_sub") {
                    ops.push_back("-");
                }
                else if (token == "S_mul") {
                    ops.push_back("*");
                }
                else if (token == "S_div") {
                    ops.push_back("/");
                }
                else if (token == "S_mod") {
                    ops.push_back("%");
                }
                else if (token == "S_pow") {
                    ops.push_back("^");
                }
                else if (token == "S_xor")
                {
					ops.push_back("x");
				}
				else if (token == "S_and")
				{
					ops.push_back("a");
				}
				else if (token == "S_or")
				{
					ops.push_back("o");
				}
                else if (token == "S_rshift")
                {
                    ops.push_back("r");
                }
                else if (token == "S_lshift")
                {
                    ops.push_back("l");
                }
            }
            else {
                numbers.push_back(std::stoll(token));
            }
        }
        temp.erase(0, pos + 1);
    }
    if (!temp.empty()) {
        numbers.push_back(std::stoll(temp));
    }

    // 反转 ops 的顺序
    std::reverse(ops.begin(), ops.end());
}







// 个体表示
struct Individual {
    std::vector<long long int> numbers;
    std::vector<std::string> ops;
    long long int fitness = 0; // 初始化 fitness
};

// 适应度函数
long long int calculateFitness(const Individual& individual, long long int target) {
    long long int result = individual.numbers[0];
    for (size_t i = 1; i < individual.numbers.size(); ++i) {
        if (individual.ops[i - 1] == "+") {
            result = S_add(result, individual.numbers[i]);
        }
        else if (individual.ops[i - 1] == "-") {
            result = S_sub(result, individual.numbers[i]);
        }
        else if (individual.ops[i - 1] == "*") {
            result = S_mul(result, individual.numbers[i]);
        }
        else if (individual.ops[i - 1] == "/") {
            result = S_div(result, individual.numbers[i]);
        }
        else if (individual.ops[i - 1] == "%") {
            result = S_mod(result, individual.numbers[i]);
        }
        else if (individual.ops[i - 1] == "^") {
            result = S_pow(result, individual.numbers[i]);
        }
        else if (individual.ops[i - 1] == "x")
        {
            result = S_xor(result, individual.numbers[i]);
        }
        else if (individual.ops[i - 1] == "a")
        {
			result = S_and(result, individual.numbers[i]);
        }
        else if (individual.ops[i - 1] == "o")
        {
			result = S_or(result, individual.numbers[i]);
        }
        else if (individual.ops[i - 1] == "r")
        {
			result = S_rshift(result, individual.numbers[i]);
		}
        else if (individual.ops[i - 1] == "l")
        {
            result = S_lshift(result, individual.numbers[i]);
        }
    }
    return std::abs(target - result);
}

// 初始化种群
std::vector<Individual> initializePopulation(int populationSize, int numLength, const std::vector<long long int>& numbers, const std::vector<std::string>& ops) {
    std::vector<Individual> population;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> numDist(0, numbers.size() - 1);
    std::uniform_int_distribution<> opDist(0, ops.size() - 1);

    for (int i = 0; i < populationSize; ++i) {
        Individual individual;
        for (int j = 0; j < numLength; ++j) {
            individual.numbers.push_back(numbers[numDist(gen)]);
            if (j < numLength - 1) {
                individual.ops.push_back(ops[opDist(gen)]);
            }
        }
        population.push_back(individual);
    }
    return population;
}


// 选择操作
Individual select(const std::vector<Individual>& population) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, population.size() - 1);
    return population[dist(gen)];
}

// 交叉操作
Individual crossover(const Individual& parent1, const Individual& parent2) {
    Individual offspring;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, parent1.numbers.size() - 1);
    int crossoverPoint = dist(gen);

    for (int i = 0; i < crossoverPoint; ++i) {
        offspring.numbers.push_back(parent1.numbers[i]);
        if (i < crossoverPoint - 1) {
            offspring.ops.push_back(parent1.ops[i]);
        }
    }
    for (int i = crossoverPoint; i < parent2.numbers.size(); ++i) {
        offspring.numbers.push_back(parent2.numbers[i]);
        if (i < parent2.numbers.size() - 1) {
            offspring.ops.push_back(parent2.ops[i]);
        }
    }
    return offspring;
}

// 变异操作
void mutate(Individual& individual, const std::vector<long long int>& numbers, const std::vector<std::string>& ops) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> numDist(0, numbers.size() - 1);
    std::uniform_int_distribution<> opDist(0, ops.size() - 1);
    std::uniform_real_distribution<> mutationChance(0.0, 1.0);

    for (size_t i = 0; i < individual.numbers.size(); ++i) {
        if (mutationChance(gen) < 0.1) { // 10% 变异概率
            individual.numbers[i] = numbers[numDist(gen)];
        }
    }
    for (size_t i = 0; i < individual.ops.size(); ++i) {
        if (mutationChance(gen) < 0.1) { // 10% 变异概率
            individual.ops[i] = ops[opDist(gen)];
        }
    }
}
long long int calcExpression(std::vector<long long int> numbers, std::vector<std::string> ops) {
   // std::string expression;
   // for (int i = 0; i < numbers.size(); i++) {
   //    expression += std::to_string(numbers[i]);
   //    if (i < numbers.size() - 1) {
		 //  expression += ops[i];
   //    }
   //}
   //std::cout << expression << std::endl;
    long long int result = numbers[0];
    for (size_t i = 1; i < numbers.size(); ++i) {
        if (ops[i - 1] == "+") {
            result = S_add(result, numbers[i]);
        }
        else if (ops[i - 1] == "-") {
            result = S_sub(result, numbers[i]);
        }
        else if (ops[i - 1] == "*") {
            result = S_mul(result, numbers[i]);
        }
        else if (ops[i - 1] == "/") {
            result = S_div(result, numbers[i]);
        }
        else if (ops[i - 1] == "%") {
            result = S_mod(result, numbers[i]);
        }
        else if (ops[i - 1] == "^") {
            result = S_pow(result, numbers[i]);
        }
        else if (ops[i - 1] == "x")
        {
			result = S_xor(result, numbers[i]);
		}
        else if (ops[i - 1] == "a")
        {
            result = S_and(result, numbers[i]);
		}
		else if (ops[i - 1] == "o")
		{
			result = S_or(result, numbers[i]);
		}
		else if (ops[i - 1] == "r")
		{
			result = S_rshift(result, numbers[i]);
		}
		else if (ops[i - 1] == "l")
		{
			result = S_lshift(result, numbers[i]);
		}
    }
    return result;
}

// 遗传算法实现
std::string generateFromResult(long long int result, int nums,int mix,int max,int iPopulation, int iGeneration) {
    const int populationSize = iPopulation;
    const int numLength = nums;
    const int generations = iGeneration;
    std::vector<long long int> numbers;
	// 为numbers赋值
	for (int i = 1; i+mix < max; i++) {
		numbers.push_back(i + mix);
	}

    const std::vector<std::string> ops = { "+", "-", "*", "/", "%", "^","x","a","o","r","l"};

    std::vector<Individual> population = initializePopulation(populationSize, numLength, numbers, ops);

    for (int generation = 0; generation < generations; ++generation) {
        for (auto& individual : population) {
            individual.fitness = calculateFitness(individual, result);
        }

        std::sort(population.begin(), population.end(), [](const Individual& a, const Individual& b) {
            return a.fitness < b.fitness;
            });

        if (population[0].fitness == 0) {
			std::cout << "在" << generation << "次迭代后找到最优解" << std::endl;
			std::cout << "最优解直接计算结果: " << calcExpression(population[0].numbers, population[0].ops) << std::endl;
            return generateExpressionCode(population[0].numbers, population[0].ops);
        }

        std::vector<Individual> newPopulation;
        for (int i = 0; i < populationSize / 2; ++i) {
            Individual parent1 = select(population);
            Individual parent2 = select(population);
            Individual offspring = crossover(parent1, parent2);

            // 确保 offspring 的 numbers 和 ops 的大小一致
            if (offspring.numbers.size() > 1 && offspring.ops.size() != offspring.numbers.size() - 1) {
                offspring.ops.resize(offspring.numbers.size() - 1);
            }

            mutate(offspring, numbers, ops);
            newPopulation.push_back(offspring);
        }

        population = newPopulation;
    }
	std::cout << "迭代未找到最优解 取近似解" << std::endl;
    return generateExpressionCode(population[0].numbers, population[0].ops);
}




int main() {
    //srand(time(NULL));

    //// 生成随机数 > 100000
    //std::vector<long long int> numbers;
    //// 生成随机数
    //int num = 1000;
    //for (int i = 0; i < num; i++) {
    //    numbers.push_back(rand() % 100000 + 1);
    //}
    //// 生成随机运算符
    //std::vector<std::string> op{ "+", "-", "*", "/", "%", "^" };
    //// 生成随机表达式
    //std::string expression;
    //std::vector<std::string> ops;
    //for (int i = 0; i < num; i++) {
    //    expression += std::to_string(numbers[i]);
    //    if (i < num - 1) {
    //        int opIndex = rand() % op.size();
    //        expression += op[opIndex];
    //        ops.push_back(op[opIndex]);
    //    }
    //}
    //std::cout << expression << std::endl;

    //// 使用代码的方式输出表达式
    //std::string codeExpression = generateExpressionCode(numbers, ops);
    //std::cout << "Generated Code: " << codeExpression << std::endl;

    //// 计算表达式
    //std::string result = expression;
    //for (int i = 0; i < num - 1; i++) {
    //    long long int a = numbers[i];
    //    long long int b = numbers[i + 1];
    //    std::string op = ops[i];
    //    long long int res = 0;
    //    if (op == "+") {
    //        res = S_add(a, b);
    //    }
    //    else if (op == "-") {
    //        res = S_sub(a, b);
    //    }
    //    else if (op == "*") {
    //        res = S_mul(a, b);
    //    }
    //    else if (op == "/") {
    //        res = S_div(a, b);
    //    }
    //    else if (op == "%") {
    //        res = S_mod(a, b);
    //    }
    //    else if (op == "^") {
    //        res = S_pow(a, b);
    //    }
    //    result = std::to_string(res);
    //}
    //std::cout << "Result: " << result << std::endl;
    int num = 100;
	int number = 1000;
    int mix = 10;
    int max = 100;
    int populationSize = 5000;
	int generations = 10000;
	std::string code = generateFromResult(number, num, mix, max, populationSize, generations);
    std::cout << "遗传算法逆推 取值区间为:["<<mix << "-" << max << "] 预期的结果:" << number << " 数字数" << num << "\n 生成的Code : " << code << std::endl;
    std::vector<long long int> numbers;
    std::vector<std::string> ops;
    parseCode(code, numbers, ops);
	


	std::cout << "由Code解析计算结果: " << calcExpression(numbers, ops) << std::endl;
    return 0;
}
