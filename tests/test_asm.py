import platform
import unittest

from keystone import Ks, KS_ARCH_X86, KS_MODE_64

from pyfastutil.unsafe import ASM


@unittest.skipUnless(platform.system() == "Windows", "ASM only support Windows now.")
class TestASM(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """
        Set up resources for the test class. This method runs once before all tests.
        """
        # Initialize Keystone assembler for x86-64
        cls.ks = Ks(KS_ARCH_X86, KS_MODE_64)

    def test_run_valid_code(self):
        """
        Test ASM.run with valid machine code.
        """
        # Assemble a simple instruction: `ret` (return from function)
        # This is the simplest valid machine code that does nothing.
        encoding, _ = self.ks.asm("ret")

        # Execute the machine code
        with ASM() as asm:
            try:
                asm.run(bytes(encoding))
            except Exception as e:
                self.fail(f"ASM.run raised an exception with valid code: {e}")

    def test_runFast_valid_code(self):
        """
        Test ASM.runFast with valid machine code.
        """
        # Assemble a simple instruction: `ret` (return from function)
        encoding, _ = self.ks.asm("ret")

        # Execute the machine code
        with ASM() as asm:
            try:
                asm.runFast(bytes(encoding))
            except Exception as e:
                self.fail(f"ASM.runFast raised an exception with valid code: {e}")

    def test_with_context(self):
        """
        Test that ASM can be used as a context manager.
        """
        try:
            with ASM() as asm:
                self.assertIsInstance(asm, ASM)
        except Exception as e:
            self.fail(f"ASM context manager raised an exception: {e}")

    def test_without_context(self):
        """
        Test that ASM can be used without a context manager.
        """
        asm = ASM()
        self.assertIsInstance(asm, ASM)

        # Assemble a simple instruction: `ret`
        encoding, _ = self.ks.asm("ret")

        try:
            asm.run(bytes(encoding))
        except Exception as e:
            self.fail(f"ASM.run raised an exception when used without context: {e}")

    def test_run_with_aligned_code(self):
        """
        Test ASM.run with aligned machine code.
        """
        # Assemble a simple instruction: `mov eax, 42; ret`
        # This moves the value 42 into the EAX register and then returns.
        encoding, _ = self.ks.asm("mov eax, 42; ret")

        with ASM() as asm:
            try:
                asm.run(bytes(encoding))
            except Exception as e:
                self.fail(f"ASM.run raised an exception with aligned code: {e}")

    def test_runFast_with_aligned_code(self):
        """
        Test ASM.runFast with aligned machine code.
        """
        # Assemble a simple instruction: `mov eax, 42; ret`
        encoding, _ = self.ks.asm("mov eax, 42; ret")

        with ASM() as asm:
            try:
                asm.runFast(bytes(encoding))
            except Exception as e:
                self.fail(f"ASM.runFast raised an exception with aligned code: {e}")


if __name__ == "__main__":
    unittest.main()
