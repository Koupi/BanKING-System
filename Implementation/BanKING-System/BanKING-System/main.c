//
//  main.c
//  BanKING-System
//
//  Created by Admin on 27.03.16.
//  Copyright © 2016 Yawning Turtles. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "system.h"
#include "operator.h"
#include "administrator.h"

void user_login();
void operator_menu();
void administrator_menu();

int main(int argc, const char * argv[]) {
    
    if(!start())
    {
        printf("Error occured. Can't run this app.\n");
        return 1;
    }
    printf("Welcome to BanKING System!\n");
    user_login();
    printf("Good bye.\n");
    finish();
    
    return 0;
}

void user_login()
{
    char login[40];
    char password[40];
    user_type user = NOT_EXIST;
    while(user == NOT_EXIST)
    {
        printf("Enter your login (q - to exit):\n");
        fpurge(stdin);
        gets(login);
        if(strcmp(login, "q") == 0)
        {
            break;
        }
        printf("Enter your password:\n");
        fpurge(stdin);
        gets(password);
        user = authorization(login, password);
        switch(user)
        {
            case OPERATOR:
                operator_menu();
                break;
            case ADMINISTRATOR:
                administrator_menu();
                break;
            case NOT_EXIST:
                printf("Wrong login or password. Try again.\n");
                break;
        }
    }
}

void operator_menu()
{
    int choice = -1;
    while(choice != 0)
    {
        printf("What to do:\n");
        printf("0 - exit\n");
        printf("1 - enroll\n");
        printf("2 - withdraw\n");
        printf("3 - transfer\n");
        fpurge(stdin);
        if(scanf("%d", &choice) != 1)
        {
            printf("Unknown operation.\n");
            fpurge(stdin);
            choice = -1;
            continue;
        }
        switch (choice)
        {
            case 0:
                break;
            case 1:
                enroll_dialog();
                break;
            case 2:
                withdraw_dialog();
                break;
            case 3:
                transfer_dialog();
                break;
            default:
                printf("Unknown operation.\n");
                break;
        }
    }
}

void administrator_menu()
{
    int choice = -1;
    while(choice != 0)
    {
        printf("What to do:\n");
        printf("0 - exit\n");
        printf("1 - add client\n");
        printf("2 - add account\n");
        printf("3 - account management\n");
        printf("4 - close account\n");
        printf("5 - remove client\n");
        fpurge(stdin);
        if(scanf("%d", &choice) != 1)
        {
            printf("Unknown operation.\n");
            fpurge(stdin);
            choice = -1;
            continue;
        }
        switch (choice)
        {
            case 0:
                break;
            case 1:
                add_client_dialog();
                break;
            case 2:
                add_account_dialog();
                break;
            case 3:
                account_management_dialog();
                break;
            case 4:
                close_account_dialog();
                break;
            case 5:
                remove_client_dialog();
                break;
            default:
                printf("Unknown operation.\n");
                break;
        }
    }
}
