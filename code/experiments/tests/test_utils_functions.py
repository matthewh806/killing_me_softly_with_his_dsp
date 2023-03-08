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

        with self.assertRaises(ValueError):
            UF.twoPointInterpolation(2.0, 0.0, 0.75)

if __name__ == "__main__":
    unittest.main()
