/* Program name: main.cpp
*  Author: Nate Mondero
*  Date last updated: 3/5/2024
* Purpose: This program provides user interface with the pokemart.db database. Users can insert, delete from, and update select tables. Users can intiate a transaction to process a sale. Users can 
*/

#include <iostream>
#include <string>
#include <limits>
#include <sqlite3.h>
#include <regex>
#include <iomanip>

const std::regex PHONE_FORMAT("\\d{3}-\\d{4}"); //Declare a constant regular expression to define the proper phone number formart (###-####)
const int QUIT = -1; //Declare a constant to hold the quit value for main menu
const int MAX_BADGES = 8; //Declare int constant to define the maximum number of badges a trainer can  

//Function prototypes
//Note: Im grouping these together based on the project requirements as best as I can (there is some looseness)
void printMainMenu(); //Prints main menu
int mainMenuChoice(); //Get main menu choice

//Insert related functions
void insertIntoTable(sqlite3 *);
void addTrainerCard(sqlite3 *);
void addEmployee(sqlite3 *);

//Update related functions
int selectPerson(sqlite3 *, std::string, std::string, std::string);
void updateTable(sqlite3 *);
void updateTrainerCard(sqlite3 *);
void updateEmployee(sqlite3 *);

//Delete related functions
void deleteFromTable(sqlite3 *);
void deleteTrainerCard(sqlite3 *);
void deleteEmployee(sqlite3 *);

//Transaction related
int selectPokemart(sqlite3 *);
void makeSale(sqlite3 *);
int insertLine(sqlite3 *, int, int, int, int, double &);
int selectProduct(sqlite3 *, int, int &, double &, double &);
int insertStockHistory(sqlite3 *, std::string, int, int);
int updateBalances(sqlite3 *, int , int , double, double);

//User reports
void viewInvoice(sqlite3 *);
void viewCertificates(sqlite3 *);

//SQL wrapper functions
int startTransaction(sqlite3 *);
int rollback(sqlite3 *);
int commit(sqlite3 *);

//Reset instream failstate
void resetStreamCheck(std::istream &);

//Start of main
int main()
{
	//Declarations
	int choice; //Main menu choice
	int rc; //Return code variable
	sqlite3 *pkdb; //Pokemart database pointer

	//Attempt to open pokemart database, quit if fail
	rc = sqlite3_open_v2("pokemart.db", &pkdb, SQLITE_OPEN_READWRITE, NULL);
	if(rc != SQLITE_OK){
		std::cout << "Error opening database (main): " << sqlite3_errmsg(pkdb) << std::endl;
		sqlite3_close(pkdb);
		return 0;
	}

	std::cout << "Welcome to PokeMart Database" << std::endl; //Welcome message

	choice = mainMenuChoice(); //Get the first menu selection

	//Main Loop. Quits if the user inputs the QUIT constant into the main menu selection
	while(choice != QUIT){
		switch(choice){
		case 1:	insertIntoTable(pkdb); break;
		case 2:	updateTable(pkdb); break;
		case 3:	deleteFromTable(pkdb); break;
		case 4:	makeSale(pkdb); break;
		case 5: viewInvoice(pkdb); break;
		case 6:	viewCertificates(pkdb); break;
		}
		choice = mainMenuChoice();
	}

	sqlite3_close(pkdb); //Close the database
	return 0;
}

//Prints the main menu options
void printMainMenu(){
	std::cout << "Please select an option (enter -1 to quit): " << std::endl;
	std::cout << "1. Add to a table" << std::endl;
	std::cout << "2. Update a table" << std::endl;
	std::cout << "3. Delete from a table" << std::endl;
	std::cout << "4. Make a sale" << std::endl;
	std::cout << "5. View invoice" << std::endl; //One of the user report options. Joins invoice, line, pokemart, and employee, trainer_card, and product tables
	std::cout << "6. View certificate records" << std::endl; //One of the user report options. Join certification, employee, and certification_history

}

//Gets the users main menu selection and returns the int choice value
int mainMenuChoice(){
	int choice; //Declare variable to input menu choice

	//Print main menu and get menu choice
	printMainMenu();
	std::cin >> choice;

	//Validate menu choice
	while(!std::cin || choice > 6 || (choice < 1 && choice != QUIT)){
		resetStreamCheck(std::cin);
		std::cout << "Invalid menu option selected. Please select an option from the menu." << std::endl; 
		std::cin >> choice;
	}
    return choice; //Return the valid user choice to main function
}

//This function selects which table to insert into
void insertIntoTable(sqlite3 *db)
{
	int choice; //Variable for menu option choice

	//Print options menu, get user selection and validate input
	std::cout << "Please choose a table addition to perform:" << std::endl;
	std::cout << "1. Add to trainer_card" << std::endl;
	std::cout << "2. Add to employee" << std::endl;
	std::cout << "3. Return to main menu" << std::endl;
	std::cin >> choice;
	while(!std::cin || choice < 1 || choice > 3){
		resetStreamCheck(std::cin);
		std::cout << "Invalid menu option selected. Please select an option from the menu." << std::endl;
		std::cin >> choice;
	}
	
	if(choice == 3){return;} //Return to the main menu if user selects 3

	//Execute the function corresponding to the users choice
	switch(choice){
	case 1:	addTrainerCard(db); break;
	case 2:	addEmployee(db); break;
	}
}

//Get information about a new trainer card and insert it into the trainer_card table
void addTrainerCard(sqlite3 *db)
{
	//Variables to hold trainer card info
	int badgeLevel;
	std::string fname, lname, phone;

	//Get trainer card info needed for insert from user
	std::cout << "Enter the first name of trainer to add: ";
	std::cin >> fname;
	std::cout << "Enter " << fname << "'s last name: ";
	std::cin >> lname;
	std::cout << "Enter " << fname << "'s badge level (0 - " << MAX_BADGES << "):" << std::endl;
	std::cin >> badgeLevel;
	while(!std::cin || badgeLevel < 0 || badgeLevel > MAX_BADGES){ //Verify badge level input is between 0 and the max amount of badges you can have
		resetStreamCheck(std::cin);
		std::cout << "Invalid badge count entered. Valid badges counts are between 0 and " << MAX_BADGES << ". Please try again." << std::endl;
		std::cin >> badgeLevel;
	}
	std::cout << "Enter " << fname << "'s phone number (###-####):" << std::endl;
	std::cin >> phone;
	while(!std::regex_match(phone, PHONE_FORMAT)){ //Verify phone input is in the required format for the database according to the phoneFormat regex
		std::cout << "Invalid phone number entered. Please enter the phone number as ###-#### (ex. 123-4567):" << std::endl;
		std::cin >> phone;
	}

	//Declared INSERT query with parameterized values
	std::string query = "INSERT INTO trainer_card (trainer_fname, trainer_lname, badge_level, trainer_phone) ";
	query += "VALUES (@fname, @lname, @badge, @phone)";
	sqlite3_stmt *res; //Declare result variable for the following prepare_v2 function
	
	//Attempt to prepare the query. Return if it fails
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Unable to insert trainer card: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}

	//Bind trainer card info from user to INSERT parameters
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@fname"), fname.c_str(), -1, SQLITE_STATIC);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Unable to bind a trainer card variable: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@lname"), lname.c_str(), -1, SQLITE_STATIC);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Unable to bind a trainer card variable: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@phone"), phone.c_str(), -1, SQLITE_STATIC);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Unable to bind a trainer card variable: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}
	rc = sqlite3_bind_int(res, sqlite3_bind_parameter_index(res, "@badge"), badgeLevel);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Unable to bind a trainer card variable: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}
	
	int choice; //Declare a variable for the following menu choice
	//Print out the user generated info to verify the information is correct before INSERTING
	std::cout << "Is this information correct?" << std::endl;
	std::cout << "Name: " << fname << " " << lname << std::endl;
	std::cout << "Phone: " << phone << std::endl;
	std::cout << "Badge Count: " << badgeLevel << std::endl;
	std::cout << "1. Yes" << std::endl;
	std::cout << "2. No" << std::endl;
	std::cin >> choice;
	while(!std::cin || choice < 1 || choice > 2){ //Input validation
		resetStreamCheck(std::cin);
		std::cout << "Invalid entry. Please try again." << std::endl;
		std::cin >> choice;
	}

	//Finalize res variable and return to main menu if information not correct
	if(choice == 2){ 
		sqlite3_finalize(res);
		std::cout << "Cancelling trainer_card insert" << std::endl;
		std::cout << std::endl;
		return;
	}

	rc = sqlite3_step(res); //Execute the INSERT SQL
	if(rc != SQLITE_DONE){ //Finalize res and return if there was an error inserting
		sqlite3_finalize(res);
		std::cout << "Error inserting into trainer_card: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	sqlite3_finalize(res); //Finalize res after a successful insert
	std::cout << "Successfully inserted into trainer_card" << std::endl;
	std::cout << std::endl;
}

//Get information about a new employee to insert into the employee table
void addEmployee(sqlite3 *db){
	std::string fname, lname, phone; //Declare variables to hold employee info

	//Get employee info from user
	std::cout << "Enter the first name of employee to add: ";
	std::cin >> fname;
	std::cout << "Enter " << fname << "'s last name: ";
	std::cin >> lname;
	std::cout << "Enter " << fname << "'s phone number (###-####):" << std::endl;
	std::cin >> phone;
	while(!std::regex_match(phone, PHONE_FORMAT)){ //Validate that the inputted phone number is in the correct format according to the phoneFormat regex
		std::cout << "Invalid phone number entered. Please enter the phone number as ###-#### (ex. 123-4567):" << std::endl;
		std::cin >> phone;
	}

	//Declare INSERT query for employee table with parameterized values
	std::string query = "INSERT INTO employee (emp_fname, emp_lname, emp_phone) ";
	query += "VALUES (@fname, @lname, @phone)";
	sqlite3_stmt *res; //Declare result variable for following prepare_v2 function
	
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL); //Attempt prepare_v2 with the query, return if unsuccessful
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Unable to insert employee: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}

	//Bind employee values to parameters in the query
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@fname"), fname.c_str(), -1, SQLITE_STATIC);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Unable to bind an employee variable: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@lname"), lname.c_str(), -1, SQLITE_STATIC);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Unable to bind an employee variable: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@phone"), phone.c_str(), -1, SQLITE_STATIC);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Unable to bind an employee variable: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}
	
	int choice; //Declare choice variable for the following selection
	//Ask user to verify the information is correct before inserting
	std::cout << "Is this information correct?" << std::endl;
	std::cout << "Name: " << fname << " " << lname << std::endl;
	std::cout << "Phone: " << phone << std::endl;
	std::cout << "1. Yes" << std::endl;
	std::cout << "2. No" << std::endl;
	std::cin >> choice; //Get user selection and verify the input
	while(!std::cin || choice < 1 || choice > 2){
		resetStreamCheck(std::cin);
		std::cout << "Invalid entry. Please try again." << std::endl;
		std::cin >> choice;
	}

	if(choice == 2){ //If information is not correct, cancel the insert and return to main menu
		sqlite3_finalize(res);
		std::cout << "Cancelling employee insert" << std::endl;
		std::cout << std::endl;
		return;
	}

	rc = sqlite3_step(res); //Execute the INSERT query
	if(rc != SQLITE_DONE){ //Return to main menu with error message if INSERT query is unsuccessful
		sqlite3_finalize(res);
		std::cout << "Error inserting into employee: " << sqlite3_errmsg(db) << std::endl;
		return;
	}
	sqlite3_finalize(res); //Finalize the result variable if the INSERT was successful
	std::cout << "Successfully inserted into employee" << std::endl;
}

int selectPerson(sqlite3 *db, std::string tableName, std::string attributePrefix, std::string context)
{
	sqlite3_stmt *res; //Declare a statement result variable
	std::string query = "SELECT " + attributePrefix + "_id, " + attributePrefix + "_fname || ' ' || " + attributePrefix + "_lname FROM " + tableName + " ORDER BY " + attributePrefix + "_id"; //Declare query to return a list of trainer cards in the trainer_card table
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL); //Attempt to prepare the query. Return if unsuccessful
	if(rc != SQLITE_OK){
		std::cout << "Error selecting from " << tableName << ": " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return -1;
	}

    // Check if there are trainer cards to update in the table
    if (sqlite3_step(res) == SQLITE_DONE) {
        std::cout << "No " << tableName << "s to select. " << tableName << " requires at least one record for this action. Try to insert a new record into " << tableName << " first." << std::endl;
        sqlite3_finalize(res);
        return -1;
    }
	sqlite3_reset(res);

	std::cout << "Select the " << tableName << " for the " << context << ": " << std::endl; //Prompt to select a trainer card from the menu printed below
	int count = 0, choice; //Declare a count variable to count the rows in trainer_card. choice is declared for user selection
	sqlite3_step(res); //Get the first row of the select statement
	do{ //Get the rest of the rows
		count++; //Increment row count
		std::cout << count << ". " << sqlite3_column_int(res, 0) << " - " << sqlite3_column_text(res, 1) << std::endl; //Output trainer id and name
		rc = sqlite3_step(res); //Step to the next row
	}while(rc == SQLITE_ROW);

	std::cin >> choice; //Get the users choice of trainer card and verify input
	while(!std::cin || choice < 1 || choice > count){
		resetStreamCheck(std::cin);
		std::cout << "Invalid selection. Please try again." << std::endl;
		std::cin >> choice;
	}
	
	//Reset the result variable and step to the specifed trainer_card row
	sqlite3_reset(res);
	for(int i = 0; i < choice; i++){
		sqlite3_step(res);
	}
	std::string rowID = reinterpret_cast<const char *>(sqlite3_column_text(res,0)); //Extract the trainer_id from the relevant row
	sqlite3_finalize(res); //Finalize the result
	
	return stoi(rowID);
}

//This function selects a table to update
void updateTable(sqlite3 *db){
	int choice; //Declare menu selection variable

	//Prompt for table to update and validate input
	std::cout << "Please select a table update to perform:" << std::endl;
	std::cout << "1. trainer_card" << std::endl;
	std::cout << "2. employee" << std::endl;
	std::cout << "3. Return to main menu" << std::endl;
	std::cin >> choice; //Get user choice and validate the input
	while(!std::cin || choice < 1 || choice > 3){
		resetStreamCheck(std::cin);
		std::cout << "Invalid entry. Please select an option from the menu:" << std::endl;
		std::cin >> choice;
	}
	if(choice == 3){return;} //Return to main menu if user enters 3

	//Choose update function based on user selection
	switch(choice){
	case 1: updateTrainerCard(db); break;
	case 2: updateEmployee(db); break;
	}
}

//This function selects the attribute from trainer_card to update, then attempts the update on that attribute with a value provided by the user
void updateTrainerCard(sqlite3 *db){
	int trainerID = selectPerson(db, "trainer_card", "trainer", "update");
	if(trainerID == -1){return;}

	int choice;
	//Prompt to choose which attribute to update
	std::cout << "Select the attribute to update:" << std::endl;
	std::cout << "1. Balance" << std::endl;
	std::cout << "2. Badge Count" << std::endl;
	std::cout << "3. Phone Number" << std::endl;
	std::cout << "4. Return to main menu" << std::endl;
	std::cin >> choice; //Get the choice and verify
	while(!std::cin || choice < 1 || choice > 4){
		resetStreamCheck(std::cin);
		std::cout << "Invalid entry. Please try again." << std::endl;
		std::cin >> choice;
	}

	if(choice == 4){return;} //Return if user selects return to main menu

	int rc;
	sqlite3_stmt *res;
	
	//Choose which update to perform based on users 
	std::string query;
	switch(choice){
	case 1: //Update the trainer balance
		double balance;
		query = "UPDATE trainer_card SET balance = @balance WHERE trainer_id = " + std::to_string(trainerID); //Declare update query with specified trainer ID
		rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
		if(rc != SQLITE_OK){
			sqlite3_finalize(res);
			std::cout << "Error updating trainer card balance: " << sqlite3_errmsg(db) << std::endl;
			std::cout << query;
			return;
		}
		std::cout << "Enter the new balance" << std::endl; //Get new balance and verify input
		std::cin >> balance;
		while(!std::cin || balance < 0){
			resetStreamCheck(std::cin);
			std::cout << "Invalid balance entered. Please try again." << std::endl;
			std::cin >> balance;
		}
		rc = sqlite3_bind_double(res, sqlite3_bind_parameter_index(res, "@balance"), balance); //Attempt to bind the new balance to the update sql query
		if(rc != SQLITE_OK){
			sqlite3_finalize(res);
			std::cout << "Error binding balance parameter: " << sqlite3_errmsg(db) << std::endl;
			return;
		}
		
		rc = sqlite3_step(res); //Execute the update
		if(rc != SQLITE_DONE){
			sqlite3_finalize(res);
			std::cout << "Error executing the trainer card balance update query: " << sqlite3_errmsg(db) << std::endl;
			return;
		}
		std::cout << "Updated balance for trainer " << trainerID << std::endl;
		break;

	case 2: //Update the badge count (badge level)
		int badge;
		query = "UPDATE trainer_card SET badge_level = @badge WHERE trainer_id = " + std::to_string(trainerID); //Declare update query with specified trainer ID
		rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL); //Attempt to prepare the query
		if(rc != SQLITE_OK){
			sqlite3_finalize(res);
			std::cout << "Error updating trainer card badge count: " << sqlite3_errmsg(db) << std::endl;
			std::cout << query;
			return;
		}
		std::cout << "Enter the new badge count (0 - 8)" << std::endl; //Get the updated badge count and verify the input
		std::cin >> badge;
		while(!std::cin || badge < 0 || badge > MAX_BADGES){
			resetStreamCheck(std::cin);
			std::cout << "Invalid badge count entered. Please try again (0 - 8)." << std::endl;
			std::cin >> badge;
		}
		rc = sqlite3_bind_int(res, sqlite3_bind_parameter_index(res, "@badge"), badge); //Attempt to bind the new badge count to the query
		if(rc != SQLITE_OK){
			sqlite3_finalize(res);
			std::cout << "Error binding badge count parameter: " << sqlite3_errmsg(db) << std::endl;
			return;
		}
		sqlite3_step(res); //Execute the update
		if(rc != SQLITE_DONE){
			sqlite3_finalize(res);
			std::cout << "Error executing the trainer card badge count update query: " << sqlite3_errmsg(db) << std::endl;
			return;
		}

		std::cout << "Updated badge count for trainer " << trainerID << std::endl;
		break;

	case 3: //Update the phone number
		std::string phone;
		query = "UPDATE trainer_card SET trainer_phone = @phone WHERE trainer_id = " + std::to_string(trainerID); //Declare SQL update for phone number with specified trained ID
		rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL); //Attempt to prepare the query
		if(rc != SQLITE_OK){
			sqlite3_finalize(res);
			std::cout << "Error updating trainer phone: " << sqlite3_errmsg(db) << std::endl;
			std::cout << query;
			return;
		}
		std::cout << "Enter the phone number (###-####)" << std::endl; //Prompt for the new phone number and verify the input
		std::cin >> phone;
		while(!std::regex_match(phone, PHONE_FORMAT)){
			std::cout << "Invalid phone number entered. Please try again (Please use this format ###-#### ex. 123-4567)" << std::endl;
			std::cin >> phone;
		}
		rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@phone"), phone.c_str(), -1, SQLITE_STATIC); //Attempt to bind the new phone number to the update query
		if(rc != SQLITE_OK){
			sqlite3_finalize(res);
			std::cout << "Error binding phone number parameter: " << sqlite3_errmsg(db) << std::endl;
			return;
		}
		sqlite3_step(res); //Execute the phone update
		if(rc != SQLITE_DONE){
			sqlite3_finalize(res);
			std::cout << "Error executing the trainer card phone number update query: " << sqlite3_errmsg(db) << std::endl;
			return;
		}
		std::cout << "Updated phone number for trainer " << trainerID << std::endl;
		break;
	}
	std::cout << std::endl; //Add an extra newline before the main menu
}

//This function attempts to update the employee table at a specified employee id. This function will only update the employees phone number
void updateEmployee(sqlite3 *db){
	sqlite3_stmt *res;
	int empID = selectPerson(db, "employee", "emp", "update");
	if(empID == -1){return;}

	std::string phone; //Declare string to hold new phone number
	std::cout << "Enter the new phone number (###-####):" << std::endl; //Prompt for new phone number and verify input according to PHONE_FORMAT regex
	std::cin >> phone;
	while(!std::regex_match(phone, PHONE_FORMAT)){
		std::cout << "Invalid phone number entered. Please try again (Please use this format ###-#### ex. 123-4567)" << std::endl; //Remind user of the proper format
		std::cin >> phone;
	}
	
	std::string query = "UPDATE employee SET emp_phone = @phone WHERE emp_id = " + std::to_string(empID); //Declare new query to update the phone number of the specified employee
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL); //Attempt to prepare the query
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error updating employee: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query;
		return;
	}
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@phone"), phone.c_str(), -1, SQLITE_STATIC); //Attempt to bind the new phone number to the update query
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding employee phone for update: " << sqlite3_errmsg(db);
		return;
	}

	rc = sqlite3_step(res); //Execute the UPDATE query
	if(rc != SQLITE_DONE){
		sqlite3_finalize(res);
		std::cout << "Error executing the employee phone number update query: " << sqlite3_errmsg(db) << std::endl;
		return;
	}
	std::cout << "Employee phone number updated" << std::endl;
	std::cout << std::endl; //Add extra newline before the main menu
}

//This function selects which table to delete from 
void deleteFromTable(sqlite3 *db){
	int choice; //Choice variable for the following menu selection
	
	std::cout << "Select which table to delete from:" << std::endl;
	std::cout << "1. trainer_card" << std::endl;
	std::cout << "2. employee" << std::endl;
	std::cout << "3. Return to main menu" << std::endl;
	std::cin >> choice; //Get the choice from the user, validate the input
	while(!std::cin || choice < 1 || choice > 3){
		resetStreamCheck(std::cin);
		std::cout << "Invalid entry. Please try again." << std::endl;
		std::cin >> choice;
	}

	if(choice == 3){return;} //Return to main menu if user selects 3
	
	switch(choice){ //Execute the necessary delete function according to the users choice
	case 1: deleteTrainerCard(db); break;
	case 2: deleteEmployee(db); break;
	}
}


void deleteTrainerCard(sqlite3 *db){
	int trainerID = selectPerson(db, "trainer_card", "trainer", "delete"); //Get id of trainer to delete
	if(trainerID == -1){return;} //Return if failure occurred

	//Prepare our query to execute
	sqlite3_stmt *res;
	std::string query = "DELETE FROM trainer_card WHERE trainer_id = @trainerID";
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error with trainer_card delete: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}
	rc = sqlite3_bind_int(res, sqlite3_bind_parameter_index(res, "@trainerID"), trainerID); //Bind trainerID to the query
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding trainer_id to delete query: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	//Execute the query
	rc = sqlite3_step(res);
	if(rc != SQLITE_DONE){ //Check to see if the query was successful
		sqlite3_finalize(res);
		std::cout << "Error executing the trainer card delete query: " << sqlite3_errmsg(db) << std::endl;
		return;
	}
	sqlite3_finalize(res); //Finalize result
	std::cout << "Deleted trainer card ID " << trainerID << std::endl;
	std::cout << std::endl;
}

void deleteEmployee(sqlite3 *db){
	//Prepare SQL to select employee to delete
	sqlite3_stmt *res; //Declare a query result variable
	std::string query = "SELECT emp_id, emp_fname || ' ' || emp_lname FROM employee ORDER BY emp_id"; 
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error selecting from employee: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}

    // Check if there are no employees to delete
    if (sqlite3_step(res) == SQLITE_DONE) {
        std::cout << "No employees to delete." << std::endl;
        sqlite3_finalize(res);
        return;
    }
	sqlite3_reset(res); //Reset result after the previous step that checks if the table has rows

	std::cout << "Select the employee to delete:" << std::endl; //Prompt for employee to delete
	int count = 0, choice; //count will count the rows in employee. choice will hold the users employee choice
	sqlite3_step(res); //Get the first row from the SELECT query
	do{
		count++; //Increment the count
		std::cout << count << ". " << sqlite3_column_int(res, 0) << " - " << sqlite3_column_text(res, 1) << std::endl; //Output the employee ID and name
		rc = sqlite3_step(res); //Step to the next row
	}while(rc == SQLITE_ROW);

	std::cin >> choice; //Get the user choice and verify input
	while(!std::cin || choice < 1 || choice > count){
		resetStreamCheck(std::cin);
		std::cout << "Invalid selection. Please try again." << std::endl;
		std::cin >> choice;
	}
	
	sqlite3_reset(res); //Reset the result then step to the selected row according to choice
	for(int i = 0; i < choice; i++){
		sqlite3_step(res);
	}
	std::string empID = reinterpret_cast<const char *>(sqlite3_column_text(res,0)); //Extract the employee ID at the user specified row
	sqlite3_finalize(res); //Finalize the result variable

	//Prepare SQL to delete specified employee
	query = "DELETE FROM employee WHERE emp_id = @empID";
	rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error with employee delete: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@empID"), empID.c_str(), -1 , SQLITE_STATIC); //Bind empID to the query
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding emp_id to delete query: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	//Execute the delete query, check that it worked
	rc = sqlite3_step(res);
	if(rc != SQLITE_DONE){
		sqlite3_finalize(res);
		std::cout << "Error executing the employee delete query: " << sqlite3_errmsg(db) << std::endl;
		return;
	}
	sqlite3_finalize(res); //Finalize result
	std::cout << "Deleted employee ID " << empID << std::endl;
	std::cout << std::endl;
}

int selectPokemart(sqlite3 *db){
	//Prepare SQL query to select a PokeMart
	sqlite3_stmt *res; //Declare a statement result variable
	std::string query = "SELECT mart_id, street_address || ' - ' || city || ' , ' || region FROM pokemart" ; 
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL); //Attempt to prepare the query. Return if unsuccessful
	if(rc != SQLITE_OK){
		std::cout << "Error selecting PokeMart: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return -1;
	}

    // Check if there are PokeMarts to select in the table
    if (sqlite3_step(res) == SQLITE_DONE) {
        std::cout << "No PokeMarts to select. PokeMart requires at least one record for this action." << std::endl;
        sqlite3_finalize(res);
        return -1;
    }
	sqlite3_reset(res); //reset res

	std::cout << "Select the PokeMart for the invoice: " << std::endl; //Prompt to select a pokemart from the menu printed below
	int count = 0, choice; //Declare a count variable to count the rows in pokemart. choice is declared for user selection
	sqlite3_step(res); //Get the first row of the select statement
	do{ //Get the rest of the rows
		count++; //Increment row count
		std::cout << count << ". " << sqlite3_column_int(res, 0) << " - " << sqlite3_column_text(res, 1) << std::endl; //Output pokemart and name
		rc = sqlite3_step(res); //Step to the next row
	}while(rc == SQLITE_ROW);

	std::cin >> choice; //Get the users choice of trainer card and verify input
	while(!std::cin || choice < 1 || choice > count){
		resetStreamCheck(std::cin);
		std::cout << "Invalid selection. Please try again." << std::endl;
		std::cin >> choice;
	}
	
	//Reset the result variable and step to the specifed pokemart row
	sqlite3_reset(res);
	for(int i = 0; i < choice; i++){
		sqlite3_step(res);
	}
	std::string martID = reinterpret_cast<const char *>(sqlite3_column_text(res,0)); //Extract the pokemart from the relevant row
	sqlite3_finalize(res); //Finalize the result
	
	return stoi(martID);
}

//This function is a transaction that attempts to make a sale. This entails many queries, including inserting a new invoice and invoice lines, updating values in
//trainer_id, mart_balance_history, stock_history, and other functions
void makeSale(sqlite3 *db){
	int rc = startTransaction(db); //Attempt to start the transaction, quit if unsuccessful
	if(rc != SQLITE_OK){
		std::cout << "Unable to start transaction" << std::endl;
		return;
	}

	//Attempt to get the attributes for the new invoice, rollback and return if unsuccessful with any
	int trainerID = selectPerson(db, "trainer_card", "trainer", "invoice");
	if(trainerID == -1){
		rollback(db);
		std::cout << "Cancelling sale" << std::endl;
		return;
	}
	int empID = selectPerson(db, "employee", "emp", "invoice");
	if(empID == -1){
		rollback(db);
		std::cout << "Cancelling sale" << std::endl;
		return;
	}
	int martID = selectPokemart(db);
	if(martID == -1){
		rollback(db);
		std::cout << "Cancelling sale" << std::endl;
		return;
	}

	//Declare query to insert a new invoice with the info collected above
	std::string query = "INSERT INTO invoice (trainer_id, emp_id, mart_id) ";
	query += "VALUES (@trainerID, @empID, @martID)";
	sqlite3_stmt *res;
	rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL); //Attempt to prepare the query, rollback and return on fail
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		rollback(db);
		std::cout << "Error inserting invoice: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}
	
	//Attempt to bind values to the query, rollback and return on any fails
	rc = sqlite3_bind_int(res, sqlite3_bind_parameter_index(res, "@trainerID"), trainerID);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		rollback(db);
		std::cout << "Error binding trainer ID to invoice: " << sqlite3_errmsg(db) << std::endl;
		return;
	}
	rc = sqlite3_bind_int(res, sqlite3_bind_parameter_index(res, "@empID"), empID);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		rollback(db);
		std::cout << "Error binding employee ID to invoice: " << sqlite3_errmsg(db) << std::endl;
		return;
	}
	rc = sqlite3_bind_int(res, sqlite3_bind_parameter_index(res, "@martID"), martID);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		rollback(db);
		std::cout << "Error binding PokeMart ID to invoice: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	//Execute the invoice insert and check if it worked. Rollback and return on fail
	rc = sqlite3_step(res); 
	if(rc != SQLITE_DONE){
		sqlite3_finalize(res);
		rollback(db);
		std::cout << "Error executing the invoice insert: " << sqlite3_errmsg(db) << std::endl;
		return;
	}
	int invoiceID = sqlite3_last_insert_rowid(db); //Extract the invoiceID of the invoice we added so we can create records in the line table referencing the new invoice
	sqlite3_finalize(res); //Finalize the result vairalbe

	int choice; //User choice variable to keep adding new lines or not
	int lineCount = 1; //We start with the first line in the invoice
	double subtotal = 0; //Declare a subtotal that accumulates according to the total price of each added line. This will be used to update the mart_balance_history and trainer_balance after all products have been selected
	do{
		rc = insertLine(db, invoiceID, trainerID, martID, lineCount, subtotal); //Attempt to insert a new line by selecting a product and the quantity to purchase
		if(rc != SQLITE_OK){ 
			rollback(db);
			return;
		}
		
		lineCount++; //Increment the line count

		std::cout << "Would you like to add more items to the invoice? Current invoice total is $" << subtotal << std::endl; //Ask the user if they would like to add more lines to the invoice
		std::cout << "1. Yes" << std::endl;
		std::cout << "2. No" << std::endl;
		std::cin >> choice; //Get the choice and verify input
		while(!std::cin || choice < 1 || choice > 2){
			resetStreamCheck(std::cin);
			std::cout << "Invalid entry. Please try again." << std::endl;
			std::cin >> choice;
		}
	}while(choice != 2);  //Exit do-while when user selects 2
	commit(db); //Commit all changes to the database and return to main menu
	return;
}

//This function will gather information to insert a new line into an invoice being created in makeSale
int insertLine(sqlite3 *db, int invoiceID, int trainerID, int martID, int lineCount, double &subtotal){
	int stockQty; //Hold the stock quantity

	//At this point, I want to find the most recent balance at the specified pokemart so that i can modify the mart_balance_history properly later on
	sqlite3_stmt *res; //Declare statement result variable
	std::string query = "SELECT balance FROM mart_balance_history WHERE balance_date = "; //Declare query to select the most recent balance history for the specified PokeMArt
	query += "(SELECT MAX(balance_date) FROM mart_balance_history WHERE mart_id = @martID)";
	
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL); //Attempt to prepare query, quit with error code indicating rollback if unsuccessful
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error selecting most recent balance_history at PokeMart " << martID << ": " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return -1;
	}
	rc = sqlite3_bind_int(res, sqlite3_bind_parameter_index(res, "@martID"), martID); //Attempt to bind to a parameter in the query, return and rollback if unsuccessful
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding mart ID to balance_history query: " << std::endl;
		return -1;
	}

	sqlite3_step(res); //Execute the SELECT query
	double balanceBefore = std::stod(reinterpret_cast<const char *>(sqlite3_column_text(res, 0))); //Extract the balance before the order
	rc = selectProduct(db, martID, stockQty, subtotal, balanceBefore); //Attempt to select a product, return with a fail code if unsuccessful
	if(rc != SQLITE_OK){return -1;}
	
	double balanceAfter = balanceBefore; //Declare a new var to hold balanceBefore with the clarification that its value now represents the value after selecting the line of products
	sqlite3_finalize(res); //Finalize the result
	
	rc = updateBalances(db, trainerID, martID, subtotal, balanceAfter); //Attempt to update the trainer balance. Also, insert a new mart_balance_history record
	if(rc != SQLITE_OK){return -1;} 

    return SQLITE_OK; //Return SQLITE_OK if no errors encountered
}

int selectProduct(sqlite3 *db, int martID, int &stockQty, double &subtotal, double &balanceBefore){
	sqlite3_stmt *res; //Declare a statement result variable
	std::string query = "SELECT p.prod_code, p.prod_name, p.unit_price, stk.stock_qty, p.min_qty, p.vendor_price FROM product p ";
	query += "JOIN (SELECT s.stock_qty, MAX(s.stock_date), s.prod_code FROM stock_history s JOIN pokemart pk ON s.mart_id = pk.mart_id ";
	query += "GROUP BY s.prod_code) stk ON p.prod_code = stk.prod_code ORDER BY p.unit_price";//Declare query to return a list of products via most recent stock record for that store
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL); //Attempt to prepare the query. Return if unsuccessful
	if(rc != SQLITE_OK){
		std::cout << "Error selecting from product: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return -1;
	}

    // Check if there are products to update in the table
    if (sqlite3_step(res) == SQLITE_DONE) {
        std::cout << "No products to select. Product requires at least one record to add a new line to the invoice. Tell the DBA to add products." << std::endl;
        sqlite3_finalize(res);
        return -1;
    }
	sqlite3_reset(res); //Reset the result

	std::cout << "Select the product for the current line:" << std::endl; //Prompt to select a product from the menu printed below
	int count = 0, choice; //Declare a count variable to count the rows in res. choice is declared for user selection
	sqlite3_step(res); //Get the first row of the select statement
	std::cout << std::fixed << std::setprecision(2);
	do{ //Get the rest of the rows
		count++; //Increment row count
		std::cout << count << ". " << sqlite3_column_text(res, 1) << " - $" << sqlite3_column_double(res, 2) << " - " << sqlite3_column_int(res, 3) << " in stock" << std::endl; //Output trainer id and name
		rc = sqlite3_step(res); //Step to the next row
	}while(rc == SQLITE_ROW);

	std::cin >> choice; //Get the users choice of product and verify input
	while(!std::cin || choice < 1 || choice > count){
		resetStreamCheck(std::cin);
		std::cout << "Invalid selection. Please try again." << std::endl;
		std::cin >> choice;
	}
	
	//Reset the result variable and step to the specifed product row
	sqlite3_reset(res);
	for(int i = 0; i < choice; i++){
		sqlite3_step(res);
	}

	//Extract the information from the specified product
	std::string prodCode = reinterpret_cast<const char *>(sqlite3_column_text(res, 0));
	std::string prodName = reinterpret_cast<const char *>(sqlite3_column_text(res, 1));
	double price = std::stod(reinterpret_cast<const char *>(sqlite3_column_text(res, 2)));
	stockQty = std::stoi(reinterpret_cast<const char *>(sqlite3_column_text(res, 3)));
	int minQty = std::stoi(reinterpret_cast<const char *>(sqlite3_column_text(res, 4)));
	double vendorPrice = std::stod(reinterpret_cast<const char *>(sqlite3_column_text(res, 5)));
	sqlite3_finalize(res); //Finalize the result
	
	int purchaseQty; //Declare variable to hold user specified quantity to purchase
	std::cout << "Enter the amount of " << prodName << "s to be purchased:" << std::endl;
	std::cin >> purchaseQty; //Get the quantity to purchase and verify the input
	while(!std::cin || purchaseQty < 1 || purchaseQty > stockQty){
		resetStreamCheck(std::cin);
		if(purchaseQty < 1){
			std::cout << "Invalid entry. You must order at least 1 product at a time. Please try again." << std::endl;
		}
		if(purchaseQty > stockQty){
			std::cout << "Invalid entry. Cannot order more products than there are in stock (" << stockQty << " " << prodName << "s in stock). Please try again." << std::endl;
		}
		std::cin >> purchaseQty;
	}

	//Note: These variables are being passed between functions in the transaction as a reference parameter to keep track of their values between functions within the transaction
	subtotal += price * purchaseQty; //Accumulate the subtotal with the price of the product multiplied by how many products were ordered
	stockQty -= purchaseQty; //Subtract the the purchased amount from the stockQty 

	//If the stock quantity goes below the minimum quantity, we must take these extra steps
	if(stockQty < minQty){ 
		int stockReplenishAmount = minQty * 1.5; //Declare the number that we the stock to get back up to (1.5 times the minimum quantity)
		int qtyToOrder = stockReplenishAmount - stockQty; //Calculate the quantity of products that will be ordered from the vendor
		double vendorOrderPrice = qtyToOrder * vendorPrice; //Determine the total price for the order so that we can determine what value we need to add to the existing balance
		balanceBefore -= vendorOrderPrice; //Be sure to subtract the price of the vendor order from balanceBefore so we get the effect of the vendor order on the Pokemart balance 
		if(balanceBefore < 0){
			std::cout << "PokeMart " << martID << " does not have enough money in its balance to order from the vendor. Cancelling order." << std::endl;
			return -1;
		}
		std::cout << prodName << " went below it's minimum stock quantity. Making order to vendor to replenish the stock." << std::endl;
		rc = insertStockHistory(db, prodCode, martID, stockReplenishAmount); //Attempt to insert the new stock history taking into account the quantity we need to replenish stock to
		if(rc != SQLITE_OK){return -1;} //Check to see that the insert worked
	}
	
	else{ //If the stock quantity did not go below the minimum quantity, then we can just update a new record in stock_history with the stockQty we landed on
		rc = insertStockHistory(db, prodCode, martID, stockQty); //Insert a new record into stock history using the chosen product, pokemart, and current stockQty 
		if(rc != SQLITE_OK){return -1;} //Return with rollback code if unsuccessful
	}
	
	return SQLITE_OK;
}

//This function provides insert into the stock_history table to create a new record of a new quantity of stock as a result of selling a quantity of a product from a particular store
int insertStockHistory(sqlite3 *db, std::string prodCode, int martID, int newQty){
	sqlite3_stmt *res; //Declare result variable
	//Declare the current time
	char formatDate[80];
	time_t currentDate = time(NULL);
	strftime(formatDate, 80, "%F %T", localtime(&currentDate));
	std::string currentTime(formatDate);
	
	//Prepare SQL query to insert a new stock_history row
	std::string query = "INSERT INTO stock_history (prod_code, mart_id, stock_qty, stock_date) ";
	query += "VALUES (@prodCode, @martID, @newQty, @currentTime)";
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1 , &res, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error inserting stock_history: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return -1;
	}
	//Attempt to bind values to the query
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@prodCode"), prodCode.c_str(), -1, SQLITE_STATIC);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding prodCode to stock_history insert: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}
	rc = sqlite3_bind_int(res, sqlite3_bind_parameter_index(res, "@martID"), martID);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding martID to stock_history insert: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}
	rc = sqlite3_bind_int(res, sqlite3_bind_parameter_index(res, "@newQty"), newQty);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding newQty to stock_history insert: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@currentTime"), currentTime.c_str(), -1, SQLITE_STATIC);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding currentTime to stock_history insert: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}

	//Execute the query, check if it worked, then finalize res
	rc = sqlite3_step(res);
	if(rc != SQLITE_DONE){
		std::cout << "Error inserting stock_history: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return -1;
	}
	sqlite3_finalize(res);

    return SQLITE_OK;
}

//This updates the trainer_card and pokemart (mart_balance_history table) balances based on trainerID, martID and subtotal obtained from making a sale up to this point
//NOTE: My naming conventions might be a little inconsistent here. Importantly, balance is the trainer_card's attribute and represents how much money the trainer on the
//trainer card owes PokeMart. On the other hand, mart_balance_history is a table that records the amount of money that a particular PokeMart has to spend.
int updateBalances(sqlite3 *db, int trainerID, int martID, double subtotal, double balanceAfter){
	sqlite3_stmt *res; //Declare a result vairalbe
	
	//Get the current time
	char formatDate[80];
	time_t currentDate = time(NULL);
	strftime(formatDate, 80, "%F %T", localtime(&currentDate));
	std::string currentTime(formatDate);

	//Prepare SQL to update the trainer card
	std::string query = "UPDATE trainer_card SET balance = balance + " + std::to_string(subtotal) + " WHERE trainer_id = @trainerID";
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error updating trainer_card balance in updateBalances: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return -1;
	}

	//Attempt to bind trainerID to the query
	rc = sqlite3_bind_int(res, sqlite3_bind_parameter_index(res, "@trainerID"), trainerID);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding trainer ID in updateBalances: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}
	
	//Attempt to execute the query, validate that it worked, and finalize res
	rc = sqlite3_step(res);
	if(rc != SQLITE_DONE){
		sqlite3_finalize(res);
		std::cout << "Error updating trainer_card balance after bind in updateBalances: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return -1;
	}
	sqlite3_finalize(res);

	//NOTE!!! This is the second part of the function that inserts a new mart_balance_history record
	//Prepare SQL to execute insert into mart_balance_history
	query = "INSERT INTO mart_balance_history (balance, mart_id, balance_date) ";
	query += "VALUES (@balance, @martID, @currentTime)";
	rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL); //Attempt prepare
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error with insert balance_history query in updateBalances: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}

	//attempt to bind values to query
	rc = sqlite3_bind_double(res, sqlite3_bind_parameter_index(res, "@balance"), balanceAfter);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding balance to balance_history insert: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}
	rc = sqlite3_bind_int(res, sqlite3_bind_parameter_index(res, "@martID"), martID);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding mart_id to balance_history insert: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@currentTime"), currentTime.c_str(), -1, SQLITE_STATIC);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding currentTime to balance_history insert: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}

	//Execute the query, validate that it worked, then finalize res
	rc = sqlite3_step(res);
	if(rc != SQLITE_DONE){
		sqlite3_finalize(res);
		std::cout << "Error inserting new balance_history: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}
	sqlite3_finalize(res);

	return SQLITE_OK;
}

void viewInvoice(sqlite3 *db){
	sqlite3_stmt *res; //Declare res variable

	//Prepare SQL to run a select on invoice table
	std::string query = "SELECT invoice_num FROM invoice";
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
	if(rc != SQLITE_OK){ //Check to see if the prepare worked
		sqlite3_finalize(res);
		std::cout << "Error selecting invoice: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
	}

    // Check if there are trainer cards to update in the table
    if (sqlite3_step(res) == SQLITE_DONE) {
        std::cout << "No invoices to select. Invoice requires at least one record for this action. Try to insert a new record into invoice first. By making a sale." << std::endl;
        sqlite3_finalize(res);
        return;
    }
	sqlite3_reset(res); //Reset res to the start

	//Prompt for which invoice to view and print a menu
	std::cout << "Select the invoice to view: " << std::endl; //Prompt to select a trainer card from the menu printed below
	int count = 0, choice; //Declare a count variable to count the rows in trainer_card. choice is declared for user selection
	sqlite3_step(res); //Get the first row of the select statement
	do{ //Get the rest of the rows
		count++; //Increment row count
		std::cout << count << ". Invoice " << sqlite3_column_int(res, 0) << std::endl;
		rc = sqlite3_step(res); //Step to the next row
	}while(rc == SQLITE_ROW);

	std::cin >> choice; //Get the users choice of trainer card and verify input
	while(!std::cin || choice < 1 || choice > count){
		resetStreamCheck(std::cin);
		std::cout << "Invalid selection. Please try again." << std::endl;
		std::cin >> choice;
	}
	
	//Reset the result variable and step to the specifed trainer_card row
	sqlite3_reset(res);
	for(int i = 0; i < choice; i++){
		sqlite3_step(res);
	}
	std::string invoiceID = reinterpret_cast<const char *>(sqlite3_column_text(res,0)); //Extract the trainer_id from the relevant row
	sqlite3_finalize(res); //Finalize the result

	//Prepare SQL query to select invoice info
	query = "SELECT t.trainer_fname || ' ' || t.trainer_lname, e.emp_fname || ' ' || e.emp_lname, pkmt.mart_id, pkmt.street_address || ' - ' || pkmt.city || ', ' || pkmt.region, i.invoice_date ";
	query += "FROM invoice i JOIN pokemart pkmt ON i.mart_id = pkmt.mart_id JOIN employee e ON i.emp_id = e.emp_id JOIN trainer_card t ON i.trainer_id = t.trainer_id WHERE i.invoice_num = @invoiceID";
	rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error selecting invoice information: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}
	//Attempt to bind invoiceID to query
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@invoiceID"), invoiceID.c_str(), -1, SQLITE_STATIC);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding invoice id to parameter in viewInvoice: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	//Execute the query, extract the info, then finalize res
	sqlite3_step(res);
	std::string trainerName = reinterpret_cast<const char *>(sqlite3_column_text(res,0));
	std::string empName = reinterpret_cast<const char *>(sqlite3_column_text(res,1));
	std::string martID = reinterpret_cast<const char *>(sqlite3_column_text(res,2));
	std::string address = reinterpret_cast<const char *>(sqlite3_column_text(res,3));
	std::string invoiceDate = reinterpret_cast<const char *>(sqlite3_column_text(res,4));
	sqlite3_finalize(res);

	//Output the invoice info
	std::cout << std::endl;
	std::cout << "//////////////////////////////////////////////////////////" << std::endl;
	std::cout << "Invoice Info: " << std::endl;
	std::cout << "PokeMart ID: " << martID << std::endl;
	std::cout << "PokeMart Address: " << address << std::endl;
	std::cout << "Trainer Name: " << trainerName << std::endl;
	std::cout << "Clerk: " << empName << std::endl;

	//Prepare the SQL query to select the info about each line on the invoice
	query = "SELECT p.prod_name, p.prod_descript, l.qty, p.unit_price * l.qty * i.tax_rate FROM line l JOIN invoice i ON l.invoice_num = i.invoice_num ";
	query += "JOIN product p ON l.prod_code = p.prod_code WHERE i.invoice_num = @invoiceID";
	rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error selecting invoice line information: " << sqlite3_errmsg(db) << std::endl;
		return;
	}
	//Attempt to bind invoiceID to query
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@invoiceID"), invoiceID.c_str(), -1, SQLITE_STATIC);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding invoice id to parameter in viewInvoice: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	//Output details for each line
	std::cout << "Products ordered: " << std::endl;
	double runningTotal = 0; //Track the total price of all lines
	rc = sqlite3_step(res); //Step into the first row of the results of the query

	//Starting with the first row in the results, extract the invoice line info and print to the user report screen
	do{
		std::string prodName = reinterpret_cast<const char *>(sqlite3_column_text(res,0));
		std::string prodDescript = reinterpret_cast<const char *>(sqlite3_column_text(res,1));
		std::string qty = reinterpret_cast<const char *>(sqlite3_column_text(res,2));
		std::string lineTotal = reinterpret_cast<const char *>(sqlite3_column_text(res,3));
		runningTotal += std::stod(lineTotal);

		std::cout << std::fixed << std::showpoint << std::setprecision(2); //Input validation attempt
		std::cout << prodName << ":\n\t" << "Description: " << prodDescript << "\n\t" << "Line Quantity: " << qty << "\n\t" << "Line Total: $" << lineTotal << std::endl;
		rc = sqlite3_step(res); //Go to the next row
	}while(rc == SQLITE_ROW); //Quit when no more rows to read
	std::cout << "Invoice Total Charge: $" << runningTotal << std::endl; //Output total 
	std::cout << "//////////////////////////////////////////////////////////" << std::endl;
	std::cout << std::endl;
	sqlite3_finalize(res); //Finalize res
}

void viewCertificates(sqlite3 *db){
	int empID = selectPerson(db, "employee", "emp", "viewing certificate records"); //Get empID of employee to view certificate records on
	if(empID == -1){ //If there was an error selecting employee, return
		std::cout << "Error selecting an employee to view certificate records: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	//Prepare SQL to select the employee certification info
	sqlite3_stmt *res;
	std::string query = "SELECT e.emp_fname || ' ' || e.emp_lname, c.cert_descript, c.cert_payrate, cr.cert_date, c.cert_title ";
	query += "FROM employee e JOIN certification_record cr ON e.emp_id = cr.emp_id ";
	query += "JOIN certification c ON cr.cert_id = c.cert_id WHERE e.emp_id = @empID";
	int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &res, NULL);
	if(rc != SQLITE_OK){ //Check to see if prepare worked
		sqlite3_finalize(res);
		std::cout << "Error selecting employee certification information: " << sqlite3_errmsg(db) << std::endl;
		std::cout << query << std::endl;
		return;
	}
	//Attempt to bind empID to the query
	rc = sqlite3_bind_text(res, sqlite3_bind_parameter_index(res, "@empID"), (std::to_string(empID)).c_str(), -1, SQLITE_STATIC);
	if(rc != SQLITE_OK){
		sqlite3_finalize(res);
		std::cout << "Error binding employee id to parameter in viewCertificates: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	rc = sqlite3_step(res); //Step into the first row to get the employee name
	std::string empName = reinterpret_cast<const char *>(sqlite3_column_text(res,0)); //Extract employees name
	
	//Output the certification record report
	std::cout << "//////////////////////////////////////////////////////////" << std::endl;
	std::cout << empName << " Certification Record:" << std::endl;
	do{
		//For each row in the results, extract and print the data to the report
		std::string certDescript = reinterpret_cast<const char *>(sqlite3_column_text(res,1));
		std::string payrate= reinterpret_cast<const char *>(sqlite3_column_text(res,2));
		std::string certDate = reinterpret_cast<const char *>(sqlite3_column_text(res,3));
		std::string certTitle = reinterpret_cast<const char *>(sqlite3_column_text(res,4));
		std::cout << std::fixed << std::showpoint << std::setprecision(2);
		std::cout << "Certification: " << certTitle << "\n\tDescription: " << certDescript << "\n\tHourly Rate: $" << payrate << "\n\tDate Earned: " << certDate << std::endl;
		rc = sqlite3_step(res); //Go to the next row
	}while(rc == SQLITE_ROW); //We quit when there are no more rows to read
	std::cout << "//////////////////////////////////////////////////////////" << std::endl;
	std::cout << std::endl;
	sqlite3_finalize(res);	//Finalize results		
}

int startTransaction(sqlite3 *db){
	std::string query = "begin transaction";
	int rc = sqlite3_exec(db, query.c_str(), NULL, NULL, NULL);
	if (rc != SQLITE_OK) {
		std::cout << "There was an error starting transaction: " << sqlite3_errmsg(db) << std::endl;
		return rc;
	}
	return SQLITE_OK;
}

int rollback(sqlite3 *db){
	std::string query = "rollback";
	int rc = sqlite3_exec(db, query.c_str(), NULL, NULL, NULL);
	if (rc != SQLITE_OK) {
		std::cout << "There was an error rolling back the transaction: " << sqlite3_errmsg(db) << std::endl;
		return rc;
	}
	return SQLITE_OK;
}

int commit(sqlite3 *db){
	std::string query = "commit";
	int rc = sqlite3_exec(db, query.c_str(), NULL, NULL, NULL);
	if (rc != SQLITE_OK) {
		std::cout << "There was an error committing transaction: " << sqlite3_errmsg(db) << std::endl;
		rollback(db);
		return rc;
	}
	return SQLITE_OK;
}

//Checks if the input stream is in failstate. Resets input stream if it is in failstate
void resetStreamCheck(std::istream &in){
	if(!in){
		in.clear();
		in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
}
