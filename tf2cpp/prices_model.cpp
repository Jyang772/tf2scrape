/*
TABLES

MariaDB [tf2scrape]> select * from backpack limit 1;
+----+------------+----------+-------------+--------------------------------------+-----------------+----------+-------+-------------+
| id | batch_id   | defindex | insert_time | item_name                            | quality_integer | currency | price | last_update |
+----+------------+----------+-------------+--------------------------------------+-----------------+----------+-------+-------------+
|  1 | 1419822012 |     6512 | NULL        | Strange Filter: Lakeside (Community) |               6 | metal    |  0.66 |  1359096830 |
+----+------------+----------+-------------+--------------------------------------+-----------------+----------+-------+-------------+

MariaDB [tf2scrape]> select * from marketplace limit 1;
+----+---------------------+----------+-----------------------------------+----------+---------+------------+
| id | insert_time         | defindex | item_name                         | currency | price   | batch_id   |
+----+---------------------+----------+-----------------------------------+----------+---------+------------+
|  1 | 2014-12-29 03:00:34 |    30140 | Circling Heart Virtual Viewfinder | USD      | 1499.99 | 1419822012 |
+----+---------------------+----------+-----------------------------------+----------+---------+------------+

MariaDB [tf2scrape]> select * from scrap limit 1;
+----+----------+---------+-----------+-----------+-----------+------------+
| id | defindex | quality | key_price | ref_price | item_name | batch_id   |
+----+----------+---------+-----------+-----------+-----------+------------+
|  1 |      143 |       6 |        13 |         0 | Earbuds   | 1419822012 |
+----+----------+---------+-----------+-----------+-----------+------------+

 * Tradetf omitted
 * 
CREATE TABLE `tf2scrape`.`refined` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `batch_id` INT UNSIGNED NULL,
  `defindex` INT NULL,
  `item_name` VARCHAR(255) NULL,
  `currency` VARCHAR(45) NULL,
  `quality` VARCHAR(45) NULL,
  `price` DOUBLE NULL,
  PRIMARY KEY (`id`));

SELECT DISTINCT batch_id FROM tf2scrape.backpack WHERE batch_id > (SELECT MAX(batch_id) FROM tf2scrape.refined) LIMIT 10
*/
#include "prices_model.h"

#include "mysql_connection.h"
#include "mysql_driver.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <string>
#include <iostream>
#include <stdlib.h>

// Class will actually just assemble batches of data structures, ten at a time, though
// this can be adjusted if needed.
prices_model::prices_model()
{
	
	driver = sql::mysql::get_driver_instance();
	
	con = driver->connect("tcp://127.0.0.1:3306", "root", "pipe");
	
	stmt = con->createStatement();
	
	getRemaining();
	
}

void prices_model::getRemaining () {

	res = stmt->executeQuery("SELECT max(batch_id) AS remaining FROM tf2scrape.backpack");
	unsigned long bp, rf;
	
	while (res->next()) {
		
		std::string result = res->getString("remaining");
		bp = std::strtoul( result.c_str(), NULL, 0 );
		
	}
	
	res = stmt->executeQuery("SELECT IFNULL(max(batch_id),0) AS remaining FROM tf2scrape.refined");
	
	while (res->next()) {
		
		std::string result = res->getString("remaining");
		rf = std::strtoul( result.c_str(), NULL, 0 );
		
	}
	
	
	remaining = bp - rf;
	
}

void prices_model::getBatches () {
	
	batches.clear();
	
	res = stmt->executeQuery("SELECT DISTINCT batch_id FROM tf2scrape.backpack WHERE batch_id > (SELECT IFNULL(MAX(batch_id), 0) FROM tf2scrape.refined) LIMIT 10");
	
	int count = 0;
	
	while (res->next()) {
		
		batches += res->getString("batch_id") + ",";
		count++;
		
	}
	
	batches = batches.substr(0, batches.size()-1);
	remaining -= count;
	
}


// If there is another batch to work on
bool prices_model::nextBatch() {

		return remaining > 0;
	
}

/*
 * The punchline of the prices_model.
 * 
 * Within this function we will assemble the next batch available
 * in chunks of 10. This function will be called publicly.It will make
 * the differing sources uniform. We will form from each record as
 * much of the columns required to populate the refined table.
 * 
 * Synthesizing this many sources requires a structure that will be easy to calculate
 * 
 * When we iterate to create each refined and analyzed record, we need to first go
 * through each batch_id. From each batch_id we iterate through every map contained inside
 * Each map is a defindex. Inside each defindex is a quality key and inside each quality key
 * is an item and its data. This is what we combine and perform a regression upon. Then we
 * create a prices_model instance to assign that data and save to the refined table.
 * 
 * Batch ID
 * 		DefIndex
 * 			Quality Integer
 * 							Item Name
 * 							Currency
 * 							Price
 * 							Etc...
 *
 * 							Item Name
 * 							Currency
 * 							Price
 * 							Etc...
 * 
 * 
 * Since each source saves different currencies, we can only interpolate values based on
 * common currencies.
 * 
 */
std::map<int, std::map<int, std::map <int, std::vector < std::map <std::string, std::string> > > > > prices_model::get () {
	
	getRemaining();
	getBatches();
	
	std::map<int, std::map<int, std::map <int, std::vector < std::map <std::string, std::string> > > > > model;
	
	
	if (remaining > 0) {
		
		// Backpack First
		std::vector< std::map<std::string, std::string> >  bp = backpack();
		
		// Through each backpack item in the batch
		for ( auto &b : bp ) { 
			try {
				model[ std::stoi(b["batch_id"]) ][ std::stoi( b["defindex"] )][ std::stoi ( b["quality_integer"] )].push_back(b);
			} catch (const std::invalid_argument& ia) {
				continue;
			}
		}

		std::cout << "Marketplace" << std::endl;		
		// Marketplace Second
		std::vector< std::map<std::string, std::string> >  mp = marketplace();

		// Through each backpack item in the batch
		for ( auto &m : mp ) { 
			try {
				model[std::stoi(m["batch_id"])][std::stoi(m["defindex"])][std::stoi(m["quality_integer"])].push_back(m);
			} catch (const std::invalid_argument& ia) {
				continue;
			}
		}
	
		std::cout << "Tradetf" << std::endl;	
		// Tradetf third
		std::vector< std::map<std::string, std::string> >  tt = tradetf();
		// Through each backpack item in the batch
		for ( auto &t : tt ) { 
			try {
				model[std::stoi(t["batch_id"])][std::stoi(t["defindex"])][std::stoi(t["quality_integer"])].push_back(t);
			} catch (const std::invalid_argument& ia) {
				continue;
			}
		}

		std::cout << "Scrap" << std::endl;		
		// Scrap Last
		std::vector< std::map<std::string, std::string> >  sp = scrap();
		// Through each backpack item in the batch
		for ( auto &s : sp ) { 
			try {
				model[std::stoi(s["batch_id"])][std::stoi(s["defindex"])][std::stoi(s["quality"])].push_back(s);
			} catch (const std::invalid_argument& ia) {
				continue;
			}
		}
		
	}
	
	getRemaining();
	
	return model;
	
	
}

// convert from garbage sql data types
std::vector< std::map<std::string, std::string> > prices_model::backpack() {

	res = stmt->executeQuery("SELECT * FROM tf2scrape.backpack WHERE batch_id IN (" + batches + ")");
	
	std::vector< std::map<std::string, std::string> > ret;

	while (res->next()) {
		
		std::map<std::string, std::string> record;
		
		record["batch_id"] = res->getString("batch_id");
		record["defindex"] = res->getString("defindex");
		record["item_name"] = res->getString("item_name");
		record["currency"] = res->getString("currency");
		record["price"] = res->getString("price");
		record["quality_integer"] = res->getString("quality_integer");
		
		ret.push_back(record);
		
		
	}
	
	return ret;
	
}

// convert from garbage sql data types
std::vector< std::map<std::string, std::string> > prices_model::marketplace () {
	
	res = stmt->executeQuery("SELECT * FROM tf2scrape.marketplace WHERE batch_id IN (" + batches + ")");
	
	std::vector< std::map<std::string, std::string> > ret;

	while (res->next()) {
		
		std::map<std::string, std::string> record;
		
		record["batch_id"] = res->getString("batch_id");
		record["defindex"] = res->getString("defindex");
		record["item_name"] = res->getString("item_name");
		record["currency"] = res->getString("currency");
		record["price"] = res->getString("price");
		// Marketplace lacks reliable quality marking assuming normal (0)
		record["quality_integer"] = "0";
		
		ret.push_back(record);
		
	}
	
	return ret;
	
}

// convert from garbage sql data types
std::vector< std::map<std::string, std::string> > prices_model::tradetf () {

	res = stmt->executeQuery("SELECT * FROM tf2scrape.tradetf WHERE batch_id IN (" + batches + ")");
	
	std::vector< std::map<std::string, std::string> > ret;

	while (res->next()) {
		
		std::map<std::string, std::string> record;
		std::map<std::string, std::string> qrecord;

		record["batch_id"] = res->getString("batch_id");
		record["defindex"] = res->getString("defindex");
		record["item_name"] = res->getString("item_name");
		
		if (res->getString("unique_max") != "0") {
			
			qrecord["currency"] = res->getString("unique_currency");
			qrecord["price"] = res->getString("unique_max");
			qrecord["quality_integer"] = "6";
			record.insert(qrecord.begin(), qrecord.end());
			ret.push_back(record);
			
		}
		
		if (res->getString("strange_max") != "0") {
			
			//qrecord["currency"] = res->getString("strange_currency");
			qrecord["price"] = res->getString("strange_max");
			qrecord["quality_integer"] = "11";
			record.insert(qrecord.begin(), qrecord.end());
			
			ret.push_back(record);
			
		}
		
		if (res->getString("uncraft_max") != "0") {
			
			qrecord["currency"] = res->getString("uncraft_currency");
			qrecord["price"] = res->getString("uncraft_max");
			qrecord["quality_integer"] = "0";
			record.insert(qrecord.begin(), qrecord.end());
			
			ret.push_back(record);
			
		}
		
		if (res->getString("vintage_max") != "0") {
			
			qrecord["currency"] = res->getString("vintage_currency");
			qrecord["price"] = res->getString("vintage_max");
			qrecord["quality_integer"] = "3";
			record.insert(qrecord.begin(), qrecord.end());
			
			ret.push_back(record);
			
		}
		
		if (res->getString("genuine_max") != "0") {
			
			qrecord["currency"] = res->getString("genuine_currency");
			qrecord["price"] = res->getString("genuine_max");
			qrecord["quality_integer"] = "1";
			record.insert(qrecord.begin(), qrecord.end());
			
			ret.push_back(record);
		}
		
	}
	
	return ret;
	
}

// convert from garbage sql data types
/*


MariaDB [tf2scrape]> select * from scrap limit 1;
+----+----------+---------+-----------+-----------+-----------+------------+
| id | defindex | quality | key_price | ref_price | item_name | batch_id   |
+----+----------+---------+-----------+-----------+-----------+------------+
|  1 |      143 |       6 |        13 |         0 | Earbuds   | 1419822012 |
+----+----------+---------+-----------+-----------+-----------+------------+


*/
std::vector< std::map<std::string, std::string> > prices_model::scrap () {
	
	res = stmt->executeQuery("SELECT * FROM tf2scrape.scrap WHERE batch_id IN (" + batches + ")");
	
	std::vector< std::map<std::string, std::string> > ret;

	while (res->next()) {
		
		std::map<std::string, std::string> record;
		
		record["batch_id"] = res->getString("batch_id");
		record["defindex"] = res->getString("defindex");
		record["item_name"] = res->getString("item_name");
		record["quality_integer"] = res->getString("quality");
		
		if (res->getString("key_price") != "0") {
			
			record["currency"] = "key";
			record["price"] = res->getString("key_price");
			ret.push_back(record);
			
		} else if (res->getString("ref_price") != "0") {
			
			record["currency"] = "ref";
			record["price"] = res->getString("ref_price");
			ret.push_back(record);
			
		}
		
	}
	
	return ret;
	
}



// Save the model to the database refined table
// This is after setting public variables
void prices_model::save() {
	
	try {
		std::string qstr = "INSERT INTO tf2scrape.refined (batch_id, defindex, item_name, currency, quality, price) VALUES";
		qstr += "(" + std::to_string(batch_id) + "," + std::to_string(defindex) + ", \"";
		qstr += item_name + "\" ,\"" + currency + "\", \"" + quality + "\"," + std::to_string(price) + ")";
		
		sql::SQLString Qstr = qstr.c_str();
		
		stmt->executeQuery(Qstr);
		
		
	} catch (sql::SQLException &e) {
		std::cout << e.what();
		return;
		
	}
	
	
	
}

prices_model::~prices_model() {

		delete driver;
		delete con;
		delete stmt;
		delete res;

}

	
	
