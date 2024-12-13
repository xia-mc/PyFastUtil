import platform
import unittest

import pytest
from keystone import Ks, KS_ARCH_X86, KS_MODE_64

from pyfastutil.unsafe import ASM, Unsafe
from tests.benchmark import benchmark_ASM


@pytest.mark.skipif(platform.system() != "Windows", reason="ASM only support Windows now.")
@unittest.skipIf(platform.system() != "Windows", "ASM only support Windows now.")
class TestASM(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """
        Set up resources for the test class. This method runs once before all tests.
        """
        # Initialize Keystone assembler for x86-64
        cls.ks = Ks(KS_ARCH_X86, KS_MODE_64)

    def test_run(self):
        """
        Test ASM.run with valid machine code.
        """
        # Assemble a simple instruction: `ret` (return from function)
        # This is the simplest valid machine code that does nothing.
        encoding, _ = self.ks.asm("ret")

        # Execute the machine code
        with ASM() as asm:
            asm.run(bytes(encoding))

    def test_runFast(self):
        """
        Test ASM.runFast with valid machine code.
        """
        # Assemble a simple instruction: `ret` (return from function)
        encoding, _ = self.ks.asm("ret")

        # Execute the machine code
        with ASM() as asm:
            asm.runFast(bytes(encoding))

    def test_benchmark(self):
        benchmark_ASM.main()

    def test_makeAndFreeFunction(self):
        encoding, _ = self.ks.asm("mov eax, 42; ret")

        with ASM() as asm, Unsafe() as unsafe:
            asmFunc = asm.makeFunction(bytes(encoding))
            self.assertEqual(unsafe.callInt(asmFunc), 42)
            asm.freeFunction(asmFunc)

    def test_makeAndFreeFunctionFast(self):
        encoding, _ = self.ks.asm("mov eax, 42; ret")

        with ASM() as asm, Unsafe() as unsafe:
            asmFunc = asm.makeFunctionFast(bytes(encoding))
            self.assertEqual(unsafe.callInt(asmFunc), 42)
            asm.freeFunctionFast(asmFunc)


if __name__ == "__main__":
    unittest.main()
