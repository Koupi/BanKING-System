-- Created by Vertabelo (http://vertabelo.com)
-- Last modification date: 2016-03-21 17:22:05.86



-- tables
-- Table: ACCOUNT_TYPE
CREATE TABLE ACCOUNT_TYPE (
    id integer  NOT NULL   PRIMARY KEY  AUTOINCREMENT,
    type text  NOT NULL,
    CONSTRAINT ACCOUNT_TYPE_ak_1 UNIQUE (type)
);

-- Table: BANK_ACCOUNTS
CREATE TABLE BANK_ACCOUNTS (
    id integer  NOT NULL   PRIMARY KEY  AUTOINCREMENT,
    type_id integer  NOT NULL,
    balance integer  NOT NULL,
    total_transactions integer  NOT NULL,
    overdraft_id integer,
    client_id integer  NOT NULL,
    account_number integer  NOT NULL,
    CONSTRAINT BANK_ACCOUNTS_ak_1 UNIQUE (account_number),
    FOREIGN KEY (type_id) REFERENCES ACCOUNT_TYPE (id),
    FOREIGN KEY (client_id) REFERENCES BANK_CLIENTS (id),
    FOREIGN KEY (overdraft_id) REFERENCES OVERDRAFTS (id)
);

-- Table: BANK_CLIENTS
CREATE TABLE BANK_CLIENTS (
    id integer  NOT NULL   PRIMARY KEY  AUTOINCREMENT,
    first_name text  NOT NULL,
    last_name text  NOT NULL,
    passport_number text  NOT NULL,
    CONSTRAINT BANK_CLIENTS_ak_1 UNIQUE (passport_number)
);

-- Table: BANK_CONFIG
CREATE TABLE BANK_CONFIG (
    param text  NOT NULL   PRIMARY KEY,
    value text  NOT NULL
);

-- Table: BANK_USERS
CREATE TABLE BANK_USERS (
    id integer  NOT NULL   PRIMARY KEY  AUTOINCREMENT,
    login text  NOT NULL,
    password text  NOT NULL,
    type_id integer  NOT NULL,
    first_name text  NOT NULL,
    last_name text  NOT NULL,
    FOREIGN KEY (type_id) REFERENCES USER_TYPE (id)
);

-- Table: OVERDRAFTS
CREATE TABLE OVERDRAFTS (
    id integer  NOT NULL   PRIMARY KEY  AUTOINCREMENT,
    credit_sum integer  NOT NULL,
    credit_date date  NOT NULL,
    limit_date date  NOT NULL
);

-- Table: USER_TYPE
CREATE TABLE USER_TYPE (
    id integer  NOT NULL   PRIMARY KEY  AUTOINCREMENT,
    type text  NOT NULL,
    CONSTRAINT USER_TYPE_ak_1 UNIQUE (type)
);





-- End of file.

