UPDATE BANK_ACCOUNTS
SET is_locked =
    CASE WHEN balance < 0 OR (SELECT OVERDRAFTS.credit_sum FROM OVERDRAFTS WHERE OVERDRAFTS.id = overdraft_id) > 200
    THEN 1
    ELSE 0
    END;
    