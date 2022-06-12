from abc import abstractmethod, ABCMeta

from matplotlib import pyplot as plt


class BaseDiff(metaclass=ABCMeta):
    @abstractmethod
    def __init__(self):
        self.Sbox = []

    def sub_bytes(self, input_bits):
        """计算输入值通过 AES 或 SM4 S-box 代换后的输出值

        :param input_bits: 输入的值
        :return: S-box 代换后的值
        """
        row = (input_bits & (15 << 4)) >> 4
        column = input_bits & 15
        return self.Sbox[row][column]

    def distribution(self):
        """计算 AES 或 SM4 S-box 的差分分布表"""
        result = [[0] * 256 for _ in range(256)]
        for input_xor in range(256):
            for input_pair1 in range(256):
                input_pair2 = input_pair1 ^ input_xor
                output_pair1 = self.sub_bytes(input_pair1)
                output_pair2 = self.sub_bytes(input_pair2)
                output_xor = output_pair1 ^ output_pair2
                result[input_xor][output_xor] += 1
        return result

    def max_diff_value(self):
        """找到 AES 或 SM4 S-box 的最大值及其输入和输出的差分值

        :return 一个元祖，依次包含最大值，输入和输出的差分值
        """
        distribution = self.distribution()
        distribution.pop(0)
        plain = [i for j in distribution for i in j]
        m_value = max(plain)
        row = plain.index(m_value) // 256
        col = plain.index(m_value) % 256
        return m_value, row + 1, col

    def possible_values(self, input_xor, output_xor):
        """计算给定输入和输出差分，给定 S-box 下，可能的输入值

        :param input_xor: 输入差分值
        :param output_xor: 输出差分值
        :return: 可能的输入值
        """
        possible_values = []
        for input_pair1 in range(256):
            input_pair2 = input_pair1 ^ input_xor
            output_pair1 = self.sub_bytes(input_pair1)
            output_pair2 = self.sub_bytes(input_pair2)
            xor = output_pair1 ^ output_pair2
            if xor == output_xor:
                possible_values.append(input_pair1)
        return possible_values

    def save_distribution_to_file(self, path):
        """将差分分布表保存到指定路径"""
        with open(path, 'w') as f:
            for key, value in enumerate(self.distribution()):
                f.write("%#04x" % key + ": " + value.__str__() + '\n\n')

    def print_distribution(self):
        """打印差分分布表"""
        for i in self.distribution():
            print(i)

    def show_distribution(self):
        """显示差分分布图"""
        y = range(0, 256)
        for x in self.distribution():
            plt.plot(y, x)
            plt.show()

    def statistics(self):
        """统计列表中每个概率出现的次数"""
        maps = [{} for _ in range(256)]
        for i, v in enumerate(self.distribution()):
            for j in v:
                # 如果j在字典的key中，则将value加1
                if j in maps[i]:
                    maps[i][j] += 1
                # 否则，将j作为key，value为1
                else:
                    maps[i][j] = 1
            print(maps[i])
        return maps
