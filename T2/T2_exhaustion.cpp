#include <bits/stdc++.h>
#include "desTools.h"
using namespace std;

/*
 * 例举所有可能的情况
 */

// 差分分布表
int distribution[8][64][16];

typedef struct item
{
    uint32_t value;
    double probability;
} item;

typedef struct result
{
    uint32_t L;
    uint32_t R;
    double probability;
} result;

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

void get_possible_value(int *s_array, int s_index, uint32_t value, double probability, vector<item> &results)
{
    if (probability < 1e-6)
        return;
    if (s_index == 8)
    {
        results.push_back(item{P_permute(value), probability});
        return;
    }
    uint32_t next_value = value << 4;
    for (int i = 0; i < 16; i++, next_value++)
    {
        int times = distribution[s_index][s_array[s_index]][i];
        if (times != 0)
        {
            get_possible_value(s_array, s_index + 1, next_value, probability * times / 64, results);
        }
    }
}

vector<item> F(uint32_t text)
{
    vector<item> results;
    uint64_t E_extended = E_extend(text);
    int s_array[8] = {0};
    for (int i = 42; i >= 0; i -= 6)
    {
        s_array[7 - i / 6] = (E_extended >> i) & 0x3f;
    }
    get_possible_value(s_array, 0, 0, 1, results);
    return results;
}

vector<result> compute()
{
    vector<result> results;
    double best_probability = 0;
    for (uint32_t R = 0; R < (1ll << 32); R++)
    {
        for (uint32_t L = 0; L < (1ll << 32); L++)
        {
            if (L == 0 && R == 0)
                continue;
            if (L % (2 << 9) == 0)
                printf("computing %#x %#x\n", R, L);
            vector<item> first_result = F(R);
            for (item first_item : first_result)
            {
                if (first_item.probability < best_probability)
                    continue;
                uint32_t resultR = first_item.value ^ L;
                vector<item> second_result = F(resultR);
                for (item second_item : second_result)
                {
                    if (first_item.probability * second_item.probability < best_probability)
                        continue;
                    uint32_t resultL = second_item.value ^ R;
                    if (L == resultR && R == resultL)
                    {
                        // 记录最高概率，方便剪枝以降低复杂度
                        best_probability = first_item.probability * second_item.probability;
                        printf("get %#x %#x with probability %lf\n", resultL, resultR, first_item.probability * second_item.probability);
                        results.push_back(result{resultL, resultR, first_item.probability * second_item.probability});
                    }
                }
            }
        }
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
    // 计算结果
    compute();
    system("pause");
    return 0;
}