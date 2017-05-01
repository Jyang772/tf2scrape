(ns tf2scrape.tfdb
  (:require [yesql.core :refer [defquery]]))

(def db-spec
  {:classname "org.mysql.Driver"
   :subprotocol "mysql"
   :subname "//localhost:3306/tf2scrape"
   :user "root"
   :password "pipe"})

;; Define SQL statements
(defquery save-backpack! "sql/backpack.sql")
(defquery marketplace! "sql/marketplace.sql")
(defquery scrap! "sql/scrap.sql")
(defquery trade! "sql/trade.sql")

;; Wrappers
(defn save-to-backpack
  "Saves data from backpack API to database"
  [batch-id defindex item-name quality-integer currency price last_update]
  (when-not (nil? price)
   (save-backpack! db-spec batch-id defindex item-name quality-integer currency price last_update)))

(defn save-marketplace
  [item-name item-price defindex batch-id]
  (marketplace! db-spec
   (clojure.string/trim item-name)
   (read-string (clojure.string/replace (clojure.string/replace item-price #"," "") #"\$" ""))
   defindex
   batch-id))

(defn save-scrap
  [quality defindex item-name keys refs batch-id]
  (scrap! db-spec quality defindex item-name keys refs batch-id))

(defn save-trade
  [item_name defindex batch_id unique_min unique_max unique_currency uncraft_min uncraft_max uncraft_currency vintage_min vintage_max vintage_currency genuine_min genuine_max genuine_currency strange_min strange_max]
  (trade! db-spec item_name defindex batch_id unique_min unique_max unique_currency uncraft_min uncraft_max uncraft_currency vintage_min vintage_max vintage_currency genuine_min genuine_max genuine_currency strange_min strange_max))
