//
//  administrator.c
//  BanKING-System
//
//  Created by Admin on 27.03.16.
//  Copyright © 2016 Yawning Turtles. All rights reserved.
//

#include "administrator.h"
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned short BOOL;
#define TRUE 1
#define FALSE 0
const char* ADD_ACCOUNT_REQUEST = "INSERT INTO BANK_ACCOUNTS(account_number, client_id, type_id)\
VALUES (@account_number, (SELECT id FROM BANK_CLIENTS WHERE passport_number = @passport_number), (SELECT id FROM ACCOUNT_TYPE WHERE TYPE = @account_type) );"
const char* ADD_CLIENT_REQUEST = "INSERT INTO BANK_CLIENTS(first_name, last_name, passport_number) VALUES (@firstname, @lastname, @pasport_number)";
const char* ADD_OVERDRAFT_REQUEST = "INSERT INTO OVERDRAFTS(credit_date, limit_date) VALUES (@credit, @limuit); \
UPDATE BANK_ACCOUNTS SET overdraft_id = (SELECT id FROM OVERDRAFTS ORDER BY id DESC LIMIT 1) \
WHERE account_number = @account_number;"
const char* CLOSE_ACCOUNT_REQUEST	= "DELETE FROM OVERDRAFTS WHERE id = (SELECT overdraft_id FROM BANK_ACCOUNTS WHERE account_number = @account_number); \
DELETE FROM BANK_ACCOUNTS WHERE account_number = @account_number;"
const char* REMOVE_OVDERDRAFT_REQUEST	= "DELETE FROM OVERDRAFTS WHERE id = (SELECT overdraft_id FROM BANK_ACCOUNTS WHERE account_number = @account_number); \
UPDATE BANK_ACCOUNTS SET overdraft_id = NULL WHERE account_number = @account_number;"
const char* REMOVE_CLIENT_REQUEST	= "DELETE FROM OVERDRAFTS WHERE (SELECT passport_number FROM BANK_CLIENTS WHERE id = \
(SELECT client_id FROM BANK_ACCOUNTS WHERE overdraft_id = OVERDRAFTS.id)) = @pasport_number; \
DELETE FROM BANK_ACCOUNTS WHERE client_id = (SELECT id FROM BANK_CLIENTS WHERE passport_number = @passport_number); \
DELETE FROM BANK_CLIENTS WHERE passport_number = @passport_number;
const char* GET_FULL_ACCOUNT_INFORMATION_REQUEST = "SELECT BANK_ACCOUNTS.id, ACCOUNT_TYPE.type, BANK_ACCOUNTS.balance, BANK_ACCOUNTS.total_transactions, \
BANK_ACCOUNTS.account_number, OVERDRAFTS.id, OVERDRAFTS.credit_sum, BANK_CLIENTS.id, BANK_ACCOUNTS.is_locked \
FROM BANK_CLIENTS JOIN BANK_ACCOUNTS ON BANK_ACCOUNTS.client_id = BANK_CLIENTS.id JOIN ACCOUNT_TYPE \
ON BANK_ACCOUNTS.type_id = ACCOUNT_TYPE.id LEFT JOIN OVERDRAFTS ON BANK_ACCOUNTS.overdraft_id = OVERDRAFTS.id WHERE BANK_ACCOUNTS.account_number = @account_number AND BANK_CLIENTS.passport_number = @pasport_number;";
const char* CHANGE_ACCOUNT_TYPE = "DELETE FROM OVERDRAFTS WHERE id = (SELECT overdraft_id FROM BANK_ACCOUNTS WHERE account_number = @account_number); \
UPDATE BANK_ACCOUNTS SET type_id = (SELECT id FROM ACCOUNT_TYPE WHERE `type` = @account_type), \
overdraft_id = NULL WHERE account_number = @account_number;";
void clean_stdin()
{
    int c;
    do
    {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

BOOL string_input(char* message, char* destination, int dest_length)
{
    char *buf = (char*)malloc(sizeof(char)*(dest_length));
    printf(message);
    memset(buf, 0, dest_length-1);
    while(!fgets(buf,dest_length-1,stdin))
    {
        printf("Try again or press 0 to escape");
        clean_stdin();
        memset(buf, 0, dest_length+1);
    }
    if(buf[0]=='0')
        return FALSE;
    strncpy(destination, buf, dest_length-1);
    destination[strlen(buf)-1] = 0;
    free(buf);
    return TRUE;
}

BOOL write_text_not_null(char* param, char* value, sqlite3_stmt *pStmt)
{
    int indx = sqlite3_bind_parameter_index(pStmt, param);
    if(value[0]!='0')
    {
        if(sqlite3_bind_text (pStmt,indx,value, strlen(value), SQLITE_STATIC)!= SQLITE_OK)
        {
            printf("Failed binding");
            return FALSE;
        }
    }
    return TRUE;
}
BOOL passport_account_validate(char* pasport_number, int account_number)
{
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
    if(!write_text_not_null("@pasport_number", pasport_number, pStmt))
    {
        printf("Faild creating");
        sqlite3_finalize(pStmt);
        return;
    }
    rc = sqlite3_step(pStmt);
    if (rc != SQLITE_ROW)
    {
        sqlite3_finalize(pStmt);
        printf("No such account with this client: %d", account_number);
        return FALSE;
    }
    else
    {
        sqlite3_finalize(pStmt);
        printf("No such account: %d", account_number);
        return FALSE;
    }
    sqlite3_finalize(pStmt);
    return TRUE;
    
}
void add_client(char* firstname, char* lastname, char* pasport_number)
{
    sqlite3_stmt *pStmt;
    int index;
    int rc = sqlite3_prepare_v2(db, ADD_CLIENT_REQUEST, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return;
    }
    if(!write_text_not_null("@firstname", firstname, pStmt))
    {
        printf("Faild creating");
        sqlite3_finalize(pStmt);
        return;
    }
    if(!write_text_not_null("@lastname", lastname, pStmt))
    {
        printf("Faild creating");
        sqlite3_finalize(pStmt);
        return;
    }
    if(!write_text_not_null("@pasport_number", pasport_number, pStmt))
    {
        printf("Faild creating");
        sqlite3_finalize(pStmt);
        return;
    }
    rc = sqlite3_step(pStmt);
    if (rc != SQLITE_DONE)
    {
        printf("execution failed: %s", sqlite3_errmsg(db));
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    sqlite3_finalize(pStmt);
    
}

void add_client_dialog()
{
    int buf_len = 50;
    char *firstname = new char[buf_len+1];
    char *lastname = new char[buf_len+1];
    char *pasport_number = new char[buf_len+1];
    
    
    if(!string_input("Enter firstame\n",firstname, buf_len+1))
    {
        printf("Canceled");
        delete []firstname;
        delete []lastname;
        delete []pasport_number;
        return;
    }
    if(!string_input("Enter lastname\n", lastname, buf_len+1)
       {
           delete []firstname;
           delete []lastname;
           delete []pasport_number;
           printf("Canceled");
           return;
       }
       if(!string_input("Enter pasport number\n", pasport_number, buf_len+1);
          {
              delete []firstname;
              delete []lastname;
              delete []pasport_number;
              printf("Canceled");
              return;
          }
          add_client(firstname,lastname,pasport_number);
          delete []firstname;
          delete []lastname;
          delete []pasport_number;
          }
          BOOL remove_client(char* pasport_number)
    {
        sqlite3_stmt *pStmt;
        int index;
        int rc = sqlite3_prepare_v2(db, REMOVE_CLIENT_REQUEST, -1, &pStmt, 0);
        if (rc != SQLITE_OK)
        {
            printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(pStmt);
            return FALSE;
        }
        if(!write_text_not_null("@pasport_number", pasport_number, pStmt))
        {
            printf("Faild creating");
            sqlite3_finalize(pStmt);
            return FALSE;
        }
        rc = sqlite3_step(pStmt);
        if (rc != SQLITE_OK)
        {
            printf("Execution failed or no such number: %s", sqlite3_errmsg(db));
            sqlite3_finalize(pStmt);
            return FALSE;
        }
        sqlite3_finalize(pStmt);
    }
          BOOL remove_client_dialog()
    {
        int buf_len = 50;
        char *pasport_number = new char[buf_len+1];
        
        if(!string_input("Enter pasport number\n", pasport_number, buf_len+1);
           {
               printf("Canceled");
               delete []pasport_number;
               return;
           }
           remove_client(pasport_number);
           delete []pasport_number;
           }
           int number_generate(int plus)
        {
            srand(time(0)+plus);
            return rand()/(RAND_MAX-10000)+10000;
        }
           int add_account(char* passport_number, int acc_type, BOOL overdraft)
        {
            sqlite3_stmt *pStmt;
            int index;
            int account_number;
            int tryal = 0;
            BOOL end = FALSE;
            int rc = 0;
            char* account_type;
            rc = sqlite3_prepare_v2(db, ADD_ACCOUNT_REQUEST, -1, &pStmt, 0);
            if (rc != SQLITE_OK)
            {
                printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
                sqlite3_finalize(pStmt);
                return FALSE;
            }
            if(!write_text_not_null("@pasport_number", passport_number, pStmt))
            {
                printf("Faild creating: incorrect pasport number?");
                sqlite3_finalize(pStmt);
                return FALSE;
            }
            
            if(acc_type==CURRENT)
            {
                account_type = "Current";
            }
            if(acc_type==SAVING)
            {
                account_type = "Saving";
            }
            if(!write_text_not_null("@account_type", passport_number, pStmt))
            {
                printf("Faild creating");
                sqlite3_finalize(pStmt);
                return FALSE;
            }
            while(!end)
            {
                account_number = number_generate(5*tryal);
                tryal++:
                indx = sqlite3_bind_parameter_index(pStmt, "@account_number");
                if(	sqlite3_bind_int(pStmt, indx, account_number)!= SQLITE_OK)
                {
                    sqlite3_finalize(pStmt);
                    printf("Failed binding");
                    return 0;
                }
                rc = sqlite3_step(pStmt);
                if (rc == SQLITE_OK)
                {
                    end = TRUE;
                }
            }
            if(overdraft)
            {
                if(!add_overdraft(account_number))
                {
                    sqlite3_finalize(pStmt);
                    return 0;
                }
            }
            sqlite3_finalize(pStmt);
            return account_number;
            
        }
           BOOL add_account_dialog()
        {
            int buf_len = 50;
            char *pasport_number = new char[buf_len+1];
            int account_number = 0;
            if(!string_input("Enter pasport number\n", pasport_number, buf_len+1);
               {
                   delete []pasport_number;
                   printf("Canceled");
                   return;
               }
               printf("Enter account type\n");
               printf("(1) Current\n");
               printf("(2) Saving\n");
               BOOL end = FALSE;
               account_type acc_type;
               while(!end)
               {
                   int option = 0;
                   if(scanf("%d",option)==1)
                   {
                   case 0:
                       {
                           end = TRUE;
                           delete []pasport_number;
                           pasport_number = NULL;
                           printf("Canceled");
                           
                           break;
                       }
                   case 1:
                       {
                           end = TRUE;
                           acc_type = CURRENT;
                           clean_stdin();
                           break;
                       }
                   case 2:
                       {
                           end = TRUE;
                           acc_type = SAVING;
                           clean_stdin();
                           break;
                           
                       }
                   default:
                       {
                           clean_stdin();
                           printf("Try again or 0 to cancel");
                           break;
                       }
                   }
                   else
                   {
                       clean_stdin();
                       printf("Try again or 0 to cancel");
                   }
                   
               }
               
               end = FALSE;
               if(acc_type==CURRENT)
               {
                   printf("Create with overdraft?n");
                   printf("(1) Yes\n");
                   printf("(2) No\n");
                   while(!end)
                   {
                       int option = 0;
                       if(scanf("%d",option)==1)
                       {
                       case 0:
                           {
                               end = TRUE;
                               delete []pasport_number;
                               pasport_number = NULL;
                               printf("Canceled");
                               
                               break;
                           }
                       case 1:
                           {
                               end = TRUE;
                               account_number = add_account(passport_number, acc_type, TRUE);
                               clean_stdin();
                               break;
                           }
                       case 2:
                           {
                               end = TRUE;
                               account_number = add_account(passport_number, acc_type, FALSE);
                               clean_stdin();
                               break;
                               
                           }
                       default:
                           {
                               clean_stdin();
                               printf("Try again or 0 to cancel");
                               break;
                           }
                       }
                       else
                       {
                           clean_stdin();
                           printf("Try again or 0 to cancel");
                       }
                       
                   }
                   else
                   {
                       account_number = add_account(passport_number, acc_type, FALSE);
                   }
                   if(account_number !=0)
                   {
                       printf("Account number created: %d", add_account);
                   }
               }
               BOOL add_overdraft(int account_number)
            {
                sqlite3_stmt *pStmt;
                int index;
                /*int rc = sqlite3_prepare_v2(db, GET_FULL_ACCOUNT_INFORMATION_REQUEST, -1, &pStmt, 0);
                 
                 if (rc != SQLITE_OK)
                 {
                 printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
                 sqlite3_finalize(pStmt);
                 return;
                 }
                 indx = sqlite3_bind_parameter_index(pStmt, "@account_type");
                 if(	sqlite3_bind_int(pStmt, indx, account_type)!= SQLITE_OK)
                 {
                 sqlite3_finalize(pStmt);
                 printf("Failed binding");
                 return 0;
                 }
                 rc = sqlite3_step(pStmt);
                 if (rc == SQLITE_ROW)
                 {
                 int i = 5;
                 char* pass = sqlite3_column_text(pStmt, i));
                 if(!strcmp(pass,passport_number))
                 {
                 sqlite3_finalize(pStmt);
                 printf("Invalide passport number: %d", passport_number);
                 return FALSE;
                 }
                 }
                 else
                 {
                 sqlite3_finalize(pStmt);
                 printf("No such account: %d", account_number);
                 return FALSE;
                 }
                 sqlite3_finalize(pStmt);
                 pStmt = NULL;
                 */
                sqlite3_finalize(pStmt);
                rc = sqlite3_prepare_v2(db, ADD_OVERDRAFT_REQUEST, -1, &pStmt, 0);
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
                    return FALSE;
                }
                if(!write_text_not_null("@credit_date", get_string_current_date(), pStmt))
                {
                    printf("Faild creating");
                    sqlite3_finalize(pStmt);
                    return FALSE;
                }
                if(!write_text_not_null("@limit_date", get_string_current_date(), pStmt))
                {
                    printf("Faild creating");
                    sqlite3_finalize(pStmt);
                    return FALSE;
                }
                rc = sqlite3_step(pStmt);
                if (rc != SQLITE_OK)
                {
                    printf("Execution failed : %s", sqlite3_errmsg(db));
                    sqlite3_finalize(pStmt);
                    return FALSE;
                }
                
                sqlite3_finalize(pStmt);
                return TRUE;
                
            }
               BOOL close_account(char* pasport_number, int account_number)
            {
                sqlite3_stmt *pStmt;
                int index;
                int rc = 0;
                if(!passport_account_validate(pasport_number, account_number))
                {
                    return FALSE;
                }
                rc = sqlite3_prepare_v2(db, CLOSE_ACCOUNT_REQUEST, -1, &pStmt, 0);
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
                    return FALSE;
                }
                rc = sqlite3_step(pStmt);
                if (rc != SQLITE_OK)
                {
                    printf("error : %s", sqlite3_errmsg(db));
                    sqlite3_finalize(pStmt);
                    return FALSE;
                }
                sqlite3_finalize(pStmt);
            }
               void close_account_dialog()
            {
                int buf_len = 50;
                char *pasport_number = new char[buf_len+1];
                
                if(!string_input("Enter pasport number\n", pasport_number, buf_len+1);
                   {
                       printf("Canceled");
                       delete []pasport_number;
                       return;
                   }
                   int account_number = 0;
                   
                   while(!scanf("%d",account_type)==1)
                   {
                       if(account_type==0)
                       {
                           delete []pasport_number;
                           printf("Canceled\n");
                           return 0;
                       }
                       clean_stdin();
                       printf("Try again or 0 to cancel");
                   }
                   close_account(pasport_number,account_number);
                   delete []pasport_number;
                   }
                   void change_type(int account_number, type)
                {
                    int rc = 0;
                    sqlite3_stmt *pStmt;
                    int index;
                    rc = sqlite3_prepare_v2(db, CHANGE_ACCOUNT_TYPE_REQUEST, -1, &pStmt, 0);
                    if (rc != SQLITE_OK)
                    {
                        printf("Cannot prepare statement: %s\n", sqlite3_errmsg(db));
                        sqlite3_finalize(pStmt);
                        return;
                    }
                    indx = sqlite3_bind_parameter_index(pStmt, "@account_number");
                    if(	sqlite3_bind_int(pStmt, indx, account_number)!= SQLITE_OK)
                    {
                        sqlite3_finalize(pStmt);
                        printf("Failed binding");
                        return;
                    }
                    if(!write_text_not_null("@account_type", type, pStmt))
                    {
                        printf("Faild creating");
                        sqlite3_finalize(pStmt);
                        return;
                    }
                    rc = sqlite3_step(pStmt);
                    if (rc != SQLITE_OK)
                    {
                        printf("error : %s", sqlite3_errmsg(db));
                        sqlite3_finalize(pStmt);
                        return;
                    }
                    sqlite3_finalize(pStmt);
                }
                   BOOL current_account_dialog(BOOL with_overdraft, BOOL* changed, int account_number)
                {
                    BOOL end = FALSE;
                    printf("Choose option\n")
                    printf("(0) - Cancel");
                    printf("(1) - change type to saving\n");
                    if(with_overdraft)
                    {
                        printf("(2) - Add overdraft\n");
                    }
                    
                    while(!end)
                    {
                        int option = 0;
                        if(scanf("%d",option)==1)
                        {
                        case 0:
                            {
                                end = TRUE;
                                clean_stdin();
                                printf("Canceled");
                                changed = FALSE;
                                return FALSE;
                            }
                        case 1:
                            {
                                end = TRUE;
                                changed = TRUE;
                                change_type(account_number,"Saving");
                                clean_stdin();
                                return TRUE;
                            }
                        case 2:
                            {
                                if(with_overdraft)
                                {
                                    end = TRUE;
                                    changed = FALSE;
                                    add_overdraft(account_number);
                                    clean_stdin();
                                    return TRUE;
                                }
                                
                            }
                        default:
                            {
                                clean_stdin();
                                printf("Try again or 0 to cancel");
                                break;
                            }
                        }
                        else
                        {
                            clean_stdin();
                            printf("Try again or 0 to cancel");
                        }
                    }
                    return TRUE;
                    
                }
                   BOOL saving_account_dialog(int account_number)
                {
                    BOOL end = FALSE;
                    printf("Choose option\n")
                    printf("(0) - Cancel");
                    printf("(1) - change type to current\n");
                    
                    while(!end)
                    {
                        int option = 0;
                        if(scanf("%d",option)==1)
                        {
                        case 0:
                            {
                                end = TRUE;
                                clean_stdin();
                                printf("Canceled");
                                return FALSE;
                            }
                        case 1:
                            {
                                end = TRUE;
                                change_type(account_number,"Current");
                                clean_stdin();
                                return TRUE;
                            }
                        default:
                            {
                                clean_stdin();
                                printf("Try again or 0 to cancel");
                                break;
                            }
                        }
                        else
                        {
                            clean_stdin();
                            printf("Try again or 0 to cancel");
                        }
                    }
                    return TRUE;
                    
                }
                   void account_management_dialog()
                {
                    int buf_len = 50;
                    char *pasport_number = new char[buf_len+1];
                    BOOL end = FALSE;
                    BOOL *changed = (BOOL *)malloc(sizeof(short));
                    *changed= FALSE;
                    if(!string_input("Enter pasport number\n", pasport_number, buf_len+1);
                       {
                           printf("Canceled");
                           delete []pasport_number;
                           return;
                       }
                       int account_number = 0;
                       
                       while(!scanf("%d",account_type)==1)
                       {
                           if(account_type==0)
                           {
                               delete []pasport_number;
                               printf("Canceled\n");
                               return 0;
                           }
                           clean_stdin();
                           printf("Try again or 0 to cancel");
                       }
                       if(!passport_account_validate(pasport_number, account_number))
                       {
                           return FALSE;
                       }
                       rc = sqlite3_prepare_v2(db, GET_FULL_ACCOUNT_INFORMATION_REQUEST, -1, &pStmt, 0);
                       
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
                           return FALSE;
                       }
                       if(!write_text_not_null("@pasport_number", pasport_number, pStmt))
                       {
                           printf("Faild creating");
                           sqlite3_finalize(pStmt);
                           return FALSE;
                       }
                       rc = sqlite3_step(pStmt);
                       if (rc == SQLITE_ROW)
                       {
                           int i = 2;
                           char* type = sqlite3_column_text(pStmt, i);
                           
                           while(!end)
                           {
                               if(strcmp(type, "SAVING")==0)
                               {
                                   if(!saving_account_dialog())
                                   {
                                       end = TRUE;
                                   }
                                   else
                                   {
                                       type ="CURRENT";
                                   }
                               }
                               else
                               {
                                   BOOL overdraft = FALSE;
                                   if(SQLITE_NULL!=sqlite3_column_type(pStmt, 6))
                                   {
                                       overdraft = TRUE;
                                   }
                                   if(!current_account_dialog(overdraft, changed))
                                   {
                                       end = TRUE;
                                   }
                                   else
                                   {
                                       if(cnanged)
                                       {
                                           type="SAVING";
                                       }
                                       
                                   }
                                   
                               }
                           }
                       }
                       sqlite3_finalize(pStmt);
                       return TRUE;
                       delete []pasport_number;
                       }
