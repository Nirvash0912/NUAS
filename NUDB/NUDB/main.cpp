//
//  main.cpp
//  NUDB
//
//  Created by Shixin Luo on 11/3/17.
//  Copyright © 2017 Shixin Luo. All rights reserved.
//

#include <mysql.h>
#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <stdio.h>
#include <limits>
#include <vector>

using namespace std;

// global variables
MYSQL *connection, mysql;

struct dataTime {
    int month;
    int year;
    string quarter;
    string nextQuarter;
    string nextQuarterYear;
} dt;

struct studentInfo {
    int id;
    string password;
} user;

void printWelcome();
void init();
void getDate();
int getCommand();
int login();
void printMenu();
int transciptScreen();
int enrollScreen();
int withdrawScreen();
int personalDetail();
int courseDetail();
void updatePasswork();
void updateAddress();

int main (int argc, const char* argv[]) {
    init();
    while (true) {
        cout << "->Operations:" << endl;
        cout << "    [1] Login" << endl;
        cout << "    [0] Exit" << endl;
        
        int cmd = getCommand();
        if (cmd == 0) {
            cout << "Good bye!" << endl;
            exit(0);
        }
        
        switch (login()) {
            case -1: {
                cout << "->Error: Query failed. Error Number:" << mysql_errno(connection) << ": " << mysql_error(connection) << endl;
                continue;
            }
            case 0: {
                continue;
            }
            case 1: {
                while (true) {
                    printMenu();
                    bool isLogout = false;
                    cmd = getCommand();
                    // [1]Transcript    [2]Enroll    [3]Withdraw    [4]Personal Details    [0]Logout
                    switch (cmd) {
                        case 1:
                            transciptScreen();
                            break;
                        case 2:
                            enrollScreen();
                            break;
                        case 3:
                            withdrawScreen();
                            break;
                        case 4:
                            personalDetail();
                            break;
                        case 0:
                            cout << "->" << user.id << " log out." << endl << endl;
                            isLogout = true;
                            break;
                    }
                    if (isLogout) break;
                }
            }
        }
    }
}

void init() {
    // print welcome info
    printWelcome();
    mysql_init(&mysql);
    connection = mysql_real_connect(&mysql, "localhost", "root", "123456", "project3-nudb", 3306, NULL, CLIENT_MULTI_RESULTS);
    if (connection == NULL) {
        // unable to connect
        cout << "Fatal Error: Failed to conncet to DB. Error Number:" << mysql_errno(connection) << ": " << mysql_error(connection) << endl;
        exit(1);
    }
    cout << "Connection Built." << endl;
    cout << "=====================================================================================================" << endl << endl;
    getDate();
}

void printWelcome() {
    cout << "=====================================================================================================" << endl;
    cout << "				Welcome to Northwestern Caesar Academic System!" << endl;
    cout << "=====================================================================================================" << endl;
    cout << "System initializing." << endl;
    cout << "Connecting to database." << endl;
}

void getDate() {
    time_t currentTime;
    struct tm *localTime;
    time(&currentTime);
    localTime = localtime(&currentTime);
    
    int month = localTime->tm_mon + 1;
    int year = localTime->tm_year + 1900;
    dt.month = month;
    dt.year = year;
    
    if (month >= 9 && month <= 12) {
        dt.quarter = "Q1";
        dt.nextQuarter = "Q2";
        dt.nextQuarterYear = year + 1;
    }
    else if (month >= 1 && month <= 3) {
        dt.quarter = "Q2";
        dt.nextQuarter = "Q3";
        dt.nextQuarterYear = year;
    }
    else if (month >= 4 && month <= 6) {
        dt.quarter = "Q3";
        dt.nextQuarter = "Q4";
        dt.nextQuarterYear = year;
    }
    else {
        dt.quarter = "Q4";
        dt.nextQuarter = "Q1";
        dt.nextQuarterYear = year;
    }
}

int getCommand() {
    cout << endl;
    cout << "->Please Enter a command number: ";
    int option;
    cin >> option;
    while (cin.fail()) {
        cin.clear();
        cin.ignore(INT_MAX, '\n');
        cout << "->Warning: You can only enter numbers, please retry.\n";
        cout << "->Please Enter a command number: ";
        cin >> option;
    }
    cout << endl;
    return option;
}

int login() {
    MYSQL_RES *res_set;
    MYSQL_ROW row;
    
    char idQuery[100];
    
    while (true) {
        cout << "->Please Enter your user ID: ";
        cin >> user.id;
        sprintf(idQuery, "SELECT * FROM student WHERE Id = %d; ", user.id);
        
        if (mysql_query(connection, idQuery) == 0) {
            res_set = mysql_store_result(connection);
            int numrows = (int) mysql_num_rows(res_set);
            if (numrows < 1) {
                cout << "->Warning: Invalid ID!" << endl;
                cout << "  [1]Retry		[0]EXIT" << endl;
                
                int cmd = getCommand();
                if (cmd == 1) {
                    continue;
                } else {
                    return 0;
                }
            } else {
                row = mysql_fetch_row(res_set);
                while (true) {
                    cout << "->Please enter your password: ";
                    cin >> user.password;
                    
                    if (user.password.compare(row[2]) != 0) {
                        cout << "->Warning: Wrong password!" << endl;
                        cout << "  [1]Retry" << endl;
                        cout << "  [0]Exit" << endl;
                        int cmd = getCommand();
                        if (cmd == 1) {
                            continue;
                        } else {
                            return 0;
                        }
                    } else {
                        mysql_free_result(res_set);
                        return 1;
                    }
                }
            }
        } else {
            return -1;
        }
    }
}

void printMenu() {
    cout << endl;
    cout << "==============================================================================================================" << endl;
    cout << "						Student Menu" << endl;
    cout << "                       Student ID: " << user.id << endl;
    cout << "==============================================================================================================" << endl;
    
    
    cout << "			Courses in " << "Academic Year: " << dt.year << ", Semester: " << dt.quarter << endl;
    cout << "  ----------------------------------------------------------------------------------------------------------" << endl;
    cout << "   			Course ID			 			Course Name" << endl;
    cout << "  ----------------------------------------------------------------------------------------------------------" << endl;
    
    MYSQL_RES *res_set;
    MYSQL_ROW row;
    
    char courseQuery[300];
    sprintf(courseQuery,
            "select u.UoSCode, u.UoSName from transcript T, unitofstudy u where T.UoSCode = u.UoSCode and T.studId = %d and T.Semester = \'%s\' and T.Year = %d and T.grade is null", user.id, dt.quarter.c_str(), dt.year);
    
    if (mysql_query(connection, courseQuery) == 0) {
        res_set = mysql_store_result(connection);
        int numrows = (int) mysql_num_rows(res_set);
        
        for (int i = 0; i < numrows; i++) {
            row = mysql_fetch_row(res_set);
            if (row != NULL) {
                cout << "			"<< row[0] << "			   " << row[1]<< endl;
            }
        }
    }
    else {
        cout << "->Error :Course list query Failed. Error Number:"<< mysql_errno(connection) << ": " << mysql_error(connection) << endl;
    }
    cout << "  ----------------------------------------------------------------------------------------------------------" << endl;
    mysql_free_result(res_set);
    cout << "->Operations:" << endl;
    cout << "    [1] Transcript" << endl;
    cout << "    [2] Enroll" << endl;
    cout << "    [3] Withdraw" << endl;
    cout << "    [4] Personal Details" << endl;
    cout << "    [0] Logout" << endl;
}

int transciptScreen() {
    MYSQL_RES *res_set;
    MYSQL_ROW row;
    
    char transQuery[300];
    sprintf(transQuery, "select T.UosCode, T.Semester, T.Year, T.Grade from transcript T where T.studId = %d; ", user.id);
    
    if (mysql_query(connection, transQuery) == 0) {
        cout << "	----------------------------------------------------------------------------------------------" << endl;
        cout << "					Transcript of Student: " << user.id << endl;
        cout << "	----------------------------------------------------------------------------------------------" << endl;
        cout << "	 Course Id			 Semester	 Year			 Grade" << endl;
        cout << "	----------------------------------------------------------------------------------------------" << endl;
        res_set = mysql_store_result(connection);
        int numrows = (int) mysql_num_rows(res_set);
        int numcol = (int) mysql_num_fields(res_set);
        for (int i = 0; i < numrows; i++) {
            cout << "		";
            row = mysql_fetch_row(res_set);
            if (row != NULL) {
                for (int j = 0; j < numcol; j++) {
                    if (row[j] == NULL) {
                        cout << "NULL" << "       		";
                    }
                    else
                        cout << row[j] << "       		";
                }
            }
            cout << endl;
        }
        cout << "	----------------------------------------------------------------------------------------------" << endl << endl;
    }
    else {
        cout << "->Error :Course list query Failed. Error Number:"<< mysql_errno(connection) << ": " << mysql_error(connection) << endl;
    }
    mysql_free_result(res_set);
    
    while (true) {
        cout << "->Operations:" << endl;
        cout << "    [1] Course Detail" << endl;
        cout << "    [0] Return to Student Menu" << endl;

        int cmd = getCommand();
        if (cmd == 1) {
            courseDetail();
        } else if (cmd == 0) {
            break;
        }
    }
    return 0;
}

int courseDetail() {
    MYSQL_RES *res_set;
    MYSQL_ROW row;
    
    string courseId;
    
    char courseQuery[500];
    
    while (true) {
        cout << "->Please enter a course ID: ";
        cin >> courseId;
        
        sprintf(courseQuery, "select T.UoSCode as CourseID,US.UoSName as CourseName,T.Semester as Semester,T.Year as Year,UO.Enrollment as StudentEnrollment,UO.MaxEnrollment as MaxStudentEnrollment, F.Name as InstructerName, T.Grade as Grade from transcript T, unitofstudy US, uosoffering UO, faculty F where T.UoSCode = US.UoSCode and UO.UoSCode = T.UoSCode and F.Id = UO.InstructorId and T.Year = UO.Year and T.Semester = UO.Semester and StudId = %d and T.UoSCode = \'%s\'; ", user.id, courseId.c_str());
        
        if (mysql_query(connection, courseQuery) == 0) {
            res_set = mysql_store_result(connection);
            int numrows = (int) mysql_num_rows(res_set);
            
            if (numrows < 1) {
                cout << "->Can't find course with ID does not exist.\n";
                cout << "    [1] Retry" << endl;
                cout << "    [0] Exit" << endl;
                mysql_free_result(res_set);
                
                int cmd = getCommand();
                
                if (cmd == 1) {
                    continue;
                } else if (cmd == 0) {
                    return 0;
                }
            } else {
                for (int i = 0; i < numrows; i++) {
                    row = mysql_fetch_row(res_set);
                    if (row != NULL) {
                        cout << "			------------------------------------------------------" << endl;
                        cout << "				   COURSE DETAIL OF COURSE " << courseId << endl;
                        cout << "			------------------------------------------------------" << endl;
                        
                        
                        cout << "				Course Id: " << row[0] << endl;
                        cout << "				Course Name: " << row[1] << endl;
                        cout << "				Semester: " << row[2] << endl;
                        cout << "				Year: " << row[3] << endl;
                        cout << "				Student Enrolled in this class: " << row[4] << endl;
                        cout << "				Maximum Enrollment: " <<row[5] << endl;
                        cout << "				Instructer Name: " << row[6] << endl;
                        if (row[7] == NULL) {
                            cout << "				Grade: " << "NULL" << endl;
                        }
                        else {
                            cout << "				Grade: " << row[7] << endl;
                        }
                        cout << endl;
                    }
                }
                break;
            }
        }
    }
    
    mysql_free_result(res_set);
    
    cout << endl;
    return 0;
}

int enrollScreen() {
    return 0;
}

int withdrawScreen() {
    return 0;
}

int personalDetail() {
    MYSQL_RES *res_set;
    MYSQL_ROW row;
    
    char personQuery[200];
    sprintf(personQuery, "select Id, Name, Password, Address from student where Id = %d; ", user.id);
    if (mysql_query(connection, personQuery) == 0) {
        res_set = mysql_store_result(connection);
        int numrows = (int)mysql_num_rows(res_set);
        
        for (int i = 0; i < numrows; i++) {
            cout << "			------------------------------------------------------" << endl;
            cout << "					PERSONAL DETAIL PAGE OF " << user.id << endl;
            cout << "			------------------------------------------------------" << endl;
            row = mysql_fetch_row(res_set);
            cout << "				Id: " << row[0] << endl;
            cout << "				Name: " << row[1] << endl;
            cout << "				Password: " << row[2] << endl;
            cout << "				Address: " << row[3] << endl;
        }
    }
    mysql_free_result(res_set);
    cout << endl;
    
    while (true) {
        bool isExit = false;
        cout << "->Operations:" << endl;
        cout << "    [1] Change password" << endl;
        cout << "    [2] Change Address" << endl;
        cout << "    [0] Exit" << endl;
        
        int cmd = getCommand();
        switch (cmd) {
            case 1:
                updatePasswork();
                break;
            case 2:
                updateAddress();
                break;
            case 0:
                isExit = true;
                break;
        }
        if (isExit)
            break;
    }
    
    return 0;
}

void updatePasswork () {
    string  newPass, reEnterPass;
    while (true) {
        cout << "->Please enter new passward: ";
        cin >> newPass;
        cout << "->Please re-enter your new password: ";
        cin >> reEnterPass;
        
        if (newPass.compare(reEnterPass) != 0) {
            cout << "Two inputs are not same please try again." << endl << endl;
            continue;
        } else {
            break;
        }
    }
    
    char updateQuery[200];
    sprintf(updateQuery, "update student set Password = \"%s\" where Id = %d; ", newPass.c_str(), user.id);
    if (mysql_query(connection, updateQuery) != 0) {
        cout << "->Error: Query failed. Error Number:" << mysql_errno(connection) << ": " << mysql_error(connection) << endl;
    }
}

void updateAddress () {
    string newAddr;
    cout << "->Please enter new address: ";
    cin >> newAddr;
    
    char updateQuery[200];
    sprintf(updateQuery, "update student set Address = \"%s\" where Id = %d; ", newAddr.c_str(), user.id);
    if (mysql_query(connection, updateQuery) != 0) {
        cout << "->Error: Update failed. Error Number:" << mysql_errno(connection) << ": " << mysql_error(connection) << endl;
    }
}
