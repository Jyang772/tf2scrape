#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <typeinfo>

#include "prices_model.h"
#include "least_squares.h"

int main(int argc, char **argv)
{

	prices_model * PM = new prices_model;
	
	//int so_far = 0;

	while ( PM->nextBatch() ) {
		
		std::map<int, std::map<int, std::map <int, std::vector < std::map <std::string, std::string> > > > > pm = PM->get();

		for (auto it = pm.begin(); it != pm.end(); ++it) {
			
			PM->batch_id = it->first;
			
			for (auto itt = it->second.begin();  itt != it->second.end(); ++itt ) {
				
				PM->defindex = itt->first;
				
				for (auto ittt = itt->second.begin(); ittt != itt->second.end(); ++ittt)  {
					
					PM->quality = std::to_string (ittt->first);
					
					std::map< std::string, std::vector<double> > acc;
					
					for (auto m = ittt->second.begin(); m != ittt->second.end(); ++m ) {
						
						PM->item_name = (*m)["item_name"];
						acc[(*m)["currency"]].push_back(std::stod((*m)["price"]));

					}
					
					std::vector<double>::size_type mx_sz = 0;
					std::string common_cur = "ref";
					for (auto a = acc.begin(); a != acc.end(); ++a) {
						
						if (a->second.size() > mx_sz) {
							
							mx_sz = a->second.size();
							common_cur = a->first;
							
						}
							
					}
					
					least_squares * ls = new least_squares(acc[common_cur]);
					PM->currency = common_cur;
					PM->price = ls->getAvg();
					
					PM->save();
					delete ls;
					//so_far++;
					//std::cout << so_far << " ";
					
				}
					
					
				
			}
			
			
		}
		
		
		
	}
	
	//char x;
	//std::cin >> x;
	
}
