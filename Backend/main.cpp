#include <iostream>
#include <mysql/mysql.h>
using namespace std;

int main() {

    MYSQL* conn = mysql_init(NULL);

    conn = mysql_real_connect(
        conn,
        "localhost",
        "root",
        "your_password",
        "hostel_management",
        3306,
        NULL,
        0
    );

    if (conn == NULL) {
        cout << "Database connection failed\n";
        return 1;
    }

    cout << "Database connected successfully!\n";

    mysql_close(conn);

    return 0;
}