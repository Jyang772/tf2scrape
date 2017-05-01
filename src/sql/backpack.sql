-- name save-backpack!
INSERT INTO `backpack`
(batch_id, defindex, item_name, quality_integer, currency, price, last_update)
VALUES (?, ?, ?, ?, ?, ?, ?);
