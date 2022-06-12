class Round:
    @staticmethod
    def E_extend(text):
        position = [32, 1, 2, 3, 4, 5,
                    4, 5, 6, 7, 8, 9,
                    8, 9, 10, 11, 12, 13,
                    12, 13, 14, 15, 16, 17,
                    16, 17, 18, 19, 20, 21,
                    20, 21, 22, 23, 24, 25,
                    24, 25, 26, 27, 28, 29,
                    28, 29, 30, 31, 32, 1]
        extended = 0
        for i in range(48):
            extended += text >> (32 - position[i]) & 1
            extended = extended << 1
        return extended >> 1

    @staticmethod
    def P_permute(text):
        position = [16, 7, 20, 21,
                    29, 12, 28, 17,
                    1, 15, 23, 26,
                    5, 18, 31, 10,
                    2, 8, 24, 14,
                    32, 27, 3, 9,
                    19, 13, 30, 6,
                    22, 11, 4, 25]
        permuted = 0
        for i in range(32):
            permuted += text >> (32 - position[i]) & 1
            permuted = permuted << 1
        return permuted >> 1


if __name__ == '__main__':
    plaintext = input("Enter plaintext: ")
    print(hex(Round.E_extend(int(plaintext, base=16))))
    print(hex(Round.P_permute(int(plaintext, base=16))))
