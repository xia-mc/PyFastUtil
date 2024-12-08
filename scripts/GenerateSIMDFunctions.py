FUNCTIONS = ['Int', 'UnsignedInt', 'Long', 'UnsignedLong', 'LongLong', 'UnsignedLongLong', 'Short', 'UnsignedShort',
             'Float', 'Double', 'LongDouble', 'Char', 'UnsignedChar', 'WChar', 'Char16', 'Char32', 'Bool', 'Int8',
             'UInt8', 'Int16', 'UInt16', 'Int32', 'UInt32', 'Int64', 'UInt64', 'VoidPtr', 'IntPtr', 'FloatPtr',
             'PyObjectPtr']

for name in FUNCTIONS:
    print(f"def memcpy{name}(self, __addressFrom: Ptr, __addressTo: Ptr, __count: int) -> None: ...")
    print(f"def memcpy{name}Aligned(self, __addressFrom: Ptr, __addressTo: Ptr, __count: int) -> None: ...")
    print(f"def reverse{name}(self, __address: Ptr, __count: int) -> None: ...")
