//
//  administrator.h
//  BanKING-System
//
//  Created by Admin on 27.03.16.
//  Copyright © 2016 Yawning Turtles. All rights reserved.
//

#ifndef administrator_h
#define administrator_h

#include <stdio.h>
#include <sqlite3.h>
typedef unsigned short BOOL;
#define TRUE 1
#define FALSE 0
sqlite3 *db;
typedef enum _account_type {CURRENT,SAVING} account_type;
const char* CHECK_ACCOUNT_REQUEST = "SELECT * FROM BANK_ACCOUNTS WHERE account_number = @account_number;";
const char* ADD_ACCOUNT_REQUEST = "INSERT INTO BANK_ACCOUNTS(account_number, client_id, type_id)\
VALUES (@account_number, (SELECT id FROM BANK_CLIENTS WHERE passport_number = @pasport_number), (SELECT id FROM \
ACCOUNT_TYPE WHERE TYPE = @account_type) );";

const char* ADD_CLIENT_REQUEST = "INSERT INTO BANK_CLIENTS(first_name, last_name, passport_number) VALUES (@firstname, @lastname, @pasport_number)";

const char* ADD_OVERDRAFT_REQUEST = "INSERT INTO OVERDRAFTS(credit_date, limit_date) VALUES (@credit_date, @limit_date);";
const char* UPDATE_OVERDRAFT_REQUEST = "UPDATE BANK_ACCOUNTS SET overdraft_id = (SELECT id FROM OVERDRAFTS ORDER BY id DESC LIMIT 1) \
WHERE account_number = @account_number;";

const char* REMOVE_OVERDRAFT_REQUEST	= "DELETE FROM OVERDRAFTS WHERE id = (SELECT overdraft_id FROM BANK_ACCOUNTS WHERE account_number = @account_number);";
const char* CLOSE_ACCOUNT_REQUEST	= "DELETE FROM BANK_ACCOUNTS WHERE account_number = @account_number;";

const char* UPDATE_REMOVE_OVDERDRAFT_REQUEST	= "DELETE FROM OVERDRAFTS WHERE id = (SELECT overdraft_id FROM BANK_ACCOUNTS WHERE account_number = @account_number); \
UPDATE BANK_ACCOUNTS SET overdraft_id = NULL WHERE account_number = @account_number;";

const char* REMOVE_CLIENT_OVERDRAFT_REQUEST	= "DELETE FROM OVERDRAFTS WHERE (SELECT passport_number FROM BANK_CLIENTS WHERE id = \
(SELECT client_id FROM BANK_ACCOUNTS WHERE overdraft_id = OVERDRAFTS.id)) = @pasport_number;";
const char* REMOVE_CLIENT_ACCOUNT_REQUEST	= "DELETE FROM BANK_ACCOUNTS WHERE client_id = (SELECT id FROM BANK_CLIENTS WHERE passport_number = @pasport_number);";
const char* REMOVE_CLIENT_REQUEST = "DELETE FROM BANK_CLIENTS WHERE passport_number = @pasport_number;";

const char* GET_FULL_ACCOUNT_INFORMATION_REQUEST = "SELECT BANK_ACCOUNTS.id, ACCOUNT_TYPE.type, BANK_ACCOUNTS.balance, BANK_ACCOUNTS.total_transactions, \
BANK_ACCOUNTS.account_number, OVERDRAFTS.id, OVERDRAFTS.credit_sum, BANK_CLIENTS.id, BANK_ACCOUNTS.is_locked \
FROM BANK_CLIENTS JOIN BANK_ACCOUNTS ON BANK_ACCOUNTS.client_id = BANK_CLIENTS.id JOIN ACCOUNT_TYPE \
ON BANK_ACCOUNTS.type_id = ACCOUNT_TYPE.id LEFT JOIN OVERDRAFTS ON BANK_ACCOUNTS.overdraft_id = OVERDRAFTS.id WHERE BANK_ACCOUNTS.account_number = @account_number AND BANK_CLIENTS.passport_number = @pasport_number;";

const char* CHANGE_ACCOUNT_TYPE_REQUEST = "DELETE FROM OVERDRAFTS WHERE id = (SELECT overdraft_id FROM BANK_ACCOUNTS WHERE account_number = @account_number); \
UPDATE BANK_ACCOUNTS SET type_id = (SELECT id FROM ACCOUNT_TYPE WHERE `type` = @account_type), \
overdraft_id = NULL WHERE account_number = @account_number;";
void add_client_dialog();
void remove_client_dialog();
void add_account_dialog();
void close_account_dialog();
void account_management_dialog();
#endif
