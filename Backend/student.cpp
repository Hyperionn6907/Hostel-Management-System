#include <iostream>
#include <mysql.h>

using namespace std;

class Student
{
public:
    void addStudent(MYSQL* conn)
    {
        int id;
        string name, email, department, phone;
        cout << "Enter student ID: ";
        cin >> id;
        cin.ignore();
        cout << "Enter name: ";
        getline(cin, name);
        cout << "Enter email: ";
        getline(cin, email);
        cout << "Enter department: ";
        getline(cin, department);
        cout << "Enter phone: ";
        getline(cin, phone);
        string query = "INSERT INTO students VALUES (" +
                       to_string(id) + ", '" +
                       name + "', '" +
                       email + "', '" +
                       department + "', '" +
                       phone + "')";
        if (mysql_query(conn, query.c_str()) == 0)
            cout << "Student added successfully\n";
        else
            cout << "Failed to add student\n";
    }
    void viewStudent(MYSQL* conn){
    string query = "SELECT * FROM students";
    if (mysql_query(conn, query.c_str()) == 0)
    {
        MYSQL_RES* result = mysql_store_result(conn);
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result)) != NULL)
        {
            cout << row[0] << " "
                 << row[1] << " "
                 << row[2] << " "
                 << row[3] << " "
                 << row[4] << endl;
        }
        mysql_free_result(result);
    }
    else
    {
        cout << "Failed to view students\n";
    }
}
void updateStudent(MYSQL* conn)
{
    int id;
    string name, email, department, phone;

    cout << "Enter student ID to update: ";
    cin >> id;

    cin.ignore();

    cout << "Enter new name: ";
    getline(cin, name);

    cout << "Enter new email: ";
    getline(cin, email);

    cout << "Enter new department: ";
    getline(cin, department);

    cout << "Enter new phone: ";
    getline(cin, phone);

    string query = "UPDATE students SET name='" + name +
                   "', email='" + email +
                   "', department='" + department +
                   "', phone='" + phone +
                   "' WHERE student_id=" + to_string(id);

    if (mysql_query(conn, query.c_str()) == 0)
        cout << "Student updated successfully\n";
    else
        cout << "Failed to update student\n";
}
void deleteStudent(MYSQL* conn)
{
    int id;

    cout << "Enter student ID to delete: ";
    cin >> id;

    string query = "DELETE FROM students WHERE student_id=" + to_string(id);

    if (mysql_query(conn, query.c_str()) == 0)
        cout << "Student deleted successfully\n";
    else
        cout << "Failed to delete student: " << mysql_error(conn) << endl;
}
};