//
//  operator.c
//  BanKING-System
//
//  Created by Admin on 27.03.16.
//  Copyright © 2016 Yawning Turtles. All rights reserved.
//

#include "operator.h"
#include "sqlite3.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
//#include "system.h"

BOOL write_text_not_null(char* param, char* value, sqlite3_stmt *pStmt)
{
    int indx = sqlite3_bind_parameter_index(pStmt, param);
    if(value[0]!='0')
    {
        if(sqlite3_bind_text (pStmt,indx,value, strlen(value), SQLITE_STATIC)!= SQLITE_OK)
        {
            printf("Failed binding %s\n", sqlite3_errmsg(db));
            return FALSE;
        }
    }
    return TRUE;
}

BOOL enroll(int sum, int account_number, char* passport_number)
{
    // Getting full info about account
    sqlite3_stmt *pStmt;
    int indx = 0;
    int rc = sqlite3_prepare_v2(db, FULL_INFO_ABOUT_ACCOUNT_REQUEST, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@account_number");
    if(	sqlite3_bind_int(pStmt, indx, account_number)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    if(!write_text_not_null("@passport_number", passport_number, pStmt))
    {
        printf("Faild creating\n");
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    rc = sqlite3_step(pStmt);
    if (rc != SQLITE_ROW)
    {
        sqlite3_finalize(pStmt);
        printf("No such account with this client: %d\n", account_number);
        return FALSE;
    }
    
    int balance = sqlite3_column_int(pStmt, 2);
    int credit_sum = sqlite3_column_int(pStmt, 6);
    int overdraft_id = sqlite3_column_int(pStmt, 5);
    int account_id = sqlite3_column_int(pStmt, 0);
    int transaction_counter = sqlite3_column_int(pStmt, 3);
    sqlite3_finalize(pStmt);
    // Calculating sum and credit sum
    if(credit_sum > 0)
    {
        if(credit_sum < sum)
        {
            int difference = sum - credit_sum;
            credit_sum = 0;
            balance += difference;
        }
        else
        {
            credit_sum -= sum;
        }
    }
    else
    {
        balance += sum;
    }
    // Updating account balance
    rc = sqlite3_prepare_v2(db, UPDATE_ACCOUNT_BALANCE_REQUEST, -1, &pStmt, 0);
    
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@balance");
    if(	sqlite3_bind_int(pStmt, indx, balance)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@id");
    if(	sqlite3_bind_int(pStmt, indx, account_id)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    rc = sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    // Updating overdraft credit sum
    rc = sqlite3_prepare_v2(db, UPDATE_OVERDRAFT_CREDIT_SUM_REQUEST, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@credit_sum");
    if(	sqlite3_bind_int(pStmt, indx, credit_sum)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@id");
    if(	sqlite3_bind_int(pStmt, indx, overdraft_id)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    rc = sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    // Updating total transactions
    transaction_counter++;
    rc = sqlite3_prepare_v2(db, UPDATE_TOTAL_TRANSACTIONS, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@total_transactions");
    if(	sqlite3_bind_int(pStmt, indx, transaction_counter)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@id");
    if(	sqlite3_bind_int(pStmt, indx, account_id)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    rc = sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    //  lock_negative_accounts();
    return TRUE;
}

BOOL withdraw(int sum, int account_number, char* passport_number)
{
    // Getting full info about account
    sqlite3_stmt *pStmt;
    int indx = 0;
    int rc = sqlite3_prepare_v2(db, FULL_INFO_ABOUT_ACCOUNT_REQUEST, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@account_number");
    if(	sqlite3_bind_int(pStmt, indx, account_number)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    if(!write_text_not_null("@passport_number", passport_number, pStmt))
    {
        printf("Faild creating\n");
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    rc = sqlite3_step(pStmt);
    if (rc != SQLITE_ROW)
    {
        sqlite3_finalize(pStmt);
        printf("No such account with this client: %d\n", account_number);
        return FALSE;
    }
    int balance = sqlite3_column_int(pStmt, 2);
    int account_id = sqlite3_column_int(pStmt, 0);
    int credit_sum = sqlite3_column_int(pStmt, 6);
    int overdraft_id = sqlite3_column_int(pStmt, 5);
    int transaction_counter = sqlite3_column_int(pStmt, 3);
    BOOL is_locked = sqlite3_column_int(pStmt, 8);
    sqlite3_finalize(pStmt);
    if(is_locked)
    {
        printf("Account is locked!");
        return FALSE;
    }
    // Calculating sum
    if(balance < sum)
    {
        int difference = sum - balance;
        balance = 0;
        credit_sum += difference;
    }
    else
    {
        balance -= sum;
    }
    // Updating account balance
    rc = sqlite3_prepare_v2(db, UPDATE_ACCOUNT_BALANCE_REQUEST, -1, &pStmt, 0);
    
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@balance");
    if(	sqlite3_bind_int(pStmt, indx, balance)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@id");
    if(	sqlite3_bind_int(pStmt, indx, account_id)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    rc = sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    // Updating overdraft credit sum
    rc = sqlite3_prepare_v2(db, UPDATE_OVERDRAFT_CREDIT_SUM_REQUEST, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@credit_sum");
    if(	sqlite3_bind_int(pStmt, indx, credit_sum)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@id");
    if(	sqlite3_bind_int(pStmt, indx, overdraft_id)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    rc = sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    // Updating total transactions
    transaction_counter++;
    rc = sqlite3_prepare_v2(db, UPDATE_TOTAL_TRANSACTIONS, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@total_transactions");
    if(	sqlite3_bind_int(pStmt, indx, transaction_counter)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@id");
    if(	sqlite3_bind_int(pStmt, indx, account_id)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    rc = sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    //  lock_negative_accounts();
    return TRUE;
}

BOOL transfer(int sum, int transfering_from_account_number, int transfering_to_account_number, char* transfering_from_passport_number, char* transfering_to_passport_number)
{
    // Getting full info about transfering from account
    sqlite3_stmt *pStmt;
    int indx = 0;
    int rc = sqlite3_prepare_v2(db, FULL_INFO_ABOUT_ACCOUNT_REQUEST, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@account_number");
    if(	sqlite3_bind_int(pStmt, indx, transfering_from_account_number)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    if(!write_text_not_null("@passport_number", transfering_from_passport_number, pStmt))
    {
        printf("Faild creating\n");
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    rc = sqlite3_step(pStmt);
    if (rc != SQLITE_ROW)
    {
        sqlite3_finalize(pStmt);
        printf("No such account with this client: %s\n", transfering_from_passport_number);
        return FALSE;
    }
    int balance_from = sqlite3_column_int(pStmt, 2);
    int account_from_id = sqlite3_column_int(pStmt, 0);
    int credit_sum_from = sqlite3_column_int(pStmt, 6);
    int overdraft_id_from = sqlite3_column_int(pStmt, 5);
    int transaction_from_counter = sqlite3_column_int(pStmt, 3);
    BOOL is_locked = sqlite3_column_int(pStmt, 8);
    sqlite3_finalize(pStmt);
    if(is_locked)
    {
        printf("Account is locked!");
        return FALSE;
    }
    // Getting full info about transfering to account
    indx = 0;
    rc = sqlite3_prepare_v2(db, FULL_INFO_ABOUT_ACCOUNT_REQUEST, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@account_number");
    if(	sqlite3_bind_int(pStmt, indx, transfering_to_account_number)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    if(!write_text_not_null("@passport_number", transfering_to_passport_number, pStmt))
    {
        printf("Faild creating\n");
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    rc = sqlite3_step(pStmt);
    if (rc != SQLITE_ROW)
    {
        sqlite3_finalize(pStmt);
        printf("No such account with this client: %s\n", transfering_to_passport_number);
        return FALSE;
    }
    int balance_to = sqlite3_column_int(pStmt, 2);
    int account_to_id = sqlite3_column_int(pStmt, 0);
    int transaction_to_counter = sqlite3_column_int(pStmt, 3);
    int credit_sum_to = sqlite3_column_int(pStmt, 6);
    int overdraft_id_to = sqlite3_column_int(pStmt, 5);
    sqlite3_finalize(pStmt);
    
    if(balance_from < sum)
    {
        int diff = credit_sum_from - sum;
        balance_from = 0;
        credit_sum_from += diff;
    }
    else
    {
        balance_from -= sum;
    }
    // Calculating sum and credit sum
    if(credit_sum_to > 0)
    {
        if(credit_sum_to < sum)
        {
            int difference = sum - credit_sum_to;
            credit_sum_to = 0;
            balance_to += difference;
        }
        else
        {
            credit_sum_to -= sum;
        }
    }
    else
    {
        balance_to += sum;
    }
    // Updating account_from balance
    rc = sqlite3_prepare_v2(db, UPDATE_ACCOUNT_BALANCE_REQUEST, -1, &pStmt, 0);
    
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@balance");
    if(	sqlite3_bind_int(pStmt, indx, balance_from)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@id");
    if(	sqlite3_bind_int(pStmt, indx, account_from_id)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    rc = sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    // Updating account_to balance
    rc = sqlite3_prepare_v2(db, UPDATE_ACCOUNT_BALANCE_REQUEST, -1, &pStmt, 0);
    
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@balance");
    if(	sqlite3_bind_int(pStmt, indx, balance_to)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@id");
    if(	sqlite3_bind_int(pStmt, indx, account_to_id)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    rc = sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    // Updating credit_sum_from
    rc = sqlite3_prepare_v2(db, UPDATE_OVERDRAFT_CREDIT_SUM_REQUEST, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@credit_sum");
    if(	sqlite3_bind_int(pStmt, indx, credit_sum_from)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@id");
    if(	sqlite3_bind_int(pStmt, indx, overdraft_id_from)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    rc = sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    // Updating credit_sum_to
    rc = sqlite3_prepare_v2(db, UPDATE_OVERDRAFT_CREDIT_SUM_REQUEST, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@credit_sum");
    if(	sqlite3_bind_int(pStmt, indx, credit_sum_to)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@id");
    if(	sqlite3_bind_int(pStmt, indx, overdraft_id_to)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    rc = sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    // Updating total transactions
    transaction_to_counter++;
    rc = sqlite3_prepare_v2(db, UPDATE_TOTAL_TRANSACTIONS, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@total_transactions");
    if(	sqlite3_bind_int(pStmt, indx, transaction_to_counter)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@id");
    if(	sqlite3_bind_int(pStmt, indx, account_to_id)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    rc = sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    // Updating total transactions
    transaction_from_counter++;
    rc = sqlite3_prepare_v2(db, UPDATE_TOTAL_TRANSACTIONS, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@total_transactions");
    if(	sqlite3_bind_int(pStmt, indx, transaction_from_counter)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    
    indx = sqlite3_bind_parameter_index(pStmt, "@id");
    if(	sqlite3_bind_int(pStmt, indx, account_from_id)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        printf("Failed binding");
        return 0;
    }
    rc = sqlite3_step(pStmt);
    sqlite3_finalize(pStmt);
    //  lock_negative_accounts();
    return TRUE;
}
