from typing import TypeVar
from enum import IntEnum


class MM_PERM_ENUM(IntEnum):
    AAAA = 0x00
    AAAB = 0x01
    AAAC = 0x02
    AAAD = 0x03
    AABA = 0x04
    AABB = 0x05
    AABC = 0x06
    AABD = 0x07
    AACA = 0x08
    AACB = 0x09
    AACC = 0x0A
    AACD = 0x0B
    AADA = 0x0C
    AADB = 0x0D
    AADC = 0x0E
    AADD = 0x0F
    ABAA = 0x10
    ABAB = 0x11
    ABAC = 0x12
    ABAD = 0x13
    ABBA = 0x14
    ABBB = 0x15
    ABBC = 0x16
    ABBD = 0x17
    ABCA = 0x18
    ABCB = 0x19
    ABCC = 0x1A
    ABCD = 0x1B
    ABDA = 0x1C
    ABDB = 0x1D
    ABDC = 0x1E
    ABDD = 0x1F
    ACAA = 0x20
    ACAB = 0x21
    ACAC = 0x22
    ACAD = 0x23
    ACBA = 0x24
    ACBB = 0x25
    ACBC = 0x26
    ACBD = 0x27
    ACCA = 0x28
    ACCB = 0x29
    ACCC = 0x2A
    ACCD = 0x2B
    ACDA = 0x2C
    ACDB = 0x2D
    ACDC = 0x2E
    ACDD = 0x2F
    ADAA = 0x30
    ADAB = 0x31
    ADAC = 0x32
    ADAD = 0x33
    ADBA = 0x34
    ADBB = 0x35
    ADBC = 0x36
    ADBD = 0x37
    ADCA = 0x38
    ADCB = 0x39
    ADCC = 0x3A
    ADCD = 0x3B
    ADDA = 0x3C
    ADDB = 0x3D
    ADDC = 0x3E
    ADDD = 0x3F
    BAAA = 0x40
    BAAB = 0x41
    BAAC = 0x42
    BAAD = 0x43
    BABA = 0x44
    BABB = 0x45
    BABC = 0x46
    BABD = 0x47
    BACA = 0x48
    BACB = 0x49
    BACC = 0x4A
    BACD = 0x4B
    BADA = 0x4C
    BADB = 0x4D
    BADC = 0x4E
    BADD = 0x4F
    BBAA = 0x50
    BBAB = 0x51
    BBAC = 0x52
    BBAD = 0x53
    BBBA = 0x54
    BBBB = 0x55
    BBBC = 0x56
    BBBD = 0x57
    BBCA = 0x58
    BBCB = 0x59
    BBCC = 0x5A
    BBCD = 0x5B
    BBDA = 0x5C
    BBDB = 0x5D
    BBDC = 0x5E
    BBDD = 0x5F
    BCAA = 0x60
    BCAB = 0x61
    BCAC = 0x62
    BCAD = 0x63
    BCBA = 0x64
    BCBB = 0x65
    BCBC = 0x66
    BCBD = 0x67
    BCCA = 0x68
    BCCB = 0x69
    BCCC = 0x6A
    BCCD = 0x6B
    BCDA = 0x6C
    BCDB = 0x6D
    BCDC = 0x6E
    BCDD = 0x6F
    BDAA = 0x70
    BDAB = 0x71
    BDAC = 0x72
    BDAD = 0x73
    BDBA = 0x74
    BDBB = 0x75
    BDBC = 0x76
    BDBD = 0x77
    BDCA = 0x78
    BDCB = 0x79
    BDCC = 0x7A
    BDCD = 0x7B
    BDDA = 0x7C
    BDDB = 0x7D
    BDDC = 0x7E
    BDDD = 0x7F
    CAAA = 0x80
    CAAB = 0x81
    CAAC = 0x82
    CAAD = 0x83
    CABA = 0x84
    CABB = 0x85
    CABC = 0x86
    CABD = 0x87
    CACA = 0x88
    CACB = 0x89
    CACC = 0x8A
    CACD = 0x8B
    CADA = 0x8C
    CADB = 0x8D
    CADC = 0x8E
    CADD = 0x8F
    CBAA = 0x90
    CBAB = 0x91
    CBAC = 0x92
    CBAD = 0x93
    CBBA = 0x94
    CBBB = 0x95
    CBBC = 0x96
    CBBD = 0x97
    CBCA = 0x98
    CBCB = 0x99
    CBCC = 0x9A
    CBCD = 0x9B
    CBDA = 0x9C
    CBDB = 0x9D
    CBDC = 0x9E
    CBDD = 0x9F
    CCAA = 0xA0
    CCAB = 0xA1
    CCAC = 0xA2
    CCAD = 0xA3
    CCBA = 0xA4
    CCBB = 0xA5
    CCBC = 0xA6
    CCBD = 0xA7
    CCCA = 0xA8
    CCCB = 0xA9
    CCCC = 0xAA
    CCCD = 0xAB
    CCDA = 0xAC
    CCDB = 0xAD
    CCDC = 0xAE
    CCDD = 0xAF
    CDAA = 0xB0
    CDAB = 0xB1
    CDAC = 0xB2
    CDAD = 0xB3
    CDBA = 0xB4
    CDBB = 0xB5
    CDBC = 0xB6
    CDBD = 0xB7
    CDCA = 0xB8
    CDCB = 0xB9
    CDCC = 0xBA
    CDCD = 0xBB
    CDDA = 0xBC
    CDDB = 0xBD
    CDDC = 0xBE
    CDDD = 0xBF
    DAAA = 0xC0
    DAAB = 0xC1
    DAAC = 0xC2
    DAAD = 0xC3
    DABA = 0xC4
    DABB = 0xC5
    DABC = 0xC6
    DABD = 0xC7
    DACA = 0xC8
    DACB = 0xC9
    DACC = 0xCA
    DACD = 0xCB
    DADA = 0xCC
    DADB = 0xCD
    DADC = 0xCE
    DADD = 0xCF
    DBAA = 0xD0
    DBAB = 0xD1
    DBAC = 0xD2
    DBAD = 0xD3
    DBBA = 0xD4
    DBBB = 0xD5
    DBBC = 0xD6
    DBBD = 0xD7
    DBCA = 0xD8
    DBCB = 0xD9
    DBCC = 0xDA
    DBCD = 0xDB
    DBDA = 0xDC
    DBDB = 0xDD
    DBDC = 0xDE
    DBDD = 0xDF
    DCAA = 0xE0
    DCAB = 0xE1
    DCAC = 0xE2
    DCAD = 0xE3
    DCBA = 0xE4
    DCBB = 0xE5
    DCBC = 0xE6
    DCBD = 0xE7
    DCCA = 0xE8
    DCCB = 0xE9
    DCCC = 0xEA
    DCCD = 0xEB
    DCDA = 0xEC
    DCDB = 0xED
    DCDC = 0xEE
    DCDD = 0xEF
    DDAA = 0xF0
    DDAB = 0xF1
    DDAC = 0xF2
    DDAD = 0xF3
    DDBA = 0xF4
    DDBB = 0xF5
    DDBC = 0xF6
    DDBD = 0xF7
    DDCA = 0xF8
    DDCB = 0xF9
    DDCC = 0xFA
    DDCD = 0xFB
    DDDA = 0xFC
    DDDB = 0xFD
    DDDC = 0xFE
    DDDD = 0xFF


class MM_MANTISSA_NORM_ENUM(IntEnum):
    NORM_1_2 = 0  # interval[1, 2)
    NORM_p5_2 = 1  # interval[0.5, 2)
    NORM_p5_1 = 2  # interval[0.5, 1)
    NORM_p75_1p5 = 3  # interval[0.75, 1.5)


class MM_MANTISSA_SIGN_ENUM(IntEnum):
    src = 0  # sign = sign(SRC)
    zero = 1  # sign = 0
    nan = 2  # DEST = NaN if sign(SRC) = 1