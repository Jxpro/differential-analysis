#include <bits/stdc++.h>
#include "desTools.h"
using namespace std;

// 差分分布表,记录最大的概率[0]以及对应的输出差分[1]
int distribution[8][64][2];

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
            // 记录每个s盒中，每一个输入差分的:
            // 最大输出概率(distribution[i][j][0])
            // 及输出差分(distribution[i][j][1])
            getline(myfile, str);
            stringstream ss(str.substr(7, str.size() - 8));
            int max_times = 0;
            int max_output = 0;
            for (int k = 0; k < 16; k++)
            {
                string temp;
                getline(ss, temp, ',');
                int times = stoi(temp);
                if (times > max_times)
                {
                    max_times = times;
                    max_output = k;
                }
            }
            distribution[i][j][0] = max_times;
            distribution[i][j][1] = max_output;
        }
    }
}

// 打印差分分布表
void printDistribution()
{
    for (int i = 0; i < 8; i++)
    {
        printf("s-box %d\n", i);
        for (int j = 0; j < 64; j++)
        {
            printf("input:%d times: %d output:%d\n", j, distribution[i][j][0], distribution[i][j][1]);
        }
        printf("\n");
    }
}

// 对于给定输入差分，计算s盒最大的输出差分及概率
void get_max_prob(int *s_array, uint32_t input, vector<result> &results)
{
    if (input % 0x1000000 == 0)
        printf("Computing input:%#x\n", input);
    uint32_t output = 0;
    double probability = 1;
    for (int i = 0; i < 8; i++)
    {
        output = output << 4;
        output += distribution[i][s_array[i]][1];
        probability *= distribution[i][s_array[i]][0] / 64.0;
        // 已知某些概率的情况下进行剪枝
        if (probability < 1e-2)
            return;
    }
    results.push_back({input, P_permute(output), probability});
    if (input % 0x100000 == 0)
        printf("input:%#x output:%#x probability:%lf\n", input, P_permute(output), probability);
}

// 遍历输入差分，找到最大概率的输出差分
vector<result> compute()
{
    vector<result> results;
    bool flag = true;
    for (uint32_t input = 0; input != 0 || flag; input++)
    {
        flag = false;
        uint64_t extended = E_extend(input);
        int s_array[8] = {0};
        for (int i = 42; i >= 0; i -= 6)
        {
            s_array[7 - i / 6] = (extended >> i) & 0x3f;
        }
        get_max_prob(s_array, input, results);
    }
    // 将results按照probability降序排序
    sort(results.begin(), results.end(), [](result a, result b)
         { return a.probability > b.probability; });
    return results;
}

int main()
{
    // 读取分布表
    readDistribution();
    // 打印分布表
    // printDistribution();

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
    for (int i = 0; i < 100; i++)
    {
        printf("%-#16x %-#16x %.6lf\n", results[i].input, results[i].output, results[i].probability);
    }
    printf("\n");
    system("pause");
    return 0;
}