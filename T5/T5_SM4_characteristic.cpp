#include <bits/stdc++.h>
using namespace std;

map<uint32_t, uint32_t> Lmap;
int distribution[256][256];

// 循环左移
uint32_t rotate_left_shift(uint32_t text, int number)
{
    return (text << number) | (text >> (32 - number));
}

// L移位变换
uint32_t shift(uint32_t text)
{
    return text ^ rotate_left_shift(text, 2) ^ rotate_left_shift(text, 10) ^
           rotate_left_shift(text, 18) ^ rotate_left_shift(text, 24);
}

// 建立L逆置换映射表(a1,a2,a3,0)格式
void buildLmap()
{
    for (uint32_t i = 0; i < 1 << 24; i++)
    {
        Lmap[shift(i)] = i;
    }
}

// 读取差分分布表
void readDistribution()
{
    ifstream myfile("sm4Distribution.txt", ios::in);
    for (int i = 0; i < 256; i++)
    {
        string str;
        string temp;
        getline(myfile, str);
        // 根据逗号分割64个数字,并赋值给distribution[i][j]
        stringstream ss(str.substr(7, str.size() - 8));
        for (int j = 0; j < 256; j++)
        {
            getline(ss, temp, ',');
            distribution[i][j] = stoi(temp);
        }
        getline(myfile, str);
    }
}

// 打印差分分布表
void printDistribution()
{
    for (int i = 0; i < 64; i++)
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

int main()
{
    int sum = 0;
    bool valid = true;
    double probability = 1;

    uint32_t s_output = 0;
    uint8_t s_in_array[4] = {0};
    uint8_t s_out_array[4] = {0};

    printf("Read distribution table...\n");
    readDistribution();
    // printDistribution();

    printf("Build L reverse map...\n");
    buildLmap();

    printf("Start to search...\n");
    for (uint32_t i = 1; i < 1 << 24; i++)
    {
        probability = 1;
        valid = true;
        s_output = Lmap[i];
        s_in_array[0] = 0;
        // 取四个S盒的输入
        for (int j = 1; j < 4; j++)
        {
            s_in_array[j] = (i >> (8 * (3 - j))) & 0xff;
        }
        // 取四个S盒的输出
        for (int j = 0; j < 4; j++)
        {
            s_out_array[j] = (s_output >> (8 * (3 - j))) & 0xff;
        }
        // 验证给定输入输出是否在差分分布表中
        for (int j = 0; j < 4; j++)
        {
            if (distribution[s_in_array[j]][s_out_array[j]] == 0)
            {
                valid = false;
                break;
            }
            probability *= distribution[s_in_array[j]][s_out_array[j]] / 256.0;
        }
        if (valid)
        {
            sum++;
            printf("Find an alpha %#.8x with %e probability\n", i, probability);
        }
    }
    printf("Total find %d valid alpha\n", sum);
    system("pause");
    return 0;
}