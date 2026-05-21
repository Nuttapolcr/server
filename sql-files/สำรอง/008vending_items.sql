CREATE TABLE `vending_items` (
  `vending_id` int(10) UNSIGNED NOT NULL,
  `index` smallint(5) UNSIGNED NOT NULL,
  `cartinventory_id` int(10) UNSIGNED NOT NULL,
  `amount` smallint(5) UNSIGNED NOT NULL,
  `price` int(10) UNSIGNED NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=tis620 ROW_FORMAT=FIXED;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `vending_items`
--
ALTER TABLE `vending_items`
  ADD PRIMARY KEY (`vending_id`,`index`) USING BTREE;
COMMIT;