INSERT INTO pokemart (city, region, street_address, phone_num)
VALUES ('Ecruteak City', 'Kalos', '123 Pikachu Road', '111-1111'), ('Jubilife City', 'Kanto', '41 Mossy Pavilion', '222-1112'), ('Pewter City', 'Alola', '912 Snovy Circle', '333-1114'), ('Pallet Town', 'Johto', '988 Surf Avenue', '334-1115'), ('Viridian City', 'Kalos', '8144 Chilly Lane', '448-7841');

INSERT INTO mart_balance_history (balance, mart_id, balance_date)
VALUES (5000.04, 1, '2024-02-12 10:14:12'), (50413.68, 2, '2024-02-16 09:06:18'), (900.12, 3, '2024-03-11 11:56:51'), (8463.05, 4, '2024-0-07 07:41:02'), (1665.24, 5, '2024-03-06 04:54:52');

INSERT INTO trainer_card (balance, badge_level, trainer_fname, trainer_lname, trainer_phone)
VALUES (0.00, 0, 'Chate', 'Chondero', '789-1011'),
        (0.00, 5, 'Charlos', 'Chonzalez', '984-2154'),
        (15.25, 2, 'Chyler', 'Chox', '412-2314'),
        (62.14, 3, 'Chett', 'Chisher', '202-2020'),
        (1000.00, 0, 'Chon', 'Chames', '456-9544');

INSERT INTO vendor (vendor_name, vendor_contact, vendor_phone)
VALUES ('Sylph Co.', 'Steven Stone', '123-9650'),
        ('Devon Corporation', 'Von Johnson', '452-0211'),
        ('Bike Shop', 'Gravy Birkens', '001-0101'),
        ('Oatmeal Store', 'Terry Crop', '104-9995'),
        ('Juice Store', 'Judy Nust', '263-6321');

INSERT INTO product (prod_code, vendor_id, prod_name, prod_descript, unit_price, min_qty, req_badges, vendor_price)
VALUES ('PB', 1, 'Poke Ball', 'Basic tool for catching Pokemon', 10.00, 1000, 0, 5.00),
        ('GB', 1, 'Great Ball', 'Better catch rate than Poke Ball', 20.00, 500, 3, 10.00),
        ('UB', 1, 'Ultra Ball', 'Better catch rate than an Ultra Ball', 30.00, 500, 5, 15.00),
        ('BP', 2, 'Potion', 'Heals 20 HP', 5.00, 2000, 0, 2.50),
        ('SP', 2, 'Super Potion', 'Heals 50 HP', 7.25, 1500, 2, 3.62);

INSERT INTO employee (emp_fname, emp_lname, emp_phone)
VALUES ('Jerry', 'Stink', '411-4142'),
        ('Benjamin', 'Purvis', '300-4980'),
        ('Sticky', 'Davis', '825-9697'),
        ('Sean', 'Gwork', '744-5226'),
        ('Stefon', 'Schwikert', '362-4700');

INSERT INTO certification (cert_payrate, cert_title, cert_descript)
VALUES (10.00, 'Front counter', 'Point of sales representative'),
        (12.00, 'Stocker', 'Stock shelves and receive shipments'),
        (20.00, 'Manager', 'Manages the other employees'),
        (0.00, 'Pokemon', 'A Pokemon used for labor'),
        (40.00, 'Store Manager', 'Manage store operations');

INSERT INTO certification_record (emp_id, cert_id)
VALUES (1, 1), (2, 2), (3,3), (4,4), (5,5);

INSERT INTO shift (emp_id, hours_worked, mart_id, cert_id)
VALUES (1, 6, 1, 1), (2, 4, 5, 2), (3, 12, 3, 3), (4, 5, 2, 4), (5, 6, 5, 5);

INSERT INTO invoice (trainer_id, emp_id, mart_id)
VALUES (1, 1, 1), (2, 1, 1), (3, 1, 1), (4, 1, 1), (5, 1, 1);

INSERT INTO line (invoice_num, line_num, prod_code, qty)
VALUES (1, 1, 'PKB', 1),
        (1, 2, 'GB', 1),
        (1, 3, 'UB', 1),
        (1, 4, 'BP', 1),
        (1, 5, 'SP', 1);

INSERT INTO stock_history (prod_code, mart_id, stock_qty, stock_date)
VALUES  ('PB', 1, 2741, '2024-03-02 10:00:00'),
        ('PB', 1, 2722, '2024-03-02 10:10:00'),
        ('PB', 1, 2699, '2024-03-02 10:20:00'),
        ('GB', 1, 2401, '2024-03-05 11:15:37'),
        ('UB', 1, 600, '2024-03-01 08:04:17'),
        ('BP', 1, 4125, '2024-03-06 09:32:17'),
        ('SP', 1, 1520, '2024-2-28 06:44:02'),
        ('PB', 2, 1945 , '2024-03-02 10:40:00'),
        ('GB', 2, 1879, '2024-03-05 11:15:37'),
        ('UB', 2, 700, '2024-03-01 08:04:17'),
        ('BP', 2, 3560, '2024-03-06 09:32:17'),
        ('SP', 2, 4512, '2024-2-28 06:44:02'),
        ('PB', 3, 2245 , '2024-03-02 10:40:00'),
        ('GB', 3, 2013, '2024-03-05 11:15:37'),
        ('UB', 3, 600, '2024-03-01 08:04:17'),
        ('BP', 3, 4125, '2024-03-06 09:32:17'),
        ('SP', 3, 1520, '2024-2-28 06:44:02'),
        ('PB', 4, 2741 , '2024-03-02 10:40:00'),
        ('GB', 4, 2401, '2024-03-05 11:15:37'),
        ('UB', 4, 844, '2024-03-01 08:04:17'),
        ('BP', 4, 4125, '2024-03-06 09:32:17'),
        ('SP', 4, 1520, '2024-2-28 06:44:02'),
        ('PB', 5, 2741 , '2024-03-02 10:40:00'),
        ('GB', 5, 2401, '2024-03-05 11:15:37'),
        ('UB', 5, 678, '2024-03-01 08:04:17'),
        ('BP', 5, 4125, '2024-03-06 09:32:17'),
        ('SP', 5, 1520, '2024-2-28 06:44:02');




