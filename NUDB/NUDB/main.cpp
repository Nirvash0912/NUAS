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
    int nextQuarterYear;
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
vector<MYSQL_ROW> printValidCourses();
int enrollScreen();
vector<MYSQL_ROW> printEnrolledCourses();
int withdrawScreen();
int personalDetail();
int courseDetail();
void updatePasswork();
void updateAddress();
string getValidCourses();
void checkWarning(string courseId);

int main (int argc, const char* argv[]) {
    init();
    while (true) {
        cout << "->Operations:" << endl;
        cout << "    [1] Login" << endl;
        cout << "    [0] Exit" << endl << endl;
        
        int cmd = getCommand();
        if (cmd == 0) {
            cout << "Good bye!" << endl;
            exit(0);
        } else if (cmd != 0 && cmd != 1) {
            continue;
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
                            while (enrollScreen() != 0);
                            break;
                        case 3:
                            while (withdrawScreen() != 0);
                            break;
                        case 4:
                            personalDetail();
                            break;
                        case 0:
                            cout << "->User: " << user.id << " log out." << endl << endl;
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
    cout << "==================================================================================================" << endl << endl;
    getDate();
}

void printWelcome() {
    cout << "==================================================================================================" << endl;
    cout << "				Welcome to Northwestern Caesar Academic System!" << endl;
    cout << "==================================================================================================" << endl;
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
    cout << "->Please Enter a command number: ";
    int option;
    cin >> option;
    while (cin.fail()) {
        cin.clear();
        cin.ignore(INT_MAX, '\n');
        cout << "->Warning: You can only enter numbers, please retry." << endl << endl;
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
                cout << "  [1]Retry" << endl;
                cout << "  [0]EXIT" << endl << endl;
                
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
                        cout << "  [0]Exit" << endl << endl;
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
    cout << "===========================================================================================================" << endl;
    cout << "						Student Menu" << endl;
    cout << "                       Student ID: " << user.id << endl;
    cout << "===========================================================================================================" << endl;
    
    
    cout << "			Courses in " << "Academic Year: " << dt.year << ", Semester: " << dt.quarter << endl;
    cout << "  -------------------------------------------------------------------------------------------------------" << endl;
    cout << left << "     "
         << setw(8) << "CourseID" << "   "
         << setw(40) << "CourseName" << "   " << endl;
    cout << "  -------------------------------------------------------------------------------------------------------" << endl;
    
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
                cout << "     " << left
                     << setw(8) << row[0] << "   "
                     << setw(8) << row[1] << "   " << endl;
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
    cout << "    [0] Logout" << endl <<endl;
}

int transciptScreen() {
    MYSQL_RES *res_set;
    MYSQL_ROW row;
    
    char transQuery[300];
    sprintf(transQuery, "select T.UosCode, T.Semester, T.Year, T.Grade from transcript T where T.studId = %d; ", user.id);
    
    if (mysql_query(connection, transQuery) == 0) {
        cout << "	----------------------------------------------------------------------------------------" << endl;
        cout << "					Transcript of Student: " << user.id << endl;
        cout << "	----------------------------------------------------------------------------------------" << endl;
        cout << "     " << left
             << setw(8) << "CourseId" << "   "
             << setw(8) << "Semester" << "   "
             << setw(8) << "Year" << "   "
             << setw(8) << "Grade" << "   " << endl;
        cout << "	----------------------------------------------------------------------------------------" << endl;
        res_set = mysql_store_result(connection);
        int numrows = (int) mysql_num_rows(res_set);
        int numcol = (int) mysql_num_fields(res_set);
        for (int i = 0; i < numrows; i++) {
            cout << "     " << left;
            row = mysql_fetch_row(res_set);
            if (row != NULL) {
                for (int j = 0; j < numcol; j++) {
                    if (row[j] == NULL)
                        cout << setw(8) << "NULL" << "   ";
                    else
                        cout << setw(8) << row[j] << "   ";
                }
            }
            cout << endl;
        }
        cout << "	-------------------------------------------------------------------------------------------" << endl << endl;
    }
    else {
        cout << "->Error :Course list query Failed. Error Number:"<< mysql_errno(connection) << ": " << mysql_error(connection) << endl;
    }
    mysql_free_result(res_set);
    
    while (true) {
        cout << "->Operations:" << endl;
        cout << "    [1] Course Detail" << endl;
        cout << "    [0] Return to Student Menu" << endl << endl;

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
        
        sprintf(courseQuery, "select T.UoSCode as CourseID,US.UoSName as CourseName,T.Semester as Semester,T.Year as Year,UO.Enrollment as StudentEnrollment, UO.MaxEnrollment as MaxStudentEnrollment, F.Name as InstructerName, T.Grade as Grade from transcript T, unitofstudy US, uosoffering UO, faculty F where T.UoSCode = US.UoSCode and UO.UoSCode = T.UoSCode and F.Id = UO.InstructorId and T.Year = UO.Year and T.Semester = UO.Semester and StudId = %d and T.UoSCode = \'%s\'; ", user.id, courseId.c_str());
        
        if (mysql_query(connection, courseQuery) == 0) {
            res_set = mysql_store_result(connection);
            int numrows = (int) mysql_num_rows(res_set);
            
            if (numrows < 1) {
                cout << "->Can't find course with ID: " << courseId << endl;
                cout << "    [1] Retry" << endl;
                cout << "    [0] Exit" << endl << endl;
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

vector<MYSQL_ROW> printValidCourses() {
    MYSQL_RES *res_set;
    MYSQL_ROW row;
    
    vector<MYSQL_ROW> validCourses;
    
    char courseQuery[500];
    sprintf(courseQuery, "select UoSCode,UoSName,Year,Semester from uosoffering natural join unitofstudy natural join lecture natural join classroom where(Year = %d and Semester = \"%s\") or (Year = %d and Semester = \"%s\"); ",dt.year, dt.quarter.c_str(), dt.nextQuarterYear, dt.nextQuarter.c_str());
    
    // for test
    //sprintf(courseQuery, "select UoSCode,UoSName,Year,Semester from uosoffering natural join unitofstudy natural join lecture natural join classroom where(Year = %d and Semester = \"%s\") or (Year = %d and Semester = \"%s\"); ",2015, "Q1", 2016, "Q2");
    
    if (mysql_query(connection, courseQuery) == 0) {
        cout << "			Valid Courses For Student " << user.id << " in " << dt.year << " " << dt.quarter << " or in " << dt.nextQuarterYear << " " << dt.nextQuarter << endl;
        cout << "	-------------------------------------------------------------------------------------------" << endl;
        cout << "     " << left
             << setw(8) << "Option"
             << setw(8)	<<"CourseID" << "   "
             << setw(40) << "CourseName" << "   "
             << setw(8) << "Year" << "   "
             << setw(8) << "Semester" << endl;
        cout << "	-------------------------------------------------------------------------------------------" << endl;
        res_set = mysql_store_result(connection);
        int numrows = (int) mysql_num_rows(res_set);
        int numcol = (int) mysql_num_fields(res_set);
        for (int i = 0; i < numrows; i++) {
            cout << "     ";
            row = mysql_fetch_row(res_set);
            validCourses.push_back(row);
            cout << left << setw(8) << "[" + to_string(i + 1) +"]";
            if (row != NULL) {
                for (int j = 0; j < numcol; j++) {
                    if (j == 1) {
                        if (row[j] == NULL)
                            cout << setw(40) << "NULL" << "   ";
                        else
                            cout << setw(40) << row[j] << "   ";
                    } else {
                        if (row[j] == NULL)
                            cout << setw(8) << "NULL" << "   ";
                        else
                            cout << setw(8) << row[j] << "   ";
                    }
                }
            }
            cout << endl;
        }
        cout << "	-------------------------------------------------------------------------------------------" << endl << endl;
        mysql_free_result(res_set);
    }
    else {
        cout << "->Error :Course list query Failed. Error Number:"<< mysql_errno(connection) << ": " << mysql_error(connection) << endl;
    }
    
    return validCourses;
}

int enrollScreen() {
    MYSQL_RES *res_set = NULL;
    MYSQL_ROW row;
    
    vector<MYSQL_ROW> validCourses = printValidCourses();
    
    if (validCourses.size() == 0) {
        cout << "->No valid course found!" << endl;
        cout << "->Operations:" << endl;
        cout << "    [0] Return to Student Menu" << endl << endl;
        
        while (true) {
            int cmd = getCommand();
            if (cmd == 0) {
                return 0;
            }
        }
    }
    
    int selection;
    while (true) {
        cout << "->Please enter the option number to enroll (Enter 0 to Exit): ";
        cin >> selection;
        if (selection == 0) {
            return 0;
        } else if (selection > validCourses.size()){
            cout << "->Invalid option number, please retry!" << endl << endl;
            continue;
        } else {
            selection --;
            break;
        }
    }
    
    string courseId = validCourses[selection][0];
    string courseYear = validCourses[selection][2];
    string courseSemester = validCourses[selection][3];
    
    char query_call_enroll[150];
    
    sprintf(query_call_enroll, "call enroll_course(\"%s\", %d, \"%s\", \"%s\");", courseId.c_str(), user.id, courseYear.c_str(), courseSemester.c_str());
    // );
    
    // message: 1:max enrollment; 2: pre-requisite; 3:course exists; 4: success.
    if (mysql_query(connection, query_call_enroll) == 0) {
        do {
            if (mysql_field_count(connection) > 0) {
                res_set = mysql_store_result(connection);
                mysql_free_result(res_set);
            }
        } while (mysql_next_result(connection) == 0);
    
        row = mysql_fetch_row(res_set);
        
        int message;
        if (row[0] != NULL) {
            message = atoi(row[0]);
            if (message == 1) {
                cout << "->Warning: The coure is filled up." << endl;
            }
            else if (message == 2) {
                cout << "->Warning: The pre-requisite is not satisfied." << endl;
                cout << "->Course(s) should be taken:" << endl;
                
                char courseQuery[300];
                sprintf(courseQuery, "SELECT uoscode, uosname FROM unitofstudy WHERE uoscode IN (SELECT r.prereqUosCode FROM requires AS r WHERE r.uoscode = \"%s\");", courseId.c_str());
                if (mysql_query(connection, courseQuery) == 0) {
                    res_set = mysql_store_result(connection);
                    int numrows = (int) mysql_num_rows(res_set);
                    for (int i = 0; i < numrows; i++) {
                        if (row != NULL) {
                            row = mysql_fetch_row(res_set);
                            cout << "   " << row[0] << ", " << row[1] << endl;
                        }
                    }
                    cout << endl;
                    mysql_free_result(res_set);
                } else {
                    cout << "->Error :Course list query Failed. Error Number:"<< mysql_errno(connection) << ": " << mysql_error(connection) << endl;
                }
                
            }
            else if (message == 3) {
                cout << "->Warning: The course has already been taken." << endl;
            }
            else if (message == 4) {
                cout << "->Enrollment succeed." << endl;
                
                checkWarning(courseId);
            }
        }
    } else {
        cout << "->Error :Enroll Procedure Failed. Error Number:"<< mysql_errno(connection) << ": " << mysql_error(connection) << endl;
    }
    
    cout << "->Operations:" << endl;
    cout << "    [1] Continue to enroll others" << endl;
    cout << "    [0] Return to Student Menu" << endl << endl;

    while (true) {
        int cmd = getCommand();
        if (cmd == 0) {
            break;
        } else if (cmd == 1) {
            return 1;
        }
    }

    return 0;
}

vector<MYSQL_ROW> printEnrolledCourses() {
    MYSQL_RES *res_set;
    MYSQL_ROW row;
    
    vector<MYSQL_ROW> enrolledCourses;
    
    char courseQuery[300];
    sprintf(courseQuery, "select UoSCode, UoSName, Semester, Year from transcript natural join unitofstudy where StudId = %d; ", user.id);
    if (mysql_query(connection, courseQuery) == 0) {
        cout << "					Enrolled Course List of Student " << user.id << endl;
        cout << "	-------------------------------------------------------------------------------------------" << endl;
        cout << "     " << left
        << setw(8) << "Option" << "   "
        << setw(8)	<<"CourseID" << "   "
        << setw(40) << "CourseName" << "   "
        << setw(8) << "Year" << "   "
        << setw(8) << "Semester" << endl;
        cout << "	-------------------------------------------------------------------------------------------" << endl;
        res_set = mysql_store_result(connection);
        int numrows = (int) mysql_num_rows(res_set);
        int numcol = (int) mysql_num_fields(res_set);
        
        for (int i = 0; i < numrows; i ++) {
            cout << "     ";
            row = mysql_fetch_row(res_set);
            enrolledCourses.push_back(row);
            cout << left << setw(8) << "[" + to_string(i + 1) +"]";
            for (int j = 0; j < numcol; j ++) {
                if (j == 1) {
                    if (row[j] == NULL)
                        cout << setw(40) << "NULL" << "   ";
                    else
                        cout << setw(40) << row[j] << "   ";
                }
                else {
                    if (row[j] == NULL)
                        cout << setw(8) << "NULL" << "   ";
                    else
                        cout << setw(8) << row[j] << "   ";
                }
            }
            cout << endl;
        }
        cout << "	-------------------------------------------------------------------------------------------" << endl;
        mysql_free_result(res_set);
    } else {
        cout << "->Error :Enrolled Course list query Failed. Error Number:"<< mysql_errno(connection) << ": " << mysql_error(connection) << endl;
    }
    
    return enrolledCourses;
}

int withdrawScreen() {
    MYSQL_RES *res_set = NULL;
    MYSQL_ROW row;
    
    vector<MYSQL_ROW> enrolledCourses = printEnrolledCourses();
    
    if (enrolledCourses.size() == 0) {
        cout << "->No enrolled course found!" << endl;
        cout << "->Operations:" << endl;
        cout << "    [0] Return to Student Menu" << endl << endl;
        
        while (true) {
            int cmd = getCommand();
            if (cmd == 0) {
                return 0;
            }
        }
    }
    
    int selection;
    while (true) {
        cout << "->Please enter the option number to withdraw (Enter 0 to Exit): ";
        cin >> selection;
        
        if (selection == 0) {
            return 0;
        } else if (selection > enrolledCourses.size()){
            cout << "->Invalid option number, please retry!" << endl << endl;
            continue;
        } else {
            selection --;
            break;
        }
    }
    
    string courseId = enrolledCourses[selection][0];
    string courseYear = enrolledCourses[selection][2];
    string courseSemester = enrolledCourses[selection][3];
    
    char withdrawQuery[150];
    sprintf(withdrawQuery, "call withdraw_course(\"%s\", %d);", courseId.c_str(), user.id);
    
    if (mysql_query(connection, withdrawQuery) == 0) {
        do {
            if (mysql_field_count(connection) > 0) {
                res_set = mysql_store_result(connection);
                mysql_free_result(res_set);
            }
        } while (mysql_next_result(connection) == 0);
    
        row = mysql_fetch_row(res_set);
        
        // message: 1:coure not enrolled; 2: course finied; 3:success.
        int message;
    
        if (row[0] == NULL) {
            cout << "The withdraw procedure is wrong." << endl;
        }
        else {
            message = atoi(row[0]);
            if (message == 1) {
                cout << "->Warning: The course is not enrolled." << endl;
            }
            else if (message == 2) {
                cout << "->Warning: Completed course can't be withdrawn." << endl;
            }
            else if (message == 3) {
                cout << "->Withdraw succeed." << endl;
            
                checkWarning(courseId);
            }
        }
    } else {
        cout << "->Error :Withdraw Procedure Failed. Error Number:"<< mysql_errno(connection) << ": " << mysql_error(connection) << endl;
        return -1;
    }
    
    cout << "->Operations:" << endl;
    cout << "    [1] Continue to withdraw others" << endl;
    cout << "    [0] Return to Student Menu" << endl << endl;
    
    while (true) {
        int cmd = getCommand();
        if (cmd == 0) {
            break;
        } else if (cmd == 1) {
            return 1;
        }
    }
    
    return 0;
}

void checkWarning(string courseId) {
    MYSQL_RES *res_set;
    MYSQL_ROW row;
    
    if (mysql_query(connection, "select * from warning_log") == 0) {
        res_set = mysql_store_result(connection);
        int numrows = (int) mysql_num_rows(res_set);
        row = mysql_fetch_row(res_set);
        
        if (numrows != 0) {
            cout << "-->Warning: Student number enrolled in course: [" << courseId << "] " << " is lower than 50% of Max Enrollment!" << endl;
        }
        mysql_free_result(res_set);
    } else {
        cout << "->Error :Warning query Failed. Error Number:"<< mysql_errno(connection) << ": " << mysql_error(connection) << endl;
    }
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
        cout << "    [0] Exit" << endl << endl;
        
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
            cout << "->Warning: Two inputs are not same please try again." << endl << endl;
            continue;
        } else if (newPass.length() > 10 || newPass.length() == 0) {
            cout << "->Warning: The length of address should be 1~10 characters." << endl << endl;
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
    while (true) {
        cout << "->Please enter new address: ";
        cin >> newAddr;
        
        if (newAddr.length() > 50 || newAddr.length() == 0) {
            cout << "->Warning: The length of address should be 1~50 characters." << endl << endl;
        } else {
            break;
        }
    }
    
    char updateQuery[200];
    sprintf(updateQuery, "update student set Address = \"%s\" where Id = %d; ", newAddr.c_str(), user.id);
    if (mysql_query(connection, updateQuery) != 0) {
        cout << "->Error: Update failed. Error Number:" << mysql_errno(connection) << ": " << mysql_error(connection) << endl;
    }
}
