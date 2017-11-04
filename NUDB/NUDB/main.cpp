//
//  main.cpp
//  NUDB
//
//  Created by Shixin Luo on 11/3/17.
//  Copyright © 2017 Shixin Luo. All rights reserved.
//

#include <mysql.h>
#include <iostream>
using namespace std;

int main (int argc, const char* argv[]) {
    MYSQL *connection, mysql;
    mysql_init(&mysql);
    connection = mysql_real_connect(&mysql, "localhost", "root", "3604818", "project3-nudb", 0, 0, 0);
    if (connection == NULL) {
        //unable to connect
        printf("Oh Noes!\n");
    } else {
        printf("You are now connected. Welcome!\n");
    }
    printf("Hi");
}
