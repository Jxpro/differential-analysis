#include <bits/stdc++.h>
#include "desTools.h"
using namespace std;

/*
 * 仅分析(φ,0) -> (0,φ) -> (φ,0)的情况
 */

// 差分分布表
int distribution[8][64];
int min_times;

// 记录s盒可能的输入及概率(输出固定为0)
typedef struct item
{
    uint64_t input;
    double probability;
} item;

// 记录最终结果
typedef struct result
{
    uint64_t input;
    uint32_t output;
    double probability;
} result;

// 读取差分分布表
void readDistribution()
{
    ifstream myfile("desDistribution.txt", ios::in);
    for (int i = 0; i < 8; i++)
    {
        string str;
        getline(myfile, str);
        for (int j = 0; j < 64; j++)
        {
            getline(myfile, str);
            // 取输出差分为0的频数，并赋值给distribution[i][j]
            stringstream ss(str.substr(7, str.size() - 8));
            string temp;
            getline(ss, temp, ',');
            distribution[i][j] = stoi(temp);
        }
    }
}

// 打印差分分布表
void printDistribution()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            printf("%d ", distribution[i][j]);
            if (j % 8 == 7)
                printf("\n");
        }
        printf("\n");
    }
}

void statistics()
{
    // 计算distribution每行中，元素大于min_times的个数
    int count[8] = {0};
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            if (distribution[i][j] > min_times)
            {
                count[i]++;
            }
        }
    }
    printf("The number of elements in distribution table that are greater than %d is:\n", min_times);
    for (int i = 0; i < 8; i++)
    {
        printf("%d ", count[i]);
    }
    printf("\n\n");
}

// 校验是否符合E扩展
bool E_verify(int pre, int after)
{
    bool first_unmatch = (pre & 1) ^ (after >> 4 & 1);
    bool second_unmatch = (pre >> 1 & 1) ^ (after >> 5 & 1);
    return !(first_unmatch || second_unmatch);
}

// 对于输出差为全0，且保证频率大于min_times,获取s盒所有可能的输入及概率
void get_possible_input(int s_index, uint64_t input, double probability,
                        vector<item> &results, vector<int> input_parts, vector<int> times_parts)
{
    // debug 关键点
    // if (input == 0xf2b00000000)
    //     cout << "key point" << endl;
    if (probability < 1e-6)
        return;
    if (s_index == 8)
    {
        if (E_verify(input & 0b111111, input >> 42 & 0b111111))
        {
            results.push_back(item{input, probability});
            if (results.size() % 20000 == 0)
            {
                printf("Have found %llu results\n", results.size());
            }
        }
        return;
    }

    // 记录上一轮的输入和概率，避免在循环中被覆盖或者重复计算
    uint64_t next_input = input << 6;

    for (int i = 0; i < 64; i++, next_input++)
    {
        int times = distribution[s_index][i];
        // 对于不符合E扩展的情况，需要剪枝以降低复杂度
        if (times > min_times && (E_verify(input & 0b111111, i) || s_index == 0))
        {
            // input_parts和times_parts是为了记录每一轮的输入和概率，方便调试
            // input_parts.push_back(i);
            // times_parts.push_back(times);

            get_possible_input(s_index + 1, next_input, probability * times / 64.0, results, input_parts, times_parts);

            // input_parts.pop_back();
            // times_parts.pop_back();
        }
    }
}

// 对于输出差分为全0，计算s盒所有可能的输入对应的E扩展的逆变换，即明文差分
vector<result> compute()
{
    vector<result> plain_results;
    vector<item> s_results;

    // 记录中间信息用于debug
    vector<int> input_parts;
    vector<int> times_parts;

    // 获取s盒所有可能的输入及概率
    get_possible_input(0, 0, 1, s_results, input_parts, times_parts);
    for (auto s : s_results)
    {
        // 计算E扩展逆变换
        uint32_t L = E_reverse(s.input);
        plain_results.push_back(result{s.input, L, s.probability});
    }
    // 将results按照probability降序排序
    sort(plain_results.begin(), plain_results.end(), [](result a, result b)
         { return a.probability > b.probability; });
    return plain_results;
}

int main()
{
    // 设置S盒的最小计算频数
    min_times = 0;
    // 读取分布表
    readDistribution();
    // 打印分布表
    // printDistribution();
    // 统计分布表各s盒中，频数大于等于8的元素个数
    statistics();

    // 计算结果，并统计计算时间
    printf("Start computing...\n");
    auto start = chrono::high_resolution_clock::now();
    auto results = compute();
    auto end = chrono::high_resolution_clock::now();
    printf("End computing...\n\n");

    // 打印结果
    printf("Totle find: %llu\n", results.size());
    printf("Time: %.3f s\n", chrono::duration<double>(end - start).count());
    // 输出前10个结果
    printf("Top 10:\n");
    printf("%-17s%-17s%-17s\n", "S_input", "Plaintext", "Probability");
    for (int i = 0; i < 10; i++)
    {
        printf("%-#16llx %-#16x %.6lf\n", results[i].input, results[i].output, results[i].probability);
    }
    printf("\n");
    system("pause");
    return 0;
}