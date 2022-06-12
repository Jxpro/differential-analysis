static char S[] = {16, 42, 28, 3, 26, 0, 31, 46,
                   27, 14, 49, 62, 37, 56, 23, 6,
                   40, 48, 53, 8, 20, 25, 33, 1,
                   2, 63, 15, 34, 55, 21, 39, 57,
                   54, 45, 47, 13, 7, 44, 61, 9,
                   60, 32, 22, 29, 52, 19, 12, 50,
                   5, 51, 11, 18, 59, 41, 36, 30,
                   17, 38, 10, 4, 58, 43, 35, 24};

static char Sr[] = {5, 23, 24, 3, 59, 48, 15, 36,
                    19, 39, 58, 50, 46, 35, 9, 26,
                    0, 56, 51, 45, 20, 29, 42, 14,
                    63, 21, 4, 8, 2, 43, 55, 6,
                    41, 22, 27, 62, 54, 12, 57, 30,
                    16, 53, 1, 61, 37, 33, 7, 34,
                    17, 10, 47, 49, 44, 18, 32, 28,
                    13, 31, 60, 52, 40, 38, 11, 25};

static char P[] = {24, 5, 15, 23, 14, 32,
                   19, 18, 26, 17, 6, 12,
                   34, 9, 8, 20, 28, 0,
                   2, 21, 29, 11, 33, 22,
                   30, 31, 1, 25, 3, 35,
                   16, 13, 27, 7, 10, 4};

static char Pr[] = {17, 26, 18, 28, 35, 1,
                    10, 33, 14, 13, 34, 21,
                    11, 31, 4, 2, 30, 9,
                    7, 6, 15, 19, 23, 3,
                    0, 27, 8, 32, 16, 20,
                    24, 25, 5, 22, 12, 29};

uint64_t S_substitution(uint64_t block, bool reverse)
{
    uint8_t parts[6];
    uint64_t result = 0;
    char *table = reverse == true ? Sr : S;
    for (int i = 0; i < 6; i++)
    {
        parts[i] = (block >> ((5 - i) * 6)) & 0x3f;
        result |= ((uint64_t)table[parts[i]]) << ((5 - i) * 6);
    }
    return result;
}

uint64_t P_permutation(uint64_t block, bool reverse)
{
    uint64_t result = 0;
    char *table = reverse == true ? Pr : P;
    for (int i = 0; i < 36; i++)
    {
        result |= ((block >> i) & 1) << table[i];
    }
    return result;
}

uint64_t easy1_encryt(uint64_t text, uint64_t *key, uint64_t *detail)
{
    uint64_t result = text;
    for (int i = 0; i < 3; i++)
    {
        result = S_substitution(result, false);
        result = P_permutation(result, false);
        result ^= key[i];
        if (detail != nullptr)
            detail[i] = result;
    }
    return result;
}

uint64_t easy1_decryt(uint64_t text, uint64_t *key)
{
    uint64_t result = text;
    for (int i = 0; i < 3; i++)
    {
        result ^= key[i];
        result = P_permutation(result, true);
        result = S_substitution(result, true);
    }
    return result;
}