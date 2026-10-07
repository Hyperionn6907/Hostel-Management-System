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
};