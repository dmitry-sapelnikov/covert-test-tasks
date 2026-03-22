#include <iostream>
#include "moving_average.h"

namespace
{
	/// Checks equality of two floating-point numbers
	void check_equal(
		double actual,
		double expected,
		double tolerance = 1e-6)
	{
		if (std::abs(expected - actual) > tolerance)
		{
			std::cerr << "Values are not equal: ";
			std::cerr << "expected " << expected << " != actual " << actual << "\n";
		}
	}

// end of anonymous namespace
}

/// Test the MovingAverage class
int main()
{
	std::cout << "Test detection of invalid values with check_equal() :\n";
	check_equal(0.1 + 0.2, 0.5); // should report error

	std::cout << "\nTesting MovingAverage class:\n";
	// time:    0    1    2    3    4    5    6    7    8    9   10
	// value:   1    1    1    2    2    3    3    3    4    5    6
	// average: 1            5/4       9/5           15/5 18/5 21/5

	moving_average::MovingAverage<uint64_t, double> ma(5);
	check_equal(ma.add_event(0, 1.0), 1.0);
	check_equal(ma.add_event(3, 2.0), 5.0 / 4.0);
	check_equal(ma.add_event(5, 3.0), 9.0 / 5.0);
	check_equal(ma.add_event(8, 4.0), 15.0 / 5.0);
	check_equal(ma.add_event(9, 5.0), 18.0 / 5.0);
	check_equal(ma.add_event(10, 6.0), 21.0 / 5.0);
	std::cout << "Tests completed.\n";

	return 0;
}
