# pgxicor: XI (ξ) Correlation Coefficient in Postgres

[![build](https://github.com/Florents-Tselai/pgxicor/actions/workflows/build.yml/badge.svg)](https://github.com/Florents-Tselai/pgxicor/actions/workflows/build.yml)
![GitHub Repo stars](https://img.shields.io/github/stars/Florents-Tselai/pgxicor)
![Docker Pulls](https://img.shields.io/docker/pulls/florents/pgxicor)

**pgxicor** is a Postgres extension that exposes a `SELECT xicor(X, Y)` aggregate function.
XI can detect functional relationships between X and Y.
You can use it as a more powerful alternative to standard `corr(X, Y)` which works best on linear relationships only.

For more information on XI, see the original paper.
[A New Coefficient of Correlation S. Chatterjee 2020](https://gwern.net/doc/statistics/order/2020-chatterjee.pdf)

## Usage

```tsql
CREATE TABLE xicor_test (x float8, y float8);
INSERT INTO xicor_test (x, y)
VALUES
    (1.0, 2.0),
    (2.5, 3.5),
    (3.0, 4.0),
    (4.5, 5.5),
    (5.0, 6.0);

-- Query to calculate the Xi correlation using the aggregate function
SELECT xicor(x, y) FROM xicor_test;
```
## Installation

```
cd /tmp
git clone https://github.com/Florents-Tselai/pgxicor.git
cd pgxicor
make
make install # may need sudo
```

After the installation, in a session:

```tsql
CREATE EXTENSION pgxicor;
```

### Docker

Get the [Docker image](https://hub.docker.com/r/florents/pgxicor) with:

```sh
docker pull florents/pgxicor:pg17
```

This adds pgxicor to the [Postgres image](https://hub.docker.com/_/postgres) (replace `17` with your Postgres server version, and run it the same way).

Run the image in a container.

```sh
docker run --name pgxicor -p 5432:5432 -e POSTGRES_PASSWORD=pass florents/pgxicor:pg17
```

Through another terminal, connect to the running server (container).

```sh
PGPASSWORD=pass psql -h localhost -p 5432 -U postgres
```
