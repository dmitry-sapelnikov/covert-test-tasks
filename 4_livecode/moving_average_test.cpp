#include "moving_average.h"
#include <iostream>
#include <sstream>

void check_equal(double actual, double expected, double epsilon = 1e-6)
{
	if (std::abs(expected - actual) > epsilon)
	{
		std::stringstream str;
		std::cerr << "Values are not equal: expected " << expected << " != actual " << actual << std::endl;
	}
}

int main()
{
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

	return 0;
}
