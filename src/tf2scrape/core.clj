(ns tf2scrape.core
  (:require [clojure.java.io :as io]
            [clj-http.client :as client]
            [cheshire.core :as cheshire]
            [net.cgrand.enlive-html :as html]
            [tf2scrape.tfdb :as db])
  (:gen-class))

(do (System/setProperty "http.agent" "TFScraper"))

(def batch-id (quot (System/currentTimeMillis) 1000))

(def backpack-api-key
  "548cf351b88d88140c8b4756")

(def backpack-api-url
  (str "http://backpack.tf/api/IGetPrices/v4/?key=" backpack-api-key))

(defn save-temp
  "General use temporary file"
  [content]
  (spit (str (System/getProperty "user.dir") "//temp.json") content))

;; Backpack API Data Functions

(defn get-backpack-api
  "Gets the backpack api and saves to a temporary file"
  []
  (save-temp
   (:body
     (client/get backpack-api-url))))

(defn backpack-json-items
  []
  (vec
   (:items
    (:response
     (cheshire/decode
      (slurp (str (System/getProperty "user.dir") "//temp.json"))
      true)))))

(defn quick-traverse
  "Backpack API structure is poorly concieved"
  [col]
  (-> col second first second first second first))

(defn parse-map-backpack-api
  "Reads the temporary file, parses JSON, creates
  collection to pass to SQL statement for inserts"
  []
  (doall
   (map
    (fn [item]
      (doall
       (map
        (fn [price]
          (let [item-detail (quick-traverse price)]
            (db/save-to-backpack
             batch-id
             (first (:defindex (second item)))
             (name (first item))
             (name (first price))
             (-> item-detail :currency str)
             (-> item-detail :value)
             (-> item-detail :last_update))))
        (:prices (second item)))))
    (backpack-json-items))))
;; DOM Loader
(defn dom-loader
  "Will always load from temp.json"
  []
  (html/html-resource
   (io/reader (str (System/getProperty "user.dir") "//temp.json"))))

;; DOM Loader for marketplace (has broken HTML)
(defn dom-loader-marketplace
  []
  (do
   (save-temp
    (str
     (:body
      (client/get "https://marketplace.tf/all")) "</script></body></html>"))
   (dom-loader)))

;; Marketplace TF2 data scrape
(defn marketplace
  "Get name, defindex, and price. All currency is in USD."
  []
  (let [dom (dom-loader-marketplace)
        items (html/select dom [:#all-items :div.item-box :div.item-box-detail])]
    (doall
     (map
      (fn [item] (db/save-marketplace
                 (html/text (first (html/select item [:div.item-box-name])))
                 (html/text (first (html/select item [:div.item-box-price])))
                 (first
                  (clojure.string/split
                   (-> (html/select item [:button]) first :attrs :data-sku str)
                   #";"))
                 batch-id))
      items))))

;; Scrap.tf scraper
(defn dom-loader-scrap []
  (do
    (save-temp
     (str
      (:body
       (client/get "https://scrap.tf/items"))))
    (dom-loader)))

(defn scrap-price
  [price-str]
  (reduce
    conj
    (doall (map
            (fn [price] (hash-map
               (clojure.string/trim (apply str (filter #(not (Character/isDigit %)) price)))
               (double (read-string price))  ))
            (clojure.string/split price-str #"," )))))

(defn scrap
  "Get the name, quality, defindex, and prices (keys and/or ref)"
  []
  (let [dom (dom-loader-scrap)
        items (html/select dom [:#itembanking-list :tbody :tr])]
    (doall
     (map
      (fn [item] (let [prices (scrap-price
                              (clojure.string/trim
                               (first
                                (:content
                                 (nth (html/select item [:td]) 2)))))]
                  (db/save-scrap
                   (-> item :attrs :data-quality)
                   (-> item :attrs :data-defindex)
                   (html/text (second (html/select item [:td])))
                   (if (nil? (prices "keys")) 0 (prices "keys"))
                   (if (nil? (prices "refined")) 0 (prices "refined"))
                   batch-id)))
      items))))

;; Trade.tf scraper. This site does not contain defindex information.
(defn dom-loader-trade []
  (do
    (save-temp
     (str
      (:body
       (client/get "http://www.trade.tf/spreadsheet"))))
    (dom-loader)))

(defn trade-def
  [def-str]
  (Integer. (nth (clojure.string/split def-str #"/") 2)))

(defn trade-def-search
  [item-data]
  (trade-def
   (:href
    (:attrs
     (first
      (html/select
       (first
        (filter
         (fn [cell] (-> (html/select cell [:a]) empty? not))
         item-data)) [:a]))))))

(defn trade-min
  [price-str]
  (if-not (empty? price-str)
    (double (read-string (first (clojure.string/split price-str #"-"))))
    0))

(defn trade-max
  [price-str]
  (if-not (empty? price-str)
    (if-let [max (second (clojure.string/split price-str #"-"))]
      (double (read-string max))
      (double (read-string (first (clojure.string/split price-str #"-")))))
    0))

(defn trade-curr
  [price-str]
  (if-not (empty? price-str)
    (clojure.string/trim (second (clojure.string/split (clojure.string/trim price-str) #" ")))
    "ref"))

(defn tradetf
  "Get the name, min-max of qualities and currencies and defindex."
  []
  (let [dom (dom-loader-trade)
        items (html/select dom [:#spreadsheet :tbody :tr])]
    (doall
     (map
      (fn [item] (let [item-data (html/select item [:td])]
                  (db/save-trade

                  (-> (html/select item [:th]) first :attrs :abbr)

                  (trade-def-search item-data)

                   batch-id

                   (trade-min (html/text (second item-data)))
                   (trade-max (html/text (second item-data)))
                   (trade-curr (html/text (second item-data)))

                   (trade-min (html/text (nth item-data 2)))
                   (trade-max (html/text (nth item-data 2)))
                   (trade-curr (html/text (nth item-data 2)))

                   (trade-min (html/text (nth item-data 3)))
                   (trade-max (html/text (nth item-data 3)))
                   (trade-curr (html/text (nth item-data 3)))

                   (trade-min (html/text (nth item-data 4)))
                   (trade-max (html/text (nth item-data 4)))
                   (trade-curr (html/text (nth item-data 4)))

                   (trade-min (html/text (nth item-data 5)))
                   (trade-max (html/text (nth item-data 5))))))
      items))))

(defn -main
  "I don't do a whole lot ... yet."
  [& args]
  (get-backpack-api)
  (parse-map-backpack-api)
  (marketplace)
  (scrap)
  (tradetf))
