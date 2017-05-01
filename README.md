# TF2Scrape



TF2Scrape is a Team Fortress 2 trading site web scraper written in Clojure. This implementation was used to test numerical linear approximation techniques. The code is provided as is.



### Installation



TF2Scrape requires [Clojure](http://www.lispcast.com/clojure-ubuntu) and [MySQL](https://www.digitalocean.com/community/tutorials/how-to-install-mysql-on-ubuntu-16-04) to run.



**1.** Install Clojure.

**2.** Create MySQL database `tf2scrape`

**3.** Create the tables listed in `create_tables.txt`

**4.** Build standalone .jar.

**5.** Compile and run `tf2cpp`.


## Run

```
$ cd ~/tf2scrape
$ lein uberjar
$ cd ~/tf2scrape/target/uberjar
$ java -jar tf2*-standalone.jar
```

## Tables

```sh
mysql> use tf2scrape;
Reading table information for completion of table and column names
You can turn off this feature to get a quicker startup with -A

Database changed

mysql> show tables;

+---------------------+
| Tables_in_tf2scrape |
+---------------------+
| backpack            |
| marketplace         |
| refined             |
| scrap               |
| tradetf             |
+---------------------+

5 rows in set (0.00 sec)

mysql> describe backpack;

+-----------------+------------------+------+-----+---------+----------------+
| Field           | Type             | Null | Key | Default | Extra          |
+-----------------+------------------+------+-----+---------+----------------+
| id              | int(10) unsigned | NO   | PRI | NULL    | auto_increment |
| batch_id        | int(10) unsigned | YES  |     | NULL    |                |
| defindex        | int(11)          | YES  |     | NULL    |                |
| item_name       | varchar(255)     | YES  |     | NULL    |                |
| quality_integer | varchar(45)      | YES  |     | NULL    |                |
| currency        | varchar(45)      | YES  |     | NULL    |                |
| price           | double           | YES  |     | NULL    |                |
| last_update     | int(8)           | YES  |     | NULL    |                |
+-----------------+------------------+------+-----+---------+----------------+
```

## Interpolated data

Since each source saves different currencies, we can only interpolate values based on common currencies (refined metal).

```
mysql> SELECT * FROM refined LIMIT 10;
+----+------------+----------+------------------+----------+---------+-------+
| id | batch_id   | defindex | item_name        | currency | quality | price |
+----+------------+----------+------------------+----------+---------+-------+
|  1 | 1493588458 |       -2 | Random Craft Hat | usd      | 0       |  0.04 |
|  2 | 1493588458 |       -2 | Random Craft Hat | metal    | 6       |  1.22 |
|  3 | 1493588458 |       10 | Shotgun          | metal    | 6       |  0.55 |
|  4 | 1493588458 |       10 | Shotgun          | metal    | 11      |     9 |
|  5 | 1493588458 |       18 | Rocket Launcher  | metal    | 6       |  0.33 |
|  6 | 1493588458 |       18 | Rocket Launcher  | keys     | 11      |    40 |
|  7 | 1493588458 |       23 | Pistol           | metal    | 6       |  0.77 |
|  8 | 1493588458 |       23 | Pistol           | metal    | 11      |  2.22 |
|  9 | 1493588458 |       24 | Revolver         | metal    | 6       |  0.66 |
| 10 | 1493588458 |       24 | Revolver         | metal    | 11      |  1.55 |
+----+------------+----------+------------------+----------+---------+-------+
```

### Todos

 - CMake

 - Add numerical methods in C/C++

 - Create tables within Clojure
