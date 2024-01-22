import os
import math
import logging
import binascii

class Fragmentation:
    def __init__(self, Input, Size, Redundancy):
        self.__Logger = logging.getLogger("ChirpStack-Fragmentation")
        self.__UncodedFragments = list()
        self.__CodedFragments = list()

        with open(Input, "rb") as f:
            while bytes_str := f.read(Size):
                self.__UncodedFragments.extend([binascii.hexlify(bytes_str).decode()])

        self.__NumberOfFragments = len(self.__UncodedFragments)

        # Apply padding
        self.__Padding = ((Size * 2) - len(self.__UncodedFragments[-1]))
        self.__UncodedFragments[-1] += '0' * self.__Padding

        self.__Logger.info(("Input file: {} | Fragment size: {} bytes | Uncoded fragments: {} | Redundancy fragments: {}").format(Input, Size, self.__NumberOfFragments, Redundancy))

        # Generate a coded array based on uncoded content
        self.__Logger.debug("Matrix:")
        for y in range(self.__NumberOfFragments):
            s = '0' * (2 * Size)

            # Line y of M x M matrix
            Line = self.__MatrixLine(y, self.__NumberOfFragments)
            self.__Logger.debug("{:03}: {} - {:03}".format(y + 1, Line, Line.count(1)))
            for x in range(self.__NumberOfFragments):
                # If bit x is set to 1 then xor the corresponding fragment
                if(Line[x] == 1):
                    s = "{:X}".format(int(s, 16) ^ int(self.__UncodedFragments[x], 16))

            # Prevent odd-length string
            s = '0' * ((Size * 2) - len(s)) + s

            self.__CodedFragments.extend([s])

    def __PRBS23(self, Start):
        b0 = Start & 1
        b1 = int((Start & 32) / 32)
        return (Start >> 1) | ((b0 ^ b1) << 22)

    def __MatrixLine(self, N, M):
        nb_coeff = 0
        line = [0] * M

        # if M is a power of 2
        if((M & (M - 1) == 0) and (M != 0)):
            pow2 = 1
        else:
            pow2 = 0

        # Initialize the seed differently for each line
        x = 1 + (1001 * (N + 1))

        # will generate a line with M / 2 bits set to 1 (50 %)
        while(nb_coeff < math.floor(M / 2)):
            r = math.pow(2, 16)

            # This can happen if m=1, in that case just try again with a different random number
            while(r >= M):
                x = self.__PRBS23(x)

                # Bit number r of the current line will be switched to 1
                r = x % (M + pow2)

            # Set to 1 the column which was randomly selected
            line[r] = 1
            nb_coeff += 1

        return line

    def Padding(self):
        return self.__Padding

    def GetFragments(self):
        Fragments = list()

        for frag in self.__UncodedFragments:
            Fragments.append(frag.upper())

        for frag in self.__CodedFragments:
            Fragments.append(frag.upper())

        return Fragments

    def WriteToFile(self, OutputPath):
        with open(OutputPath, "wb") as f:
            self.__Logger.debug("Uncoded fragments:")
            for num, frag in enumerate(self.__UncodedFragments, start = 1):
                self.__Logger.debug("{:03}: {}".format(num, frag.upper()))
                f.write(binascii.unhexlify(frag.encode()))

            self.__Logger.debug("Coded fragments:")
            for num, frag in enumerate(self.__CodedFragments, start = 1):
                self.__Logger.debug("{:03}: {}".format(num, frag.upper()))
                f.write(binascii.unhexlify(frag.encode()))

        self.__Logger.info("Output file: {} | File size: {} bytes".format(OutputPath, os.path.getsize(OutputPath)))