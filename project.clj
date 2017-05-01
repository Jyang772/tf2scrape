(defproject tf2scrape "1.0.0"
  :description "TF2 Scraper from multiple sites"
  :url "http://micropenguin.net/"
  :license {:name "Private"
            :url "http://micropenguin.net/"}
  :dependencies [[org.clojure/clojure "1.6.0"]
                 [yesql "0.4.0"]
                 [mysql/mysql-connector-java "5.1.6"]
                 [cheshire "5.3.1"]
                 [clj-http "1.0.1"]
                 [enlive "1.1.5"]
                 [overtone/at-at "1.2.0"]]
  :plugins [[cider/cider-nrepl "0.8.1"]]
  :main ^:skip-aot tf2scrape.core
  :target-path "target/%s"
  :profiles {:uberjar {:aot :all}})
