#include <bits/stdc++.h>
#include "desTools.h"
using namespace std;

char distribution[8][64][16];

int subkey_counter[8][64] = {0};
char guess_roundkey[8] = {0};
uint64_t guess_key = 0;

uint64_t true_key = 0x1234567890abcdef;
char true_roundkey[8] = {0};
// 密钥初始置换：1111 0000 1100 1100 1010 1010 0001 1010 0101 1100 0110 1110 1000 1111
// 第六轮轮密钥 001000(8) 110010(50) 010100(20) 011110(30) 010111(23) 000110(6) 111100(60) 100101(37)

uint64_t plain_diff1 = 0x4008000004000000;
uint64_t plain_diff2 = 0x0020000800000400;
char diff_index1[5] = {2, 5, 6, 7, 8};
char diff_index2[5] = {1, 2, 4, 5, 6};
char search_index[14] = {4, 8, 9, 12, 18, 19, 22, 23, 25, 26, 35, 38, 43, 54};

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

// 随机产生密钥并打印正确密钥
void randomKey()
{
    int r[4] = {rand(), rand(), rand(), rand()};
    true_key = (uint64_t)r[0] << 48 | (uint64_t)r[1] << 32 | (uint64_t)r[2] << 16 | (uint64_t)r[3];
    uint64_t *temp_sub_key = new uint64_t[6];
    uint64_t lastkey = key_generate(6, true_key, temp_sub_key);
    for (int i = 42; i >= 0; i -= 6)
    {
        true_roundkey[7 - i / 6] = (lastkey >> i) & 0x3f;
    }
    printf("The true key is: %#.16llx\n", true_key);
    printf("The 6th sound subkey is: ");
    for (int i = 0; i < 8; i++)
    {
        printf("%d ", true_roundkey[i]);
    }
    printf("\n");
}

// 产生正确的明密文对，仅调试用
void generateRightPair(uint64_t &in1, uint64_t &in2, uint64_t &out1, uint64_t &out2, int mode)
{
    // 记录中间值
    uint64_t round_values1[6] = {0};
    uint64_t round_values2[6] = {0};
    // 循环知道产生满足条件的明密文对
    do
    {
        int r[4] = {rand(), rand(), rand(), rand()};
        in1 = (uint64_t)r[0] << 48 | (uint64_t)r[1] << 32 | (uint64_t)r[2] << 16 | (uint64_t)r[3];
        in2 = in1 ^ (mode == 0 ? plain_diff1 : plain_diff2);
        out1 = des(6, round_values1, in1, true_key, 'e');
        out2 = des(6, round_values2, in2, true_key, 'e');
    } while ((round_values1[2] ^ round_values2[2]) != (mode == 0 ? 0x0400000040080000 : 0x0000040000200008));
    // 打印每轮的中间值差分
    for (int i = 0; i < 6; i++)
    {
        printf("%d round diff: %.16llx\n", i + 1, round_values1[i] ^ round_values2[i]);
    }
    // 打印 l'
    printf("F' = %.8x\n", (uint32_t)((out1 ^ out2) >> 32));
    // 打印 l' xor 0x04000000 或 0x00000400
    printf("F' xor 0x... = %.8x\n", (uint32_t)((out1 ^ out2) >> 32) ^ (mode == 0 ? 0x04000000 : 0x00000400));
    // 打印 l' xor 0x04000000 或 0x00000400 的P逆置换
    printf("-P(F' xor 0x...) = %.8x\n", P_reverse((out1 ^ out2) >> 32 ^ (mode == 0 ? 0x04000000 : 0x00000400)));
}

// 产生明密文对
void generatePair(uint64_t &in1, uint64_t &in2, uint64_t &out1, uint64_t &out2, int mode)
{
    int r[4] = {rand(), rand(), rand(), rand()};
    in1 = (uint64_t)r[0] << 48 | (uint64_t)r[1] << 32 | (uint64_t)r[2] << 16 | (uint64_t)r[3];
    in2 = in1 ^ (mode == 0 ? plain_diff1 : plain_diff2);
    out1 = des(6, nullptr, in1, true_key, 'e');
    out2 = des(6, nullptr, in2, true_key, 'e');
}

// 计算指定S盒位置的差分是否相等
bool validateKey(uint32_t diff1, uint32_t diff2, int index)
{
    char v1 = (diff1 >> (8 - index) * 4) & 0xf;
    char v2 = (diff2 >> (8 - index) * 4) & 0xf;
    return v1 == v2;
}

// 过滤错误对
bool validatePair(uint64_t in, uint32_t out, int mode)
{
    char *index = mode == 0 ? diff_index1 : diff_index2;
    for (int i = 0; i < 5; i++)
    {
        int s_in = (in >> (8 - index[i]) * 6) & 0x3f;
        int s_out = (out >> (8 - index[i]) * 4) & 0xf;
        if (distribution[index[i] - 1][s_in][s_out] == 0)
            return false;
    }
    return true;
}

// 通过两个差分特征猜测密钥
void guessKey(int mode, int pair_number)
{
    char *diff_index = mode == 0 ? diff_index1 : diff_index2;

    uint64_t plaintext1;
    uint64_t plaintext2;
    uint64_t ciphertext1;
    uint64_t ciphertext2;
    uint64_t cipher_diff;
    uint64_t s_in_diff;
    uint64_t guess_key;

    uint32_t cipher_R1;
    uint32_t cipher_R2;
    uint32_t cipher_diff_R;
    uint32_t cipher_diff_L;

    uint32_t s_out1;
    uint32_t s_out2;
    uint32_t s_out_diff;

    for (int i = 0; i < pair_number; i++)
    {
        // 随机产生明文
        generatePair(plaintext1, plaintext2, ciphertext1, ciphertext2, mode);
        cipher_R1 = ciphertext1 & 0xffffffff;
        cipher_R2 = ciphertext2 & 0xffffffff;

        // 计算密文差分
        cipher_diff = ciphertext1 ^ ciphertext2;
        cipher_diff_R = cipher_diff & 0xffffffff;
        cipher_diff_L = cipher_diff >> 32 & 0xffffffff;
        s_in_diff = E_extend(cipher_diff_R);
        s_out_diff = P_reverse(cipher_diff_L ^ (mode == 0 ? 0x04000000 : 0x00000400));

        // 过滤密文
        if (!validatePair(s_in_diff, s_out_diff, mode))
            continue;
        for (int j = 0; j < 5; j++)
        {
            for (int k = 0; k < 64; k++)
            {
                // 猜测密钥
                guess_key = (uint64_t)k << (8 - diff_index[j]) * 6;
                s_out1 = S_substitution(E_extend(cipher_R1) ^ guess_key);
                s_out2 = S_substitution(E_extend(cipher_R2) ^ guess_key);

                // 如果从右往左通过密钥计算得到的密文差分，与从左往右从密文推导得到的差分相等，则计数器加1
                if (validateKey(s_out1 ^ s_out2, s_out_diff, diff_index[j]))
                    subkey_counter[diff_index[j] - 1][k]++;
            }
        }
    }
}

// 打印猜测密钥
void printGuessKey()
{
    for (int i = 0; i < 8; i++)
    {
        if (i == 2)
        {
            printf("S3k is unkown\n");
            continue;
        }
        int times = 0;
        int gkey = 0;
        for (int j = 0; j < 64; j++)
        {
            if (subkey_counter[i][j] > times)
            {
                times = subkey_counter[i][j];
                gkey = j;
            }
        }
        guess_roundkey[i] = gkey;
        printf("S%dk = %2d with %2d times\n", i + 1, gkey, times);
    }
}

// 验证猜测的密钥
bool verifyGuessKey()
{
    for (int i = 0; i < 8; i++)
    {
        if (i != 2 && guess_roundkey[i] != true_roundkey[i])
            return false;
    }
    return true;
}

// 猜测多次密钥，测试准确性
void guessRate(int times, int pair_number)
{
    double correct_times = 0;
    for (int i = 0; i < times; i++)
    {
        // 将猜测密钥清零
        memset(subkey_counter, 0, sizeof(subkey_counter));
        memset(guess_roundkey, 0, sizeof(guess_roundkey));

        randomKey();

        guessKey(0, pair_number);
        guessKey(1, pair_number);

        printf("The guess result is:\n");
        printGuessKey();
        if (verifyGuessKey())
        {
            printf("Guess key is correct!!!\n");
            correct_times++;
        }
        else
        {
            printf("Guess key is wrong!!!\n");
        }
        printf("\n");
    }
    printf("Guess key correct rate: %.2f%%\n", correct_times / times * 100);
}

// 将已推出的轮密钥组合，并进行PC2逆置换
uint64_t combineGuessRoundkey()
{
    uint64_t guess_key = 0;
    for (int i = 0; i < 8; i++)
    {
        guess_key <<= 6;
        guess_key |= guess_roundkey[i];
    }
    return PC2_reverse(guess_key);
}

// 由轮密钥逆推原始密钥
uint64_t getOriginKey(uint64_t roundkey)
{
    uint32_t C = (roundkey >> 28) & 0xfffffff;
    uint32_t D = roundkey & 0xfffffff;

    C = (C >> 10) | (0xffc0000 & (C << 18));
    D = (D >> 10) | (0xffc0000 & (D << 18));

    roundkey = (((uint64_t)C) << 28) | (uint64_t)D;
    return PC1_reverse(roundkey);
}

// 穷搜剩下比特
bool exhaustiveSearch()
{
    bool found = false;
    uint64_t plaintext = 0;
    uint64_t search_key = 0;
    uint64_t finnal_key = 0;
    uint64_t fragment_key = combineGuessRoundkey();

    for (uint64_t i = 0; i < 1 << 14; i++)
    {
        // 设置剩下的比特位为0或1
        search_key = 0;
        for (int j = 0; j < 14; j++)
        {
            search_key |= (i >> (13 - j) & 1) << (56 - search_index[j]);
        }

        // 得到原始密钥
        finnal_key = getOriginKey(fragment_key | search_key);

        // 随机产生明文
        int r[4] = {rand(), rand(), rand(), rand()};
        plaintext = (uint64_t)r[0] << 48 | (uint64_t)r[1] << 32 | (uint64_t)r[2] << 16 | (uint64_t)r[3];

        // 用两个密钥分别加密，然后比对明文
        if (des(6, nullptr, plaintext, finnal_key, 'e') == des(6, nullptr, plaintext, true_key, 'e'))
        {
            // 对于给定明文，不确定是否会有多个密钥可解
            // 所以不提前return，而是将所有情况搜索完毕
            printf("Find correct key regardless of the parity bits: %llx\n", finnal_key);
            // 如果找到多个密钥，记录最后一个密钥
            // 当然应该只能找到一个密钥，找到多个只是假设情况，如果存在，则再进行调整
            guess_key = finnal_key;
            // 加个标志位记录判断是否找到，用于输出提示
            found = true;
        }
    }
    if (!found)
        printf("No correct key found\n");

    return found;
}

// 猜测多次密钥，测试最终准确性
void finnalRate(int times, int pair_number)
{
    double correct_times = 0;
    for (int i = 0; i < times; i++)
    {
        // 将猜测密钥清零
        memset(subkey_counter, 0, sizeof(subkey_counter));
        memset(guess_roundkey, 0, sizeof(guess_roundkey));
        guess_key = 0;

        randomKey();

        guessKey(0, pair_number);
        guessKey(1, pair_number);

        printGuessKey();
        if (exhaustiveSearch())
            correct_times++;
    }
    printf("Guess key correct rate: %.2f%%\n", correct_times / times * 100);
}

int main()
{
    // 寻找三轮特征：(0x40080000,0x04000000)->...->(0x04000000,0x40080000) 第 * 2 * * 5 6 7 8 个S盒差分为0
    // 寻找三轮特征：(0x00200008,0x00000400)->...->(0x00000400,0x00200008) 第 1 2 * 4 5 6 * * 个S盒差分为0
    // 综上，通过两个差分特征，我们可以找到7*6比特密钥，剩下14比特需要穷搜
    // 计算信噪比：S/N = (2^8 * 1/16) / (0.8^5 * 4) = 12
    // 故选择明文对：m = c * 1/p = 20 * 16 = 320

    srand(time(NULL));
    readDistribution();

    // 测试密钥猜测的准确性
    // 通过计算(猜测1000次)，发现：
    // pair_number=320，即c=20时，准确率99.8~100%
    // pair_number=160，即c=10时，准确率97~98%
    // guessRate(1000, 160);

    // 测试猜测加穷搜后最终的准确性，理论上与密钥猜测的准确性一致
    finnalRate(1000, 320);

    system("pause");
    return 0;
}