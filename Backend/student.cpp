#include <iostream>
#include <mysql.h>

using namespace std;

class Student
{
public:
    int student_id;
    string name;
    string email;
    string department;
    string phone;
    string password;

    Student() : student_id(0), password("student123") {}

    Student(int id, string n, string e, string d, string p, string pwd = "student123")
        : student_id(id), name(n), email(e), department(d), phone(p), password(pwd) {}

    // Method to create and insert a new student record into MySQL
    static bool createStudent(MYSQL* conn, int id, const string& name, const string& email,
                              const string& department, const string& phone,
                              const string& password, string& errMsg)
    {
        if (conn == NULL) {
            errMsg = "Database connection not initialized";
            return false;
        }

        char escName[205];
        char escEmail[205];
        char escDept[105];
        char escPhone[35];
        char escPwd[515];

        mysql_real_escape_string(conn, escName, name.c_str(), min((int)name.length(), 100));
        mysql_real_escape_string(conn, escEmail, email.c_str(), min((int)email.length(), 100));
        mysql_real_escape_string(conn, escDept, department.c_str(), min((int)department.length(), 50));
        mysql_real_escape_string(conn, escPhone, phone.c_str(), min((int)phone.length(), 15));
        mysql_real_escape_string(conn, escPwd, password.c_str(), min((int)password.length(), 255));

        string query = "INSERT INTO students (student_id, name, email, department, phone, password) VALUES (" +
                       to_string(id) + ", '" +
                       string(escName) + "', '" +
                       string(escEmail) + "', '" +
                       string(escDept) + "', '" +
                       string(escPhone) + "', '" +
                       string(escPwd) + "')";

        if (mysql_query(conn, query.c_str()) != 0) {
            errMsg = mysql_error(conn);
            return false;
        }
        return true;
    }

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
        string query = "INSERT INTO students (student_id, name, email, department, phone, password) VALUES (" +
                       to_string(id) + ", '" +
                       name + "', '" +
                       email + "', '" +
                       department + "', '" +
                       phone + "', 'student123')";
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