CREATE TABLE pokemart (
mart_id INTEGER PRIMARY KEY AUTOINCREMENT,
city VARCHAR(50),
region VARCHAR(50) NOT NULL,
street_address VARCHAR(50) UNIQUE NOT NULL,
phone_num CHAR(8) UNIQUE NOT NULL,
UNIQUE (city, street_address),
UNIQUE (street_address, phone_num));

CREATE TABLE mart_balance_history (
balance_id INTEGER PRIMARY KEY AUTOINCREMENT,
balance NUMERIC(9,3) NOT NULL,
balance_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
mart_id SMALLINT REFERENCES pokemart(mart_id) NOT NULL);

CREATE TABLE trainer_card (
trainer_id INTEGER PRIMARY KEY AUTOINCREMENT,
balance NUMERIC(9,3) DEFAULT 0,
badge_level TINYINT NOT NULL DEFAULT 0,
trainer_fname VARCHAR(20) NOT NULL,
trainer_lname VARCHAR(20) NOT NULL,
trainer_phone CHAR(8),
registration_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
UNIQUE (trainer_fname, trainer_lname, trainer_phone));

CREATE TABLE vendor (
vendor_id INTEGER PRIMARY KEY AUTOINCREMENT,
vendor_name VARCHAR(30) NOT NULL UNIQUE,
vendor_contact VARCHAR(20) NOT NULL UNIQUE,
vendor_phone CHAR(8) NOT NULL UNIQUE,
UNIQUE (vendor_name, vendor_phone),
UNIQUE (vendor_phone, vendor_contact));

CREATE TABLE product (
prod_code VARCHAR(20) PRIMARY KEY,
vendor_id SMALLINT REFERENCES vendor(vendor_id),
prod_name VARCHAR(50) UNIQUE NOT NULL,
prod_descript VARCHAR(50) NOT NULL,
unit_price NUMERIC(6,3) NOT NULL,
min_qty SMALLINT NOT NULL DEFAULT 0,
req_badges SMALLINT NOT NULL DEFAULT 0,
vendor_price NUMERIC(6,3) NOT NULL);

CREATE TABLE employee (
emp_id INTEGER PRIMARY KEY AUTOINCREMENT,
emp_fname VARCHAR(20) NOT NULL,
emp_lname VARCHAR(20) NOT NULL,
emp_phone CHAR(8),
UNIQUE (emp_fname, emp_lname, emp_phone));

CREATE TABLE certification (
cert_id INTEGER PRIMARY KEY AUTOINCREMENT,
cert_payrate NUMERIC(5,3) NOT NULL,
cert_descript VARCHAR(50) NOT NULL,
cert_title VARCHAR(30) NOT NULL,
UNIQUE (cert_title, cert_descript));

CREATE TABLE certification_record (
emp_id INTEGER REFERENCES employee(emp_id) NOT NULL,
cert_id INTEGER REFERENCES certification(cert_id) NOT NULL,
cert_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
PRIMARY KEY (emp_id, cert_id),
UNIQUE (emp_id, cert_id));

CREATE TABLE shift (
shift_id INTEGER PRIMARY KEY AUTOINCREMENT,
emp_id INTEGER REFERENCES employee(emp_id) NOT NULL,
hours_worked NUMERIC(5,3) NOT NULL DEFAULT 0,
mart_id INTEGER NOT NULL,
cert_id INTEGER REFERENCES certification(cert_id) NOT NULL,
shift_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP);

CREATE TABLE invoice (
invoice_num INTEGER PRIMARY KEY AUTOINCREMENT,
trainer_id INTEGER REFERENCES trainer_card(trainer_id) NOT NULL,
emp_id INTEGER REFERENCES employee(emp_id) NOT NULL,
mart_id INTEGER REFERENCES pokemart(mart_id) NOT NULL,
tax_rate NUMERIC(5,3) NOT NULL DEFAULT 0.07,
invoice_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP);

CREATE TABLE line (
invoice_num INTEGER REFERENCES invoice(invoice_num) NOT NULL,
line_num INTEGER NOT NULL,
prod_code VARCHAR(20) REFERENCES product(prod_code) NOT NULL,
qty SMALLINT NOT NULL,
PRIMARY KEY (invoice_num, line_num));

CREATE TABLE stock_history (
stock_id INTEGER PRIMARY KEY AUTOINCREMENT,
prod_code VARCHAR(20) REFERENCES product(prod_code) NOT NULL,
stock_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
mart_id SMALLINT REFERENCES pokemart(mart_id) NOT NULL,
stock_qty SMALLINT NOT NULL);
