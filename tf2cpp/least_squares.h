#ifndef LEAST_SQUARES_H
#define LEAST_SQUARES_H

#include <vector>

class least_squares
{
	
public:
	least_squares (std::vector<double>);
	double getAvg ();
	~least_squares();
private:
	
	double avg_pt;
	double avgY(std::vector<double>);
	
};

#endif // LEAST_SQUARES_H
