#include <iostream>
#include <mysql.h>
using namespace std;
int main() {
    MYSQL* conn = mysql_init(NULL);
    conn = mysql_real_connect(
        conn,
        "localhost",
        "root",
        "ashu@2025",
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
    Student s;
    s.addStudent(conn);
    mysql_close(conn);
    return 0;
}