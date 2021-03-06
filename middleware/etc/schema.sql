DROP TABLE IF EXISTS File CASCADE;
DROP TABLE IF EXISTS Package CASCADE;
DROP TABLE IF EXISTS Node CASCADE;

CREATE TABLE File(
file_id SERIAL PRIMARY KEY UNIQUE,
pathname TEXT NOT NULL UNIQUE,
type CHAR NOT NULL,
data INT[] DEFAULT '{}' NOT NULL,
size INT DEFAULT 0 NOT NULL,
status BOOL DEFAULT TRUE NOT NULL,
order_num INT DEFAULT 0 NOT NULL
);

CREATE TABLE Node(
node_id SERIAL PRIMARY KEY ,
ip TEXT UNIQUE NOT NULL,
tcp_port INT UNIQUE NOT NULL,
udp_port INT UNIQUE NOT NULL
);

CREATE TABLE Package(
pack_id SERIAL PRIMARY KEY ,
node_id INT NOT NULL,
next_package INT DEFAULT NULL,
file_id INT NOT NULL,
status BOOL DEFAULT FALSE NOT NULL

--FOREIGN KEY (next_package) REFERENCES Package (node_id),
--FOREIGN KEY (node_id) REFERENCES Node (node_id),
--FOREIGN KEY (file_id) REFERENCES File (file_id)
);
