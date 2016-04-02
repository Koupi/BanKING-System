//
//  operator.c
//  BanKING-System
//
//  Created by Admin on 27.03.16.
//  Copyright © 2016 Yawning Turtles. All rights reserved.
//

#include "operator.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "system.h"

#include <stdio.h>
#include <sqlite3.h>

BOOL enroll(int sum, int account_number, char* passport_number)
{
    // Getting full info about account
    sqlite3_stmt *pStmt;
    int indx = 0;
    int rc = sqlite3_prepare_v2(db, GET_FULL_ACCOUNT_INFORMATION_REQUEST, -1, &pStmt, 0);
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
    
    if(!write_text_not_null("@pasport_number", passport_number, pStmt))
    {
        printf("Faild creating passport number\n");
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
    lock_negative_accounts();
    return TRUE;
}

BOOL withdraw(int sum, int account_number, char* passport_number)
{
    // Getting full info about account
    sqlite3_stmt *pStmt;
    int indx = 0;
    int rc = sqlite3_prepare_v2(db, GET_FULL_ACCOUNT_INFORMATION_REQUEST, -1, &pStmt, 0);
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
    
    if(!write_text_not_null("@pasport_number", passport_number, pStmt))
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
    BOOL check = FALSE;
    sqlite3_finalize(pStmt);
    if(is_locked)
    {
        printf("Account is locked!");
        return FALSE;
    }
    // Calculating sum
    if(balance < sum)
    {
        if(credit_sum == 0)
        {
            check = TRUE;
        }
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
    if(check)
    {
        rc = sqlite3_prepare_v2(db, UPDATE_OVERDRAFT_INFO_REQUEST, -1, &pStmt, 0);
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
        int size = 11;
        char *current_date = (char*)malloc(sizeof(char)*(size));
        get_string_current_date(current_date, size);
        date current_date_struct = parse_date(current_date);
        if(!write_text_not_null("@credit_date", current_date, pStmt))
        {
            printf("Faild creating credit date\n");
            sqlite3_finalize(pStmt);
            return FALSE;
        }
        date limit_date_struct;
        if(current_date_struct.month == 12)
        {
            limit_date_struct.month = 1;
            limit_date_struct.year = current_date_struct.year + 1;
        }
        else
        {
            limit_date_struct.month = current_date_struct.month + 1;
            limit_date_struct.year = current_date_struct.year;
        }
        if(current_date_struct.day > 28)
        {
            limit_date_struct.day = 28;
        }
        else
        {
            limit_date_struct.day = current_date_struct.day;
        }
        char *limit_date = (char*)malloc(sizeof(char)*(size*5));
        get_string_date(limit_date, size, limit_date_struct);
        if(!write_text_not_null("@limit_date", limit_date, pStmt))
        {
            printf("Faild creating limit date\n");
            sqlite3_finalize(pStmt);
            return FALSE;
        }
        rc = sqlite3_step(pStmt);
        sqlite3_finalize(pStmt);
    }
    else
    {
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
    }
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
    lock_negative_accounts();
    return TRUE;
}

BOOL transfer(int sum, int transfering_from_account_number, int transfering_to_account_number, char* transfering_from_passport_number, char* transfering_to_passport_number)
{
    // Getting full info about transfering from account
    sqlite3_stmt *pStmt;
    int indx = 0;
    int rc = sqlite3_prepare_v2(db, GET_FULL_ACCOUNT_INFORMATION_REQUEST, -1, &pStmt, 0);
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
    
    if(!write_text_not_null("@pasport_number", transfering_from_passport_number, pStmt))
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
    rc = sqlite3_prepare_v2(db, GET_FULL_ACCOUNT_INFORMATION_REQUEST, -1, &pStmt, 0);
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
    
    if(!write_text_not_null("@pasport_number", transfering_to_passport_number, pStmt))
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
    BOOL check = FALSE;
    sqlite3_finalize(pStmt);
    
    if(balance_from < sum)
    {
        if(credit_sum_from == 0)
        {
            check = TRUE;
        }
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
    if(check)
    {
        rc = sqlite3_prepare_v2(db, UPDATE_OVERDRAFT_INFO_REQUEST, -1, &pStmt, 0);
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
        int size = 11;
        char *current_date = (char*)malloc(sizeof(char)*(size));
        get_string_current_date(current_date, size);
        date current_date_struct = parse_date(current_date);
        if(!write_text_not_null("@credit_date", current_date, pStmt))
        {
            printf("Faild creating\n");
            sqlite3_finalize(pStmt);
            return FALSE;
        }
        date limit_date_struct;
        if(current_date_struct.month == 12)
        {
            limit_date_struct.month = 1;
            limit_date_struct.year = current_date_struct.year + 1;
        }
        else
        {
            limit_date_struct.month = current_date_struct.month + 1;
            limit_date_struct.year = current_date_struct.year;
        }
        if(current_date_struct.day > 28)
        {
            limit_date_struct.day = 28;
        }
        else
        {
            limit_date_struct.day = current_date_struct.day;
        }
        char *limit_date = (char*)malloc(sizeof(char)*(size));
        get_string_date(limit_date, size, limit_date_struct);
        if(!write_text_not_null("@limit_date", limit_date, pStmt))
        {
            printf("Faild creating\n");
            sqlite3_finalize(pStmt);
            return FALSE;
        }
        rc = sqlite3_step(pStmt);
        sqlite3_finalize(pStmt);
    }
    else
    {
        
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
    }
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
    lock_negative_accounts();
    return TRUE;
}

void enroll_dialog()
{
    int account_number, sum;
    char *passport_number = (char*)malloc(sizeof(char)*(20));
    fpurge(stdin);
    printf("Enter an account number:\n");
    scanf("%d", &account_number);
    fpurge(stdin);
    printf("Enter a passport number:\n");
    scanf("%s", passport_number);
    fpurge(stdin);
    printf("Enter a sum to enroll:\n");
    scanf("%d", &sum);
    
    if(enroll(sum, account_number, passport_number))
    {
        printf("Enrolled successfully.\n");
    }
    else
    {
        printf("Error!\n");
    }
    free(passport_number);
}

void withdraw_dialog()
{
    int account_number, sum;
    char *passport_number = (char*)malloc(sizeof(char)*(20));
    fpurge(stdin);
    printf("Enter an account number:\n");
    scanf("%d", &account_number);
    fpurge(stdin);
    printf("Enter a passport number:\n");
    scanf("%s", passport_number);
    fpurge(stdin);
    printf("Enter a sum to withdraw:\n");
    scanf("%d", &sum);
    if(withdraw(sum, account_number, passport_number))
    {
        printf("Withdrawed successfully.\n");
    }
    else
    {
        printf("Error!\n");
    }
    free(passport_number);
}

void transfer_dialog()
{
    int transfering_from_account_number, transfering_to_account_number, sum;
    char *passport_transfering_from_number = (char*)malloc(sizeof(char)*(20));
    char *passport_transfering_to_number = (char*)malloc(sizeof(char)*(20));
    fpurge(stdin);
    printf("Enter an account number you want to transfer from :\n");
    scanf("%d", &transfering_from_account_number);
    fpurge(stdin);
    printf("Enter this account owner's passport number:\n");
    scanf("%s", passport_transfering_from_number);
    fpurge(stdin);
    printf("Enter an account number you want to transfer to:\n");
    scanf("%d", &transfering_to_account_number);
    fpurge(stdin);
    printf("Enter this account owner's passport number:\n");
    scanf("%s", passport_transfering_to_number);
    fpurge(stdin);
    printf("Enter a sum to transfer:\n");
    scanf("%d", &sum);
    if(transfer(sum, transfering_from_account_number, transfering_to_account_number, passport_transfering_from_number, passport_transfering_to_number))
    {
        printf("Transfered successfully.\n");
    }
    else
    {
        printf("Error!\n");
    }
}