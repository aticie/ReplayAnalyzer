#include "math_helpers.h"
#include <cmath>

float vector_distance(sf::Vector2f pt1, sf::Vector2f pt2)
{
	return sqrt((pt2.x - pt1.x) * (pt2.x - pt1.x) +
		(pt2.y - pt1.y) * (pt2.y - pt1.y));
}

int binary_search(std::vector<replay_frame> arr, const int l, const int r, const int x)
{
	const auto mid = l + (r - l) / 2;
	if (r >= l)
	{
		// If the element is present at the middle 
		// itself 
		if (arr[mid].get_abs_time() == x)
			return mid;

		// If element is smaller than mid, then 
		// it can only be present in left subarray 
		if (arr[mid].get_abs_time() > x)
			return binary_search(arr, l, mid - 1, x);

		// Else the element can only be present 
		// in right subarray 
		return binary_search(arr, mid + 1, r, x);
	}

	// We reach here when element is not 
	// present in array 
	return mid;
}
