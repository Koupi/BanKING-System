//
//  operator.h
//  BanKING-System
//
//  Created by Admin on 27.03.16.
//  Copyright © 2016 Yawning Turtles. All rights reserved.
//

#ifndef operator_h
#define operator_h

#include <stdio.h>
#include <sqlite3.h>

typedef unsigned short BOOL;
#define TRUE 1
#define FALSE 0

sqlite3 *db;

const char* GET_FULL_ACCOUNT_INFORMATION_REQUEST = "SELECT BANK_ACCOUNTS.id, ACCOUNT_TYPE.type, BANK_ACCOUNTS.balance, BANK_ACCOUNTS.total_transactions, BANK_ACCOUNTS.account_number, OVERDRAFTS.id, OVERDRAFTS.credit_sum, BANK_CLIENTS.id, BANK_ACCOUNTS.is_locked FROM BANK_CLIENTS JOIN BANK_ACCOUNTS ON BANK_ACCOUNTS.client_id = BANK_CLIENTS.id JOIN ACCOUNT_TYPE ON BANK_ACCOUNTS.type_id = ACCOUNT_TYPE.id LEFT JOIN OVERDRAFTS ON BANK_ACCOUNTS.overdraft_id = OVERDRAFTS.id WHERE BANK_ACCOUNTS.account_number = @account_number AND BANK_CLIENTS.passport_number = @passport_number;";

const char* LOCK_ACCOUNT_REQUEST = "UPDATE BANK_ACCOUNTS SET is_locked = 0 WHERE id = @id;";

const char* UPDATE_ACCOUNT_BALANCE_REQUEST = "UPDATE BANK_ACCOUNTS SET balance = @balance WHERE id = @id;";

const char* UPDATE_OVERDRAFT_CREDIT_SUM_REQUEST = "UPDATE OVERDRAFTS SET credit_sum = @credit_sum WHERE id = @id;";

const char* UPDATE_OVERDRAFT_INFO_REQUEST = "UPDATE OVERDRAFTS SET credit_sum = @credit_sum, credit_date = @credit_date, limit_date = @limit_date WHERE id = @id;";

const char* UPDATE_TOTAL_TRANSACTIONS = "UPDATE BANK_ACCOUNTS SET total_transactions = @total_transactions WHERE id = @id;";

void enroll_dialog();
void withdraw_dialog();
void transfer_dialog();

#endif
