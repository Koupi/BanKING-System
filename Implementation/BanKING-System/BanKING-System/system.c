//
//  system.c
//  testSystemLab5
//
//  Created by Admin on 31.03.16.
//  Copyright © 2016 Yawning Turtles. All rights reserved.
//

#include "system.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

sqlite3 *db;
date current_date;
bank_config config;
sqlite3* get_db()
{
    return db;
}
date parse_date(char* d)
{
    date res = {1,1,2016};
    char temp[5];
    char *pdate;
    if(strlen(d) < 10 || !(d[8] >= '0' && d[8] <= '3') || !(d[9] >= '0' && d[9] <= '9') || d[7] != '-'
       || !(d[5] >= '0' && d[5] <= '1') || !(d[6] >= '0' && d[6] <= '9') || d[4] != '-'
       || !(d[0] >= '0' && d[0] <= '2') || !(d[1] >= '0' && d[1] <= '9') || !(d[2] >= '0' && d[2] <= '9') || !(d[3] >= '0' && d[3] <= '9'))
    {
        return res;
    }
    pdate = d;
    strncpy(temp, pdate, 4);
    res.year = atoi(temp);
    pdate += 5;
    memset(temp, 0, 5);
    strncpy(temp, pdate, 2);
    res.month = atoi(temp);
    pdate += 3;
    memset(temp, 0, 5);
    strncpy(temp, pdate, 2);
    res.day = atoi(temp);
    return res;
}

BOOL init_bank_config_int_elem(int *dest, char* param)
{
    int rc;
    sqlite3_stmt* res;
    rc = sqlite3_prepare_v2(db, BANK_CONFIG_DATA_REQUEST, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
        return FALSE;
    }
    int idx = sqlite3_bind_parameter_index(res, "@param");
    sqlite3_bind_text(res, idx, param, (int)strlen(param), SQLITE_STATIC);
    rc = sqlite3_step(res);
    if(rc != SQLITE_ROW)
    {
        sqlite3_finalize(res);
        return FALSE;
    }
    *dest = atoi(sqlite3_column_text(res, 0));
    sqlite3_finalize(res);
    return TRUE;
}
void init_bank_config()
{
    int rc;
    sqlite3_stmt *res;
    if(!init_bank_config_int_elem(&config.interest_rate_period, "interest_rate_period"))
    {
        config.interest_rate_period = 1;
    }
    if(!init_bank_config_int_elem(&config.overdraft_fee, "overdraft_fee"))
    {
        config.overdraft_fee = 2;
    }
    if(!init_bank_config_int_elem(&config.monthly_quota, "monthly_quota"))
    {
        config.monthly_quota = 100;
    }
    if(!init_bank_config_int_elem(&config.per_transaction_fee, "per_transaction_fee"))
    {
        config.per_transaction_fee = 1;
    }
    if(!init_bank_config_int_elem(&config.overdraft_max_sum, "overdraft_max_sum"))
    {
        config.overdraft_max_sum = 200;
    }
    if(!init_bank_config_int_elem(&config.overdraft_max_period, "overdraft_max_period"))
    {
        config.overdraft_max_period = 1;
    }
    rc = sqlite3_prepare_v2(db, BANK_CONFIG_DATA_REQUEST, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
        config.interest_rate = 1.5;
    }
    else
    {
        int idx = sqlite3_bind_parameter_index(res, "@param");
        sqlite3_bind_text(res, idx, "interest_rate", (int)strlen("interest_rate"), SQLITE_STATIC);
        rc = sqlite3_step(res);
        if(rc == SQLITE_ROW)
        {
            config.interest_rate = (double)atof(sqlite3_column_text(res, 0));
        }
        else
        {
            config.interest_rate = 1.5;
        }
        sqlite3_finalize(res);
    }
    
    rc = sqlite3_prepare_v2(db, BANK_CONFIG_DATA_REQUEST, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
        config.last_update.day = 1;
        config.last_update.month = 1;
        config.last_update.year = 2016;
    }
    else
    {
        int idx = sqlite3_bind_parameter_index(res, "@param");
        sqlite3_bind_text(res, idx, "last_update", (int)strlen("last_update"), SQLITE_STATIC);
        rc = sqlite3_step(res);
        if(rc == SQLITE_ROW)
        {
            config.last_update = parse_date(sqlite3_column_text(res, 0));
        }
        else
        {
            config.last_update.day = 1;
            config.last_update.month = 1;
            config.last_update.year = 2016;
        }
        sqlite3_finalize(res);
    }
}

BOOL start()
{
    int rc;
    rc = sqlite3_open("/Users/admin/Desktop/BanKING-System-master/Database/BanKING_System_database.db", &db);
    if(rc != SQLITE_OK)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return FALSE;
    }
    init_current_date();
    init_bank_config();
    update_db_date();
    
    transaction_fee_pay();
    interest_rate_pay();
    overdraft_fee_pay();
    return TRUE;
}

void lock_negative_accounts()
{
    sqlite3_stmt *res;
    int rc;
    rc = sqlite3_prepare_v2(db, LOCK_ACCOUNTS_REQUEST, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
        return;
    }
    rc = sqlite3_step(res);
    sqlite3_finalize(res);
}

void init_current_date()
{
    time_t curr_time = 0;
    struct tm* timeinfo;
    time(&curr_time);
    timeinfo = localtime(&curr_time);
    current_date.day = timeinfo->tm_mday;
    current_date.month = timeinfo->tm_mon + 1;
    current_date.year = timeinfo->tm_year + 1900;
}

void update_db_date()
{
    sqlite3_stmt *res;
    int rc, idx;
    char cur_date[15];
    get_string_current_date(cur_date, 15);
    rc = sqlite3_prepare_v2(db, UPDATE_DB_DATE_REQUEST, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(res);
        return;
    }
    idx = sqlite3_bind_parameter_index(res, "@date");
    sqlite3_bind_text(res, idx, cur_date, (int)strlen(cur_date), SQLITE_STATIC);
    rc = sqlite3_step(res);
    sqlite3_finalize(res);
}

void check_negative_balances()
{
    sqlite3_stmt *res;
    int rc, idx;
    char limit_date[15];
    date d = current_date;
    d.month += 1;
    if(d.day > 28)
    {
        d.day = 28;
    }
    get_string_date(limit_date, 15, d);
    rc = sqlite3_prepare_v2(db, CHECK_NEGATIVE_BALANCES_OVERDRAFT_REQUEST, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(res);
        return;
    }
    idx = sqlite3_bind_parameter_index(res, "@limit_date");
    sqlite3_bind_text(res, idx, limit_date, (int)strlen(limit_date), SQLITE_STATIC);
    rc = sqlite3_step(res);
    sqlite3_finalize(res);
    rc = sqlite3_prepare_v2(db, CHECK_NEGATIVE_BALANCES_ACCOUNT_REQUEST, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(res);
        return;
    }
    rc = sqlite3_step(res);
    sqlite3_finalize(res);
}

void transaction_fee_pay()
{
    sqlite3_stmt *res;
    int rc;
    if(current_date.month == config.last_update.month && current_date.year == config.last_update.year)
    {
        return;
    }
    rc = sqlite3_prepare_v2(db, PAY_TRANSACTION_FEE_REQUEST, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(res);
        return;
    }
    rc = sqlite3_step(res);
    sqlite3_finalize(res);
    check_negative_balances();
    lock_negative_accounts();
}

void overdraft_fee_pay()
{
    sqlite3_stmt *res;
    int rc;
    if(current_date.month == config.last_update.month && current_date.year == config.last_update.year && current_date.day == config.last_update.day)
    {
        return;
    }
    rc = sqlite3_prepare_v2(db, PAY_OVERDRAFT_FEE_REQUEST, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(res);
        return;
    }
    rc = sqlite3_step(res);
    sqlite3_finalize(res);
    lock_negative_accounts();
}

void interest_rate_pay()
{
    sqlite3_stmt *res;
    int rc;
    if(current_date.month == config.last_update.month && current_date.year == config.last_update.year)
    {
        return;
    }
    rc = sqlite3_prepare_v2(db, INTEREST_RATE_PAY_REQUEST, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(res);
        return;
    }
    rc = sqlite3_step(res);
    sqlite3_finalize(res);
}

void finish()
{
    sqlite3_close(db);
}

void get_string_current_date(char* buffer, int size)
{
    get_string_date(buffer, size, current_date);
}

void get_string_date(char* buffer, int size, date d)
{
    if(size < 11)
    {
        return;
    }
    memset(buffer, 0, size);
    sprintf(buffer, "%04d-%02d-%02d", d.year, d.month, d.day);
}

user_type authorization(char* login, char* password)
{
    sqlite3_stmt *res;
    int rc, idx;
    rc = sqlite3_prepare_v2(db, CHECK_USER_TYPE_REQUEST, -1, &res, 0);
    if (rc != SQLITE_OK)
    {
        return NOT_EXIST;
    }
    idx = sqlite3_bind_parameter_index(res, "@login");
    sqlite3_bind_text(res, idx, login, (int)strlen(login), SQLITE_STATIC);
    idx = sqlite3_bind_parameter_index(res, "@password");
    sqlite3_bind_text(res, idx, password, (int)strlen(password), SQLITE_STATIC);
    rc = sqlite3_step(res);
    if(rc != SQLITE_ROW)
    {
        sqlite3_finalize(res);
        return NOT_EXIST;
    }
    if(strcmp(sqlite3_column_text(res, 0), "OPERATOR") == 0)
    {
        sqlite3_finalize(res);
        return OPERATOR;
    }
    if(strcmp(sqlite3_column_text(res, 0), "ADMINISTRATOR") == 0)
    {
        sqlite3_finalize(res);
        return ADMINISTRATOR;
    }
    sqlite3_finalize(res);
    return NOT_EXIST;
}

