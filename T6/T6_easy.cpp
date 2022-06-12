#include <bits/stdc++.h>
#include "easyTool.h"
using namespace std;
typedef struct item
{
    uint64_t first_s_in;
    uint64_t first_s_out;
    uint64_t first_p_out;
    uint64_t second_s_out;
    uint64_t second_p_out;
    double probability;
} item;

uint64_t key[] = {0x671234589, 0x985437621, 0x123456789};
uint64_t final_key;
// 差分分布表
int distribution[64][64];
// 每个输入差分对应的最大概率的输出差分，[0]差分，[1]差分概率
int distribution_max[64][2];
// 差分路径对应的S盒以及密钥索引
int diff_s_index1[3] = {1, 2, 3};
int diff_s_index2[3] = {0, 4, 5};
int diff_key_index1[18] = {0, 1, 2, 3, 8, 9, 11, 20, 21, 22, 25, 28, 29, 30, 31, 33, 34, 35};
int diff_key_index2[18] = {4, 5, 6, 7, 10, 12, 13, 14, 15, 16, 17, 18, 19, 23, 24, 26, 27, 32};
// 密钥计数器
int key_counter1[0x3ffff];
int key_counter2[0x3ffff];

// 计算S盒的差分概率
void calcDiff()
{
    int s_in1 = 0;
    int s_in2 = 0;
    int s_out1 = 0;
    int s_out2 = 0;
    int max_diff = 0;
    int max_times = 0;
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            s_in1 = j;
            s_in2 = j ^ i;
            s_out1 = S[s_in1];
            s_out2 = S[s_in2];
            distribution[i][s_out1 ^ s_out2]++;
        }
        max_diff = 0;
        max_times = 0;
        for (int j = 0; j < 64; j++)
        {
            if (distribution[i][j] > max_times)
            {
                max_times = distribution[i][j];
                max_diff = j;
            }
        }
        distribution_max[i][0] = max_diff;
        distribution_max[i][1] = max_times;
    }
}

// 打印差分分布表
void printDiff()
{
    for (int i = 0; i < 64; i++)
    {
        cout << i << endl;
        for (int j = 0; j < 64; j++)
        {
            cout << distribution[i][j] << " ";
            if (j % 16 == 15)
                cout << endl;
        }
        cout << endl;
    }
}

// 寻找差分路径
void findDiff()
{
    double prob;
    int s_single;
    int active_index;

    uint64_t input;
    uint64_t first_s_out;
    uint64_t first_p_out;
    uint64_t second_s_out;
    uint64_t output;
    vector<item> diff;

    for (int i = 0; i < 6; i++)
    {
        // 第一轮只让一个S盒活跃，其他S盒都不活跃，遍历64*64种输入输出情况
        for (int j = 1; j < 64; j++)
        {
            for (int k = 0; k < 64; k++)
            {
                input = (uint64_t)j << (i * 6);
                // 记录第一轮概率
                prob = (double)distribution[j][k] / 64.0;
                // 计算S盒输出，并移位到对应的位置
                first_s_out = (uint64_t)k << (i * 6);
                // 计算P置换的值
                first_p_out = P_permutation(first_s_out, false);
                if (prob > 0)
                {
                    // 按6个S盒位置，分割P置换后的值
                    // 第二轮取最大概率的差分
                    for (int k = 0; k < 6; k++)
                    {
                        // 计算概率
                        s_single = (first_p_out >> 6 * (5 - k)) & 0x3f;
                        prob *= distribution_max[s_single][1] / 64.0;
                        // 计算输出
                        second_s_out <<= 6;
                        second_s_out |= distribution_max[s_single][0];
                    }
                    output = P_permutation(second_s_out, false);
                    // 记录第一轮S盒的输入，输出，P置换后的值，概率
                    diff.push_back({input, first_s_out >> (i * 6), first_p_out,
                                    second_s_out, output, prob});
                }
            }
        }
    }

    // 将差分根据概率排序
    sort(diff.begin(), diff.end(), [](item a, item b)
         { return a.probability > b.probability; });

    //  打印概率前100的差分
    for (int i = 0; i < 100; i++)
    {
        input = diff[i].first_s_in;
        first_s_out = diff[i].first_s_out;
        first_p_out = diff[i].first_p_out;
        second_s_out = diff[i].second_s_out;
        output = diff[i].second_p_out;
        active_index = 0;
        // 将输入用S盒的值和位置进行表示
        while (input >> 6)
        {
            active_index++;
            input >>= 6;
        }
        cout << "first_active_index: " << 5 - active_index << endl;
        cout << "first_s_input:" << input << "\nfirst_s_out:" << first_s_out << endl;
        cout << "second_s_input:" << endl;
        // 计算第二轮的最大概率及差分
        for (int k = 0; k < 6; k++)
        {
            s_single = (first_p_out >> 6 * (5 - k)) & 0x3f;
            cout << s_single << " ";
        }
        cout << "\nsecond_s_out: " << endl;
        for (int k = 0; k < 6; k++)
        {
            s_single = (second_s_out >> 6 * (5 - k)) & 0x3f;
            cout << s_single << " ";
        }
        cout << "\nfinal_out: " << endl;
        for (int k = 0; k < 6; k++)
        {
            s_single = (output >> 6 * (5 - k)) & 0x3f;
            cout << s_single << " ";
        }
        // 输出最终概率
        cout << "\nprobability:" << diff[i].probability << "\n"
             << endl;
    }
}

// 随机密钥
void randomKey()
{
    for (int i = 0; i < 3; i++)
    {
        int r[3] = {rand(), rand(), rand()};
        key[i] = ((uint64_t)r[1] << 32 | (uint64_t)r[2] << 16 | (uint64_t)r[3]) & 0xfffffffff;
    }
    printf("The true 3th key is: %#.9llx\n", key[2]);
}

// 产生正确的明密文对，仅调试
void generateRightPair(uint64_t &in1, uint64_t &in2, uint64_t &out1, uint64_t &out2, uint64_t diff_in, uint64_t diff_out)
{
    uint64_t detail1[3] = {0};
    uint64_t detail2[3] = {0};
    do
    {
        int r[3] = {rand(), rand(), rand()};
        in1 = ((uint64_t)r[0] << 32 | (uint64_t)r[1] << 16 | (uint64_t)r[2]) & 0xfffffffff;
        in2 = in1 ^ diff_in;
        out1 = easy1_encryt(in1, key, detail1);
        out2 = easy1_encryt(in2, key, detail2);
    } while ((detail1[1] ^ detail2[1]) != diff_out);
}

// 产生明密文对
void generatePair(uint64_t &in1, uint64_t &in2, uint64_t &out1, uint64_t &out2, uint64_t diff_in)
{
    int r[3] = {rand(), rand(), rand()};
    in1 = ((uint64_t)r[0] << 32 | (uint64_t)r[1] << 16 | (uint64_t)r[2]) & 0xfffffffff;
    in2 = in1 ^ diff_in;
    out1 = easy1_encryt(in1, key, nullptr);
    out2 = easy1_encryt(in2, key, nullptr);
}

// 猜测密钥
void guessKey(int mode, int pair_number)
{
    int *diff_key_index = mode == 0 ? diff_key_index1 : diff_key_index2;
    int *key_conuter = mode == 0 ? key_counter1 : key_counter2;
    uint64_t diff_in = mode == 0 ? 0xf00000000 : 0xe00000;
    uint64_t diff_out = mode == 0 ? 0x1804000 : 0x200000804;
    uint64_t diff_mask = mode == 0 ? 0x3ffff000 : 0xfc0000fff;

    uint64_t guess_key;
    uint64_t plaintext1;
    uint64_t plaintext2;
    uint64_t ciphertext1;
    uint64_t ciphertext2;
    uint64_t reverse_cipher1;
    uint64_t reverse_cipher2;
    uint64_t reverse_diff;

    for (int i = 0; i < pair_number; i++)
    {
        // 生成密文对
        generatePair(plaintext1, plaintext2, ciphertext1, ciphertext2, diff_in);
        // generateRightPair(plaintext1, plaintext2, ciphertext1, ciphertext2, diff_in, diff_out);
        for (uint32_t j = 0; j < 1 << 18; j++)
        {
            // 生成猜测密钥
            guess_key = 0;
            for (int i = 0; i < 18; i++)
                guess_key |= (uint64_t)(j >> i & 1) << diff_key_index[i];

            // 用猜测密钥进行解密并计算差分
            reverse_cipher1 = S_substitution(P_permutation(ciphertext1 ^ guess_key, true), true);
            reverse_cipher2 = S_substitution(P_permutation(ciphertext2 ^ guess_key, true), true);

            // 如果正确，则计数器加1
            reverse_diff = reverse_cipher1 ^ reverse_cipher2;
            if ((reverse_diff & diff_mask) == diff_out)
                key_conuter[j]++;
        }
    }

    // 找出最大可能的密钥
    int max_times = 0;
    uint64_t final_part_key = 0;
    for (uint32_t j = 0; j < 1 << 18; j++)
    {
        if (key_conuter[j] > max_times)
        {
            max_times = key_conuter[j];
            final_part_key = j;
        }
    }
    // 将其与最终密钥合并
    guess_key = 0;
    for (int i = 0; i < 18; i++)
        guess_key |= (final_part_key >> i & 1) << diff_key_index[i];
    final_key |= guess_key;
    cout << "Guess partial key 0x" << hex << guess_key << " with " << max_times << " times" << endl;
}

// 猜测多次密钥，测试准确性
void guessRate(int times, int pair_number)
{
    double correct_times = 0;
    for (int i = 0; i < times; i++)
    {
        // 将猜测密钥清零
        memset(key_counter1, 0, sizeof(key_counter1));
        memset(key_counter2, 0, sizeof(key_counter2));
        final_key = 0;

        randomKey();

        guessKey(0, pair_number);
        guessKey(1, pair_number);

        printf("The final guess key is: %#.9llx\n", final_key);
        if (final_key == key[2])
        {
            printf("The guess key is correct!\n\n");
            correct_times++;
        }
        else
            printf("The guess key is wrong!\n\n");
    }
    printf("Guess key correct rate: %.2f%%\n", correct_times / times * 100);
}

int main()
{
    srand(time(NULL));

    calcDiff();
    // printDiff();
    // findDiff();

    // 差分路径1 ( 概率：6*8 / (64*64) = 0.0117 ) ：
    // (输入差分)60 0 0 0 0 0 ->(第一轮S盒输出)32 0 0 0 0 0 ->
    // (第一轮P置换/第二轮S盒输入) 0 0 0 0 0 16 ->(第二轮S盒输出)0 0 0 0 0 25 ->
    // (输出差分) 0 1 32 4 0 0
    // 可破解 2 3 4 位置的S盒

    // 差分路径2 ( 概率：6*8 / (64*64) = 0.117 ) ：
    // (输入差分)0 0 56 0 0 0 ->(第一轮S盒输出)0 0 32 0 0 0 ->
    // (第一轮P置换/第二轮S盒输入) 0 0 16 0 0 0 ->(第二轮S盒输出)0 0 25 0 0 0 ->
    // (输出差分) 8 0 0 0 32 4
    // 可破解 1 5 6 位置的S盒

    // 信噪比S/N = 2^18 * 0.0117 / (1*1) = 3067 ?
    // m = c * 1/p = 3 * 85.47 = 256

    // 准确率 40~60%
    guessRate(10, 256);

    system("pause");
    return 0;
}