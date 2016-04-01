
//  system.h
//  testSystemLab5
//
//  Created by Admin on 31.03.16.
//  Copyright © 2016 Yawning Turtles. All rights reserved.
//

#ifndef system_h
#define system_h

#include <stdio.h>
#include <sqlite3.h>

#define FALSE 0
#define TRUE 1
typedef int BOOL;
sqlite3* get_db();
const static char* CHECK_USER_TYPE_REQUEST = "SELECT USER_TYPE.type FROM USER_TYPE JOIN BANK_USERS ON BANK_USERS.type_id = USER_TYPE.id WHERE BANK_USERS.login = @login AND BANK_USERS.password = @password;";
const static char* UPDATE_DB_DATE_REQUEST = "UPDATE BANK_CONFIG SET value = @date WHERE param = \"last_update\";";
const static char* BANK_CONFIG_DATA_REQUEST = "SELECT value FROM BANK_CONFIG WHERE param = @param;";
const static char* LOCK_ACCOUNTS_REQUEST = "UPDATE BANK_ACCOUNTS SET is_locked = CASE WHEN balance < 0 OR (SELECT OVERDRAFTS.credit_sum FROM OVERDRAFTS WHERE OVERDRAFTS.id = overdraft_id) > CAST((SELECT BANK_CONFIG.value FROM BANK_CONFIG WHERE param = \"overdraft_max_sum\") AS Integer) THEN 1 ELSE 0 END;";
const static char* CHECK_CLIENT_REQUEST = "SELECT * FROM BANK_CLIENTS WHERE passport_number = @pasport_number;";
const static char* CHECK_ACCOUNT_REQUEST = "SELECT * FROM BANK_ACCOUNTS WHERE account_number = @account_number;";
const static char* ADD_ACCOUNT_REQUEST = "INSERT INTO BANK_ACCOUNTS(account_number, client_id, type_id)\
VALUES (@account_number, (SELECT id FROM BANK_CLIENTS WHERE passport_number = @pasport_number), (SELECT id FROM \
ACCOUNT_TYPE WHERE TYPE = @account_type) );";
const static char* ADD_CLIENT_REQUEST = "INSERT INTO BANK_CLIENTS(first_name, last_name, passport_number) VALUES (@firstname, @lastname, @pasport_number)";
const static char* UPDATE_REMOVE_OVERDRAFT_REQUEST	= "UPDATE BANK_ACCOUNTS SET overdraft_id = NULL WHERE account_number = @account_number;";
const static char* ADD_OVERDRAFT_REQUEST = "INSERT INTO OVERDRAFTS(credit_date, limit_date) VALUES (@credit_date, @limit_date);";
const static char* UPDATE_OVERDRAFT_REQUEST = "UPDATE BANK_ACCOUNTS SET overdraft_id = (SELECT id FROM OVERDRAFTS ORDER BY id DESC LIMIT 1) \
WHERE account_number = @account_number;";
const static char* CHANGE_ACCOUNT_TYPE_REQUEST = "UPDATE BANK_ACCOUNTS SET type_id = (SELECT id FROM ACCOUNT_TYPE WHERE `type` = @account_type), \
overdraft_id = NULL WHERE account_number = @account_number;";
const static char* REMOVE_OVERDRAFT_REQUEST	= "DELETE FROM OVERDRAFTS WHERE id = (SELECT overdraft_id FROM BANK_ACCOUNTS WHERE account_number = @account_number);";
const static char* CLOSE_ACCOUNT_REQUEST	= "DELETE FROM BANK_ACCOUNTS WHERE account_number = @account_number;";
const static char* REMOVE_CLIENT_OVERDRAFT_REQUEST	= "DELETE FROM OVERDRAFTS WHERE (SELECT passport_number FROM BANK_CLIENTS WHERE id = \
(SELECT client_id FROM BANK_ACCOUNTS WHERE overdraft_id = OVERDRAFTS.id)) = @pasport_number;";
const static char* REMOVE_CLIENT_ACCOUNT_REQUEST	= "DELETE FROM BANK_ACCOUNTS WHERE client_id = (SELECT id FROM BANK_CLIENTS WHERE passport_number = @pasport_number);";
const static char* REMOVE_CLIENT_REQUEST = "DELETE FROM BANK_CLIENTS WHERE passport_number = @pasport_number;";
const static char* GET_FULL_ACCOUNT_INFORMATION_REQUEST = "SELECT BANK_ACCOUNTS.id, ACCOUNT_TYPE.type, BANK_ACCOUNTS.balance, BANK_ACCOUNTS.total_transactions, \
BANK_ACCOUNTS.account_number, OVERDRAFTS.id, OVERDRAFTS.credit_sum, BANK_CLIENTS.id, BANK_ACCOUNTS.is_locked \
FROM BANK_CLIENTS JOIN BANK_ACCOUNTS ON BANK_ACCOUNTS.client_id = BANK_CLIENTS.id JOIN ACCOUNT_TYPE \
ON BANK_ACCOUNTS.type_id = ACCOUNT_TYPE.id LEFT JOIN OVERDRAFTS ON BANK_ACCOUNTS.overdraft_id = OVERDRAFTS.id WHERE BANK_ACCOUNTS.account_number = @account_number AND BANK_CLIENTS.passport_number = @pasport_number;";
const static char* UPDATE_ACCOUNT_BALANCE_REQUEST = "UPDATE BANK_ACCOUNTS SET balance = @balance WHERE id = @id;";
const static char* UPDATE_OVERDRAFT_CREDIT_SUM_REQUEST = "UPDATE OVERDRAFTS SET credit_sum = @credit_sum WHERE id = @id;";
const static char* UPDATE_OVERDRAFT_INFO_REQUEST = "UPDATE OVERDRAFTS SET credit_sum = @credit_sum, credit_date = @credit_date, limit_date = @limitdate WHERE id = @id;";
const static char* UPDATE_TOTAL_TRANSACTIONS = "UPDATE BANK_ACCOUNTS SET total_transactions = @total_transactions WHERE id = @id;";
const static char* PAY_OVERDRAFT_FEE_REQUEST = "UPDATE OVERDRAFTS SET credit_sum = credit_sum + CAST((SELECT BANK_CONFIG.value FROM BANK_CONFIG WHERE param = \"overdraft_fee\") as Integer) WHERE credit_sum > 0 AND limit_date < (SELECT BANK_CONFIG.value FROM BANK_CONFIG WHERE param = \"last_update\");";
const static char* INTEREST_RATE_PAY_REQUEST = "UPDATE BANK_ACCOUNTS SET balance = CAST((balance * CAST((SELECT BANK_CONFIG.value FROM BANK_CONFIG WHERE param = \"interest_rate\") AS Double)) AS Integer) WHERE balance > 0;";
const static char* PAY_TRANSACTION_FEE_REQUEST = "UPDATE BANK_ACCOUNTS SET balance = balance - (CAST((SELECT BANK_CONFIG.value FROM BANK_CONFIG WHERE param = \"per_transaction_fee\") AS Integer) * (total_transactions - CAST((SELECT BANK_CONFIG.value FROM BANK_CONFIG WHERE param = \"monthly_quota\") AS Integer))), total_transactions = 0 WHERE total_transactions > CAST((SELECT BANK_CONFIG.value FROM BANK_CONFIG WHERE param = \"monthly_quota\") AS Integer);";
const static char* CHECK_NEGATIVE_BALANCES_OVERDRAFT_REQUEST = "UPDATE OVERDRAFTS SET limit_date = CASE WHEN credit_sum = 0 THEN @limit_date ELSE limit_date END, credit_date = CASE WHEN credit_sum = 0 THEN (SELECT BANK_CONFIG.value FROM BANK_CONFIG WHERE BANK_CONFIG.param = \"last_update\") ELSE limit_date END, credit_sum = credit_sum - (SELECT BANK_ACCOUNTS.balance FROM BANK_ACCOUNTS WHERE BANK_ACCOUNTS.overdraft_id = id) WHERE (SELECT BANK_ACCOUNTS.balance FROM BANK_ACCOUNTS WHERE BANK_ACCOUNTS.overdraft_id = id) < 0;";
const static char* CHECK_NEGATIVE_BALANCES_ACCOUNT_REQUEST = "UPDATE BANK_ACCOUNTS SET balance = 0 WHERE balance < 0 AND overdraft_id IS NOT NULL;";

typedef struct _date
{
    int day;
    int month;
    int year;
} date;

typedef enum _user_type
{
    OPERATOR,
    ADMINISTRATOR,
    NOT_EXIST
} user_type;

typedef enum _account_type
{
    CURRENT,
    SAVING
} account_type;

typedef struct _config
{
    double interest_rate;
    int interest_rate_period;
    int overdraft_fee;
    int monthly_quota;
    int per_transaction_fee;
    int overdraft_max_sum;
    int overdraft_max_period;
    date last_update;
} bank_config;


extern date current_date;
extern sqlite3 *db;
extern bank_config config;

BOOL start();
void init_current_date();
void init_bank_config();
void update_db_date();
void transaction_fee_pay();
void overdraft_fee_pay();
void interest_rate_pay();
void finish();

void lock_negative_accounts();
void check_negative_balances();
void get_string_date(char* buffer, int size, date d);
void get_string_current_date(char* buffer, int size);
date parse_date(char* d);
user_type authorization(char* login, char* password);

#endif /* system_h */
