#ifndef PRICES_MODEL_H
#define PRICES_MODEL_H


#include "mysql_connection.h"
#include "mysql_driver.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <vector>
#include <map>
#include <string>

class prices_model
{
	
public:


	// Variables to populate when saving a model
	int batch_id, defindex;
	std::string item_name, currency, quality;
	double price;

	prices_model ();
	void save();
	std::map<int, std::map<int, std::map <int, std::vector < std::map <std::string, std::string> > > > > get();
	bool nextBatch();
	~prices_model();
	
private:

	
	
	sql::mysql::MySQL_Driver *driver;
	sql::Connection *con;
	sql::Statement *stmt;
	sql::ResultSet *res;
	std::string batches;
	unsigned long remaining;
	
	void getRemaining();
	void getBatches();
	
	std::vector< std::map<std::string, std::string> > backpack();
	std::vector< std::map<std::string, std::string> > scrap ();
	std::vector< std::map<std::string, std::string> >  marketplace ();
	std::vector< std::map<std::string, std::string> >  tradetf ();
	
	

};

#endif // PRICES_MODEL_H
