DELETE FROM OVERDRAFTS
WHERE id = (SELECT overdraft_id FROM BANK_ACCOUNTS WHERE account_number = 73452345);
UPDATE BANK_ACCOUNTS
SET overdraft_id = NULL
WHERE account_number = 73452345;