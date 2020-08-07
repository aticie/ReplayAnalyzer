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

hit_event find_closest(std::vector<hit_event> arr, const int target)
{
    // Corner cases
    arr.pop_back();
    if (target <= arr[0].hit_time)
        return arr[0];
    if (target >= arr.back().hit_time)
        return arr.back();

    // Doing binary search 
    unsigned i = 0, j = arr.size(), mid = 0;
    while (i < j) {
        mid = (i + j) / 2;

        if (arr[mid].hit_time == target)
            return arr[mid];

        /* If target is less than array element,
            then search in left */
        if (target < arr[mid].hit_time) {

            // If target is greater than previous 
            // to mid, return closest of two 
            if (mid > 0 && target > arr[mid - 1].hit_time)
                return arr[mid + get_closest(arr[mid - 1].hit_time,
                    arr[mid].hit_time, target)];

            /* Repeat for left half */
            j = mid;
        }

        // If target is greater than mid 
        else {
            if (mid < arr.size() - 1 && target < arr[mid + 1].hit_time)
                return arr[mid + get_closest(arr[mid].hit_time,
                    arr[mid + 1].hit_time, target) + 1];
            // update i 
            i = mid + 1;
        }
    }

    // Only single element left after search 
    return arr[mid];
}

auto get_closest(const int64_t val1, const int64_t val2,
                 const int target) -> int
{
    if (target - val1 >= val2 - target)
        return 0;
    else
        return -1;
}
