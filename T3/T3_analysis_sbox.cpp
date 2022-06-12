#include <bits/stdc++.h>
#include "desTools.h"
using namespace std;

// 差分分布表
int distribution[8][64][16];
int min_times;

// 记录s盒可能的输入输出及概率
typedef struct item
{
    uint64_t s_input;
    uint32_t s_output;
    double probability;
} item;

// 记录最终结果
typedef struct result
{
    uint32_t input;
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
            // 根据逗号分割16个数字,并赋值给distribution[i][j]
            stringstream ss(str.substr(7, str.size() - 8));
            for (int k = 0; k < 16; k++)
            {
                string temp;
                getline(ss, temp, ',');
                distribution[i][j][k] = stoi(temp);
            }
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
            for (int k = 0; k < 16; k++)
            {
                printf("%d ", distribution[i][j][k]);
            }
            printf("\n");
        }
        printf("\n");
    }
}

void statistics()
{
    // 计算distribution每行中，元素大于min_times的个数
    int count[8][64] = {0};
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            for (int k = 0; k < 16; k++)
            {
                if (distribution[i][j][k] > min_times)
                {
                    count[i][j]++;
                }
            }
        }
    }
    // 打印统计结果
    printf("The number of times that the distribution is greater than %d is:\n", min_times);
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            printf("%d ", count[i][j]);
            if (j % 8 == 7)
                printf("\n");
        }
        printf("\n");
    }
}

// 校验是否符合E扩展
bool E_verify(int pre, int after)
{
    bool first_unmatch = (pre & 1) ^ (after >> 4 & 1);
    bool second_unmatch = (pre >> 1 & 1) ^ (after >> 5 & 1);
    return !(first_unmatch || second_unmatch);
}

// 保证频率大于min_times,获取s盒所有可能的输入输出及概率
void get_possible_input(int s_index, uint64_t input, uint32_t output, double probability,
                        vector<item> &results, vector<int> input_parts, vector<int> output_parts, vector<int> times_parts)
{
    // debug 关键点
    // if (input == 0x8000000000)
    // {
    //     printf("input: %#llx P_permute(output)%#llx\n", input, P_permute(output));
    // }
    // if (probability < 1e-6)
    //     return;
    if (s_index == 8)
    {
        if (E_verify(input & 0b111111, input >> 42 & 0b111111))
        {
            results.push_back(item{input, P_permute(output), probability});
            if (results.size() % 5000 == 0)
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
        // 记录上一轮的输出
        uint32_t next_output = output << 4;
        for (int j = 0; j < 16; j++, next_output++)
        {
            int times = distribution[s_index][i][j];
            // 对于不符合E扩展的情况，需要剪枝以降低复杂度
            if (times > min_times && (E_verify(input & 0b111111, i) || s_index == 0))
            {
                // input_parts,output_parts和times_parts是为了记录每一轮的输入和概率，方便调试
                input_parts.push_back(i);
                output_parts.push_back(j);
                times_parts.push_back(times);

                get_possible_input(s_index + 1, next_input, next_output,
                                   probability * times / 64.0, results, input_parts, output_parts, times_parts);

                input_parts.pop_back();
                output_parts.pop_back();
                times_parts.pop_back();
            }
        }
    }
}

// 计算s盒所有可能的输入差分对应的E扩展的逆变换，即明文差分
vector<result> compute()
{
    vector<result> plain_results;
    vector<item> s_results;

    // 记录中间信息用于debug
    vector<int> input_parts;
    vector<int> output_parts;
    vector<int> times_parts;

    // 获取s盒所有可能的输入及概率
    get_possible_input(0, 0, 0, 1, s_results, input_parts, output_parts, times_parts);
    for (auto s : s_results)
    {
        // 计算E扩展逆变换
        uint32_t R = E_reverse(s.s_input);
        plain_results.push_back(result{R, s.s_output, s.probability});
    }
    // 将results按照probability降序排序
    sort(plain_results.begin(), plain_results.end(), [](result a, result b)
         { return a.probability > b.probability; });
    return plain_results;
}

int main()
{
    // 设置S盒的最小计算频数
    min_times = 13;
    // 读取分布表
    readDistribution();
    // 打印分布表
    // printDistribution();
    // 统计分布表各s盒中，频数大于等于8的元素个数
    // statistics();

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
    printf("%-17s%-17s%-17s\n", "R", "F(R)", "Probability");
    for (int i = 0; i < 10; i++)
    {
        printf("%-#16x %-#16x %.6lf\n", results[i].input, results[i].output, results[i].probability);
    }
    printf("\n");
    system("pause");
    return 0;
}