INSERT INTO BANK_ACCOUNTS(account_number, client_id, type_id)
VALUES ("87651234", 
        (SELECT id FROM BANK_CLIENTS WHERE passport_number = "MT3451213"),
        (SELECT id FROM ACCOUNT_TYPE WHERE TYPE = "CURRENT")
        );