#include "least_squares.h"
#include <numeric>
#include <vector>

/**
 * Over time the least squares is nonlinear, and so
 * we will numerically analyze each into a nonlinear case.
 * Due to the period between points in time (batch_ids) 
 * the slope is identically zero. A least squares will 
 * thus only have the Y avg between subsets of points.
 * 
 * Note: batch_id's are unix timestamps
 */
least_squares::least_squares(std::vector<double> pts)
{
	
	avg_pt = avgY(pts);
	
}

double least_squares::avgY (std::vector<double> ys) {
	
	/*double sum = 0;
	
	for (std::vector<double>::iterator it = ys.begin(); it != ys.end(); ++it) {
		
		sum += *it;
		
	}*/
	
	
	double sum = std::accumulate(ys.begin(), ys.end(), 0.0);
	return sum / ys.size();
	
}

double least_squares::getAvg() { return avg_pt; }


least_squares::~least_squares()
{
}


