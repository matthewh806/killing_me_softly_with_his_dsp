import unittest
import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../'))
import utils_functions as UF

class TestUtilsFunctions(unittest.TestCase):

    def test_linear_interpolation(self):
        self.assertAlmostEqual(UF.twoPointInterpolation(0.0, 1.0, 0.0), 0.0)
        self.assertAlmostEqual(UF.twoPointInterpolation(0.0, 1.0, 1.0), 1.0)
        self.assertAlmostEqual(UF.twoPointInterpolation(0.0, 1.0, 0.25), 0.25)
        self.assertAlmostEqual(UF.twoPointInterpolation(0.0, 1.0, 0.75), 0.75)
        
        self.assertAlmostEqual(UF.twoPointInterpolation(1.0, 2.0, -0.3), 1.0)
        self.assertAlmostEqual(UF.twoPointInterpolation(1.0, 2.0, 2.6), 2.0)

        self.assertAlmostEqual(UF.twoPointInterpolation(2.5, 2.5, 0.375), 2.5)

        self.assertAlmostEqual(UF.twoPointInterpolation(3.75, 4.20, 0.67), 4.0515)

        self.assertAlmostEqual(UF.twoPointInterpolation(2.4, 0.5, 0.4), 1.64)


class TestMultitapCircularBuffer(unittest.TestCase):

    def test_zero_size_multitap_initialisation(self):
        with self.assertRaises(ValueError):
            UF.MultitapCircularBuffer(0)

    def test_multitap_initialisation(self):
        size = 10
        multitap = UF.MultitapCircularBuffer(size)

        # Validate Size
        self.assertTrue(multitap.size, size)
        self.assertTrue(len(multitap.data), size)

        # Validate initialisation
        for i in range(len(multitap.data)):
            self.assertAlmostEqual(multitap.data[i], 0.0)

    def test_multitap_write(self):
        size = 10
        multitap = UF.MultitapCircularBuffer(size)

        multitap.write(3.0)

        # Validate Value
        self.assertEqual(multitap.data[0], 3)

        # Validate write HEAD position
        self.assertEqual(multitap.head, 1)

        # Track read and write head in loop
        for i in range(1, size-1):
            multitap.write(i)

            self.assertEqual(multitap.data[i], i)
            self.assertEqual(multitap.head, i+1)

        # Check for wrap around and overwrite
        multitap.write(10)

        self.assertEqual(multitap.data[size-1], 10)
        self.assertEqual(multitap.head, 0)

        multitap.write(11)
        self.assertEqual(multitap.data[0], 11)
        self.assertEqual(multitap.head, 1)

    def test_multitap_read(self):
        size = 10
        multitap = UF.MultitapCircularBuffer(size)

        # Fill buffer with data
        for i in range(size):
            multitap.write(i)

        with self.assertRaises(ValueError):
            multitap.read(5)

        # By default the tap delays are all equal to 1 sample
        # So check we get the value we'd expect one before the write head
        self.assertEqual(multitap.read(0), 9)
        self.assertEqual(multitap.read(1), 9)
        self.assertEqual(multitap.read(2), 9)
        self.assertEqual(multitap.read(3), 9)

        # Advance the read pointer
        multitap.write(100)
        self.assertEqual(multitap.read(0), 100)
        self.assertEqual(multitap.read(1), 100)
        self.assertEqual(multitap.read(2), 100)
        self.assertEqual(multitap.read(3), 100)

        # Different delay times
        # Check for wrapping correctly
        # Check for fractional delays
        multitap = UF.MultitapCircularBuffer(size, 5, 7.5, 16.75, 93.3)
        for i in range(size):
            multitap.write(i)

        # Test values can simply be obtained with delayfrac % size
        self.assertEqual(multitap.read(0), 5)
        self.assertEqual(multitap.read(1), 2.5)
        self.assertEqual(multitap.read(2), 3.25)
        self.assertEqual(multitap.read(3), 6.700000000000003)

        # Advance write pointer by arbitrary amount
        multitap.write(0)
        multitap.write(1)
        multitap.write(2)
        multitap.write(3)
        multitap.write(4)
        multitap.write(5)
        multitap.write(6)

        # Head is now at idx 7
        self.assertEqual(multitap.head, 7)
        # Same tests again, values will be different
        self.assertEqual(multitap.read(0), 2)
        self.assertEqual(multitap.read(1), 4.5)
        self.assertEqual(multitap.read(2), 0.25)
        self.assertEqual(multitap.read(3), 3.700000000000003)

    def test_multitap_peek(self):
        size = 10
        multitap = UF.MultitapCircularBuffer(size)

        for i in range(size):
            multitap.write(i)

        self.assertEqual(multitap.data[multitap.head-1], multitap.peek())

    def test_multitap_clear(self):
        size = 10
        multitap = UF.MultitapCircularBuffer(size)
        
        for i in range(7):
            multitap.write(i)

        self.assertEqual(multitap.head, 7)

        multitap.clear()
        
        for i in range(size):
            self.assertAlmostEqual(multitap.data[i], 0.0)

        self.assertEqual(multitap.head, 0)
        self.assertEqual(multitap.size, size)

if __name__ == "__main__":
    unittest.main()
