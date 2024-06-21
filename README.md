# nuketh

A dead-simple Ethereum collision tool

## Usage

### 1. Collect Target Ethereum Addresses

> [!WARNING]
>
> This step will cost you $50+ on Google BigQuery, please make sure you have enough budget.
>
> Or you can just download [Pre-collected CSV Files](https://www.patreon.com/yankeguo/shop/csv-all-eth-addresses-with-balances-2024-241529?utm_medium=clipboard_copy&utm_source=copyLink&utm_campaign=productshare_fan&utm_content=join_link)

#### 1.1 Collect Ethereum Addresses and Balance

Google BigQuery provides public Ethereum on-chain data and allows you to perform complex SQL aggregation queries.

Execute this SQL with Google BigQuery and export to a dedicated table `eth_addresses`:

```sql
#standardSQL
  -- MIT License
  -- Copyright (c) 2018 Evgeny Medvedev, evge.medvedev@gmail.com
WITH
  double_entry_book AS (
    -- debits
  SELECT
    to_address AS address,
    value AS value
  FROM
    `bigquery-public-data.crypto_ethereum.traces`
  WHERE
    to_address IS NOT NULL
    AND status = 1
    AND (call_type NOT IN ('delegatecall',
        'callcode',
        'staticcall')
      OR call_type IS NULL)
  UNION ALL
    -- credits
  SELECT
    from_address AS address,
    -value AS value
  FROM
    `bigquery-public-data.crypto_ethereum.traces`
  WHERE
    from_address IS NOT NULL
    AND status = 1
    AND (call_type NOT IN ('delegatecall',
        'callcode',
        'staticcall')
      OR call_type IS NULL)
  UNION ALL
    -- transaction fees debits
  SELECT
    miner AS address,
    SUM(CAST(receipt_gas_used AS numeric) * CAST(gas_price AS numeric)) AS value
  FROM
    `bigquery-public-data.crypto_ethereum.transactions` AS transactions
  JOIN
    `bigquery-public-data.crypto_ethereum.blocks` AS blocks
  ON
    blocks.number = transactions.block_number
  GROUP BY
    blocks.miner
  UNION ALL
    -- transaction fees credits
  SELECT
    from_address AS address,
    -(CAST(receipt_gas_used AS numeric) * CAST(gas_price AS numeric)) AS value
  FROM
    `bigquery-public-data.crypto_ethereum.transactions` )
SELECT
  address,
  SUM(value) AS balance
FROM
  double_entry_book
GROUP BY
  address
ORDER BY
  balance DESC
```

> Reference: https://medium.com/google-cloud/how-to-query-balances-for-all-ethereum-addresses-in-bigquery-fb594e4034a7

#### 1.2 Filter Addresses with Balance

Execute a new query and save to a new table `eth_addresses_with_balance`

```sql
select * from `eth_addresses` where balance > 0;
```

#### 1.3 Export to CSV Files and Download

Export table to CSV files to Google Storage and download them, remember to add a asterisk to export destination filename since it exceeds the maximum allowed single file size (1G).

### 2. Build a Binary Addresses List

Every Ethereum address is 20-bytes width, we just list all addresses in binary format, and this will be our Binary Addresses List (BAL) file.

```shell
./nuketh-csv2bal dir/to/csv nuketh.bal
```

### 3. Nuke it

```shell
./nuketh nuketh.bal result.txt
```

## Credits

GUO YANKE, MIT License
