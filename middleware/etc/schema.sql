DROP TABLE IF EXISTS File CASCADE;
DROP TABLE IF EXISTS Package CASCADE;
DROP TABLE IF EXISTS Node CASCADE;

CREATE TABLE File(
file_id SERIAL PRIMARY KEY UNIQUE,
pathname TEXT NOT NULL UNIQUE,
type CHAR NOT NULL,
data INT[] DEFAULT '{}' NOT NULL,
size INT DEFAULT 0 NOT NULL,
status BOOL DEFAULT FALSE NOT NULL,
order_num INT DEFAULT 0 NOT NULL
);

CREATE TABLE Node(
node_id SERIAL PRIMARY KEY ,
ip TEXT UNIQUE NOT NULL,
port INT UNIQUE NOT NULL,
status BOOL DEFAULT FALSE NOT NULL
);

CREATE TABLE Package(
pack_id SERIAL PRIMARY KEY ,
node_id INT UNIQUE,
next_package INT UNIQUE,

FOREIGN KEY (next_package) REFERENCES Package (node_id),
FOREIGN KEY (node_id) REFERENCES Node (node_id)
);