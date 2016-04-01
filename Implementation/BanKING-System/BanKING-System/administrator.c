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
#include <time.h>

typedef unsigned short RET_TYPE;
void clean_stdin()
{
    int c;
    do
    {
        c = getchar();
    } while (c != '\n' && c != EOF);
}
int number_generate(int plus)
{
    srand((int)time(0)+plus);
    return rand()%(99999999-10000000)+10000000;
}
//OK
BOOL string_input(char* message, char* destination, int dest_length)
{
    char *buf = (char*)malloc(sizeof(char)*(dest_length));
    printf(message);
	printf("Enter 0 to cancel\n");
    memset(buf, 0, dest_length);
    while(!fgets(buf,dest_length-1,stdin))
    {
        printf("Try again or press 0 to escape\n");
        clean_stdin();
        memset(buf, 0, dest_length);
    }
    if(buf[0]=='0')
        return FALSE;
    strncpy(destination, buf, dest_length-1);
    destination[strlen(buf)-1] = 0;
    free(buf);
    return TRUE;
}
//ок
BOOL write_text_not_null(char* param, char* value, sqlite3_stmt *pStmt)
{
    int indx = sqlite3_bind_parameter_index(pStmt, param);
    if(value[0]!='0')
    {
        if(sqlite3_bind_text (pStmt,indx,value, strlen(value), SQLITE_STATIC)!= SQLITE_OK)
        {
            return FALSE;
        }
    }
    return TRUE;
}

//ок
BOOL passport_account_validate(char* pasport_number, int account_number)
{
    sqlite3_stmt *pStmt;
    int indx = 0;
    int rc = sqlite3_prepare_v2(db, GET_FULL_ACCOUNT_INFORMATION_REQUEST, -1, &pStmt, 0);
    
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    indx = sqlite3_bind_parameter_index(pStmt, "@account_number");
    if(	sqlite3_bind_int(pStmt, indx, account_number)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        return 0;
    }
    if(!write_text_not_null("@pasport_number", pasport_number, pStmt))
    {
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

    sqlite3_finalize(pStmt);
    return TRUE;
    
}
//OK
RET_TYPE only_pass_request(char* pasport_number, const char* STMT)
{
	sqlite3_stmt *pStmt;
    int rc = sqlite3_prepare_v2(db, STMT, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        return SQLITE_ERROR;
    }
    if(!write_text_not_null("@pasport_number", pasport_number, pStmt))
    {
        sqlite3_finalize(pStmt);
        return SQLITE_ERROR;
    }
    rc = sqlite3_step(pStmt);
	sqlite3_finalize(pStmt);
	return rc;

}
//OK
RET_TYPE only_account_number_request(int account_number, const char* STMT)
{
	sqlite3_stmt *pStmt;
	int rc = sqlite3_prepare_v2(db, STMT, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        return SQLITE_ERROR;
    }
	int index = sqlite3_bind_parameter_index(pStmt, "@account_number");
	//printf("ind =  %d %d", index, account_number);
    if(	sqlite3_bind_int(pStmt, index, account_number)!= SQLITE_OK)
    {
		sqlite3_finalize(pStmt);
        return SQLITE_ERROR;
    }
	rc = sqlite3_step(pStmt);
	sqlite3_finalize(pStmt);
	return rc;

}
//ОК
BOOL add_overdraft(int account_number)
{
    sqlite3_stmt *pStmt;
    int index;
    int rc  = 0;
    rc = sqlite3_prepare_v2(db, ADD_OVERDRAFT_REQUEST, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        return FALSE;
    }
	char curr_date[51];
	get_string_current_date(curr_date,sizeof(curr_date));
    if(!write_text_not_null("@credit_date", curr_date, pStmt))
    {
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    if(!write_text_not_null("@limit_date", curr_date, pStmt))
    {
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    rc = sqlite3_step(pStmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(pStmt);
        return FALSE;
    }
	sqlite3_finalize(pStmt);
	if(only_account_number_request(account_number,UPDATE_OVERDRAFT_REQUEST)!= SQLITE_DONE)
	{
        return FALSE;
	}
	return TRUE;
}
BOOL  remove_overdraft(int account_number)
{
	if(only_account_number_request(account_number,REMOVE_OVERDRAFT_REQUEST)!= SQLITE_DONE)
	{
        return FALSE;
	}

	if(only_account_number_request(account_number,UPDATE_REMOVE_OVERDRAFT_REQUEST)!= SQLITE_DONE)
	{
        return FALSE;
	}
	return TRUE;
}

//ОК
BOOL add_client(char* firstname, char* lastname, char* pasport_number)
{
    sqlite3_stmt *pStmt;
    int rc =0;
	if(only_pass_request(pasport_number,CHECK_CLIENT_REQUEST)==SQLITE_ROW)
	{
		return FALSE;
	}
	rc = sqlite3_prepare_v2(db, ADD_CLIENT_REQUEST, -1, &pStmt, 0);

    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    if(!write_text_not_null("@firstname", firstname, pStmt))
    {
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    if(!write_text_not_null("@lastname", lastname, pStmt))
    {
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    if(!write_text_not_null("@pasport_number", pasport_number, pStmt))
    {
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    rc = sqlite3_step(pStmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    sqlite3_finalize(pStmt);
	return TRUE;
    
}
//ОК
void add_client_dialog()
{
    int buf_len = 50;
    char *firstname = (char*)malloc(sizeof(char)*(buf_len+1));
    char *lastname = (char*)malloc(sizeof(char)*(buf_len+1));
    char *pasport_number = (char*)malloc(sizeof(char)*(buf_len+1));
    
    
    if(!string_input("Enter firstame\n",firstname, buf_len+1))
    {
        printf("Canceled\n");
        free(firstname);
        free(lastname);
        free(pasport_number);
        return;
    }
    if(!string_input("Enter lastname\n", lastname, buf_len+1))
    {
        free(firstname);
        free(lastname);
        free(pasport_number);
        printf("Canceled\n");
        return;
    }
    if(!string_input("Enter pasport number\n", pasport_number, buf_len+1))
    {
        free(firstname);
        free(lastname);
        free(pasport_number);
        printf("Canceled\n");
        return;
    }
    if(!add_client(firstname,lastname,pasport_number))
	{
		printf("Unknown error\n");
	}
	else
	{
		printf("Operation succeed\n");
	}
    free(firstname);
    free(lastname);
    free(pasport_number);
}

//OK
BOOL remove_client(char* pasport_number)
{
	if(only_pass_request(pasport_number,CHECK_CLIENT_REQUEST)!=SQLITE_ROW)
	{
		printf("No such client\n");
		return FALSE;
	}
	if(SQLITE_DONE!=only_pass_request(pasport_number, 	REMOVE_CLIENT_OVERDRAFT_REQUEST))
	{
		return FALSE;
	}
	if(SQLITE_DONE!=only_pass_request(pasport_number, 	REMOVE_CLIENT_ACCOUNT_REQUEST))
	{
		return FALSE;
	}
	if(SQLITE_DONE!=only_pass_request(pasport_number, 	REMOVE_CLIENT_REQUEST))
	{
		return FALSE;
	}
    return TRUE;
}
//OK
void remove_client_dialog()
{
    int buf_len = 50;
    char *pasport_number = (char*)malloc(sizeof(char)*(buf_len+1));
    
    if(!string_input("Enter pasport number\n", pasport_number, buf_len+1))
    {
        printf("Canceled\n");
        free(pasport_number);
        return;
    }
    if(!remove_client(pasport_number))
	{
		printf("Unknown error\n");
	}
	else
	{
		printf("Operation succeed\n");
	}
    free(pasport_number);
}

//ОК
int add_account(char* passport_number, int acc_type, BOOL overdraft)
{
    sqlite3_stmt *pStmt;
    int index;
    int account_number = 0;
    int tryal = 0;
    BOOL end = FALSE;
    int rc = 0;
    char* account_type;
    rc = sqlite3_prepare_v2(db, ADD_ACCOUNT_REQUEST, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("Unknown error\n");
        sqlite3_finalize(pStmt);
        return 0;
    }
    if(!write_text_not_null("@pasport_number", passport_number, pStmt))
    {
        printf("Unknown error\n");
        sqlite3_finalize(pStmt);
        return 0;
    }
    if(acc_type==CURRENT)
    {
        account_type = "CURRENT";
    }
    if(acc_type==SAVING)
    {
        account_type = "SAVING";
    }
    if(!write_text_not_null("@account_type", account_type, pStmt))
    {
        printf("Unknown error\n");
        sqlite3_finalize(pStmt);
        return 0;
    }
    while(!end)
    {
        account_number = number_generate(5*tryal);
        tryal++;
		if(only_account_number_request(account_number,CHECK_ACCOUNT_REQUEST)== SQLITE_ROW)
		{
			end = FALSE;
		}
		else
		{
			//one or many times
			index = sqlite3_bind_parameter_index(pStmt, "@account_number");

			if(	sqlite3_bind_int(pStmt, index, account_number)!= SQLITE_OK)
			{
				printf("Unknown error\n");
				sqlite3_finalize(pStmt);
				return 0;
			}
			rc = sqlite3_step(pStmt);
			if (rc == SQLITE_DONE)
			{
				printf("Succeed\n");
				end = TRUE;
			}
			else
			{
				printf("Failed: unknown error\n");

			}
		}
    }
    if(overdraft)
    {
        if(!add_overdraft(account_number))
		{
			printf("Unknown error\n");
		}
    }
    sqlite3_finalize(pStmt);
    return account_number;
            
}
//ОК
void add_account_dialog()
{
    int buf_len = 50;
    char *pasport_number = (char*)malloc(sizeof(char)*(buf_len+1));
    int account_number = 0;
    if(!string_input("Enter pasport number\n", pasport_number, buf_len+1))
    {
        free(pasport_number);
        printf("Canceled\n");
        return;
    }
	if(only_pass_request(pasport_number,CHECK_CLIENT_REQUEST)!=SQLITE_ROW)
	{
		printf("This client doesn't exist\n");
		free(pasport_number);
		return;
	}
    printf("Enter account type\n");
    printf("(1) Current\n");
    printf("(2) Saving\n");
    BOOL end = FALSE;
    account_type acc_type = 0;
    while(!end)
    {
        int option = 0;
        if(scanf("%d",&option)==1)
        {
            switch(option)
            {
            case 0:
            {
                end = TRUE;
                free(pasport_number);
                pasport_number = NULL;
				printf("Canceled\n");
				clean_stdin();
                return;
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
                    printf("Try again or press 0 to cancel\n");
                    break;
                }
            }
        }

        else
        {
            clean_stdin();
            printf("Try again or press 0 to cancel\n");
        }
    }
    end = FALSE;
    if(acc_type==CURRENT)
    {
        printf("Create account with overdraft?\n");
        printf("(1) Yes\n");
        printf("(2) No\n");
        while(!end)
        {
            int option = 0;
            if(scanf("%d",&option)==1)
            {
                switch(option)
                {
                       case 0:
                           {
                               end = TRUE;
                               free(pasport_number);
                               pasport_number = NULL;
                               printf("Canceled\n");
							   clean_stdin();
                               break;
                           }
                       case 1:
                           {
                               end = TRUE;
                               account_number = add_account(pasport_number, acc_type, TRUE);
                               clean_stdin();
                               break;
                           }
                       case 2:
                           {
                               end = TRUE;
                               account_number = add_account(pasport_number, acc_type, FALSE);
                               clean_stdin();
                               break;
                               
                           }
                       default:
                           {
                               clean_stdin();
                               printf("Try again press or 0 to cancel\n");
                               break;
                           }
                }
            }
            else
            {

                clean_stdin();
                printf("Try again or press 0 to cancel\n");
            }
        }
    }
    else
    {
        account_number = add_account(pasport_number, acc_type, FALSE);
    }
    if(account_number !=0)
    {
        printf("Number of account created: %d\n", account_number);
    }
}

BOOL close_account(char* pasport_number, int account_number)
{
    int index = 0;
    int rc = 0;
    if(!passport_account_validate(pasport_number, account_number))
    {
        return FALSE;
    }
	if(only_account_number_request(account_number, REMOVE_OVERDRAFT_REQUEST)!= SQLITE_DONE)
    {
        return FALSE;
    }
	if(only_account_number_request(account_number, CLOSE_ACCOUNT_REQUEST)!= SQLITE_DONE)
    {
        return FALSE;
    }
    return TRUE;
}
void close_account_dialog()
{
	int buf_len = 50;
	char *pasport_number = (char*)malloc(sizeof(char)*(buf_len+1));
	if(!string_input("Enter pasport number\n", pasport_number, buf_len+1))
	{
		printf("Canceled\n");
		free(pasport_number);
		return;
	}
	if(only_pass_request(pasport_number,CHECK_CLIENT_REQUEST)!=SQLITE_ROW)
	{
		printf("This client doesn't exist\n");
		free(pasport_number);
		return;
	}
	int account_number = -1;
	printf("Enter account_number:\n");
	while(scanf("%d",&account_number)!=1)
	{
		clean_stdin();
		printf("Try again or 0 to cancel\n");
	}
	clean_stdin();
	if(account_number==0)
	{
		free(pasport_number);
		printf("Canceled\n");
		return;
	}
   if(!close_account(pasport_number,account_number))
   {
	   printf("Unknown error\n");
   }
   	else
	{
		printf("Operation succeed\n");
	}
    free(pasport_number);
}
BOOL change_account_type(int account_number, char* type)
{
    int rc = 0;
    sqlite3_stmt *pStmt;
    int index;
	if(only_account_number_request(account_number, REMOVE_OVERDRAFT_REQUEST)!= SQLITE_DONE)
    {
        return FALSE;
    }
    rc = sqlite3_prepare_v2(db, CHANGE_ACCOUNT_TYPE_REQUEST, -1, &pStmt, 0);
    if (rc != SQLITE_OK)
    {

        sqlite3_finalize(pStmt);
        return FALSE;
    }

    index = sqlite3_bind_parameter_index(pStmt, "@account_number");
    if(	sqlite3_bind_int(pStmt, index, account_number)!= SQLITE_OK)
    {
        sqlite3_finalize(pStmt);

        return FALSE;

    }
    if(!write_text_not_null("@account_type", type, pStmt))
    {

        sqlite3_finalize(pStmt);
        return FALSE;
    }
    rc = sqlite3_step(pStmt);
    if (rc != SQLITE_DONE)
    {
        sqlite3_finalize(pStmt);
        return FALSE;
    }
    sqlite3_finalize(pStmt);
	return TRUE;
}
BOOL current_account_dialog(BOOL with_overdraft, BOOL* changed, int account_number)
{
    BOOL end = FALSE;
    printf("Choose option\n");
    printf("(0) - Cancel\n");
    printf("(1) - Change type to saving\n");
    if(!with_overdraft)
    {
        printf("(2) - Add overdraft\n");
    }
	else
	{
		 printf("(2) - Remove overdraft\n");
	}
    while(!end)
    {
        int option = 0;
        if(scanf("%d",&option)==1)
        {
            switch(option)
            {
                case 0:
                {
                    end = TRUE;
                    clean_stdin();
                    printf("Canceled\n");
                    *changed = FALSE;
                    return FALSE;

                }
                case 1:
                {
                    end = TRUE;
                    *changed = TRUE;
                    if(!change_account_type(account_number,"SAVING"))
					{
						printf("Unknown error\n");
					}
					else
					{
						printf("Operation succeed\n");
					}
                    clean_stdin();
                    return TRUE;
                }
                case 2:
                {
                    if(!with_overdraft)
                    {
                        end = TRUE;
                        *changed = FALSE;
                        if(!add_overdraft(account_number))
						{
							printf("Unknown error\n");
						}
						else
						{
							printf("Operation succeed\n");
						}
                        clean_stdin();
                        return TRUE;
                    }
					else
					{
						end = TRUE;
                        *changed = FALSE;
                        if(!remove_overdraft(account_number))
						{
							printf("Unknown error\n");
						}
						else
						{
							printf("Operation succeed\n");
						}
						clean_stdin();
                        return TRUE;
					}
                }
                default:
                {
                    clean_stdin();
                    printf("Try again or press 0 to cancel\n");
                    break;
                }
            }
        }
        else
        {
            clean_stdin();

            printf("Try again or press 0 to cancel\n");
        }
    }
    return TRUE;
}
BOOL saving_account_dialog(int account_number)
{
    BOOL end = FALSE;
    printf("Choose option\n");
    printf("(0) - Cancel\n");
    printf("(1) - Change type to current\n");

    while(!end)
    {
        int option = 0;
        if(scanf("%d",&option)==1)
        {
            switch(option)
            {
                case 0:
                {
                    end = TRUE;
                    clean_stdin();
                    printf("Canceled\n");
                    return FALSE;
                }
                case 1:
                {
                    end = TRUE;
                    if(!change_account_type(account_number,"CURRENT"))
					{
						printf("Unknown error\n");
					}
					else
					{
						printf("Operation succeed\n");
					}
                    clean_stdin();
                    return TRUE;
                }
                default:
                {
                    clean_stdin();
                    printf("Try again or press 0 to cancel\n");
                    break;
                }
            }
        }
        else
        {
            clean_stdin();
            printf("Try again or press 0 to cancel\n");
        }
    }
    return TRUE;
}
void account_management_dialog()
 {
     int buf_len = 50;
     char *pasport_number = (char*)malloc(sizeof(char)*(buf_len+1));
     BOOL end = FALSE;
     sqlite3_stmt *pStmt;
     BOOL *changed = (BOOL *)malloc(sizeof(short));
     *changed= FALSE;
     int rc = 0;
     int index = 0;
     if(!string_input("Enter pasport number\n", pasport_number, buf_len+1))
     {
         printf("Canceled\n");
         free(pasport_number);
         return;
     }
	 if(only_pass_request(pasport_number,CHECK_CLIENT_REQUEST)!=SQLITE_ROW)
	 {
		printf("This client doesn't exist\n");
		free(pasport_number);
		free(changed);

		return;
	 }
	 int account_number = -1;
	 printf("Enter account_number:\n");
	 while(scanf("%d",&account_number)!=1)
	 {
		 clean_stdin();
		 printf("Try again or press 0 to cancel\n");
	 }
	 clean_stdin();
	 if(account_number==0)
	 {
		 free(pasport_number);
		 printf("Canceled\n");
		 return;
	 }
     if(!passport_account_validate(pasport_number, account_number))
     {
        return;
     }
     rc = sqlite3_prepare_v2(db, GET_FULL_ACCOUNT_INFORMATION_REQUEST, -1, &pStmt, 0);
     if (rc != SQLITE_OK)
     {
         printf("Unknown error\n");
         sqlite3_finalize(pStmt);
         return;
     }
     index = sqlite3_bind_parameter_index(pStmt, "@account_number");
     if(sqlite3_bind_int(pStmt, index, account_number)!= SQLITE_OK)
     {
         sqlite3_finalize(pStmt);
         printf("Unknown error\n");
         return;
     }
     if(!write_text_not_null("@pasport_number", pasport_number, pStmt))
     {
         printf("Unknown error\n");
         sqlite3_finalize(pStmt);
         return;
     }
     rc = sqlite3_step(pStmt);
     if (rc == SQLITE_ROW)
     {
         int i = 2;
		 BOOL overdraft = FALSE;
         char* type = sqlite3_column_text(pStmt, i);
		 int id_type = 0;
		 if(strcmp(type, "SAVING")==0)
		 {
			 id_type = 2;
		 }
		 else
		 {
			 id_type = 1;

             if(SQLITE_NULL!=sqlite3_column_type(pStmt, 5))
			 {
				 overdraft = TRUE;
			 }
			 else
			 {
				 overdraft = FALSE;
			 }
		 }

         while(!end)
         {
             if(id_type==2)
             {
                 if(!saving_account_dialog(account_number))
                 {
                     end = TRUE;
                 }
                 else
                 {
                    id_type = 1;
                 }
             }
             else
             {

                 if(!current_account_dialog(overdraft, changed, account_number))
                 {
                     end = TRUE;
                 }
                 else
                 {
                     if(*changed)
                     {
						 id_type = 2;
                     }
					 else
					 {
						 overdraft=!overdraft;
					 }
                     
                 }
             }
         }
     }
     sqlite3_finalize(pStmt);
     free(pasport_number);
     return;
 }
