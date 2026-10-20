#include <iostream>
#include <mysql.h>
#include "httplib.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

#include "student.cpp"

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

    httplib::Server server;

    server.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"}
    });

    server.Options(R"(.*)", [](const httplib::Request& req,
                               httplib::Response& res) {
        res.status = 200;
    });

    server.Post("/api/login",
        [&](const httplib::Request& req, httplib::Response& res) {

        try {
            json data = json::parse(req.body);

            string userId = data.at("userId").get<string>();
            string password = data.at("password").get<string>();

            cout << "Login request received for: "
                 << userId << endl;

            // Escape user input before putting it into SQL.
            char* escapedPassword = new char[password.length() * 2 + 1];

            mysql_real_escape_string(
                conn,
                escapedPassword,
                password.c_str(),
                password.length()
            );

            string query =
                "SELECT student_id FROM students WHERE student_id = "
                + userId
                + " AND password = '"
                + escapedPassword
                + "'";

            delete[] escapedPassword;

            if (mysql_query(conn, query.c_str()) != 0) {
                cout << "MySQL query error: "
                     << mysql_error(conn) << endl;

                res.status = 500;
                res.set_content(
                    R"({"success":false,"message":"Database query failed"})",
                    "application/json"
                );
                return;
            }

            MYSQL_RES* result = mysql_store_result(conn);

            if (result == NULL) {
                cout << "Result error: "
                     << mysql_error(conn) << endl;

                res.status = 500;
                res.set_content(
                    R"({"success":false,"message":"Could not read database result"})",
                    "application/json"
                );
                return;
            }

            my_ulonglong rows = mysql_num_rows(result);

            cout << "Matching rows: " << rows << endl;

            if (rows > 0) {
                res.set_content(
                    R"({"success":true,"role":"student"})",
                    "application/json"
                );
            } else {
                res.set_content(
                    R"({"success":false})",
                    "application/json"
                );
            }

            mysql_free_result(result);

        } catch (const exception& e) {
            cout << "Login error: " << e.what() << endl;

            res.status = 400;
            res.set_content(
                R"({"success":false,"message":"Invalid login request"})",
                "application/json"
            );
        }
    });

        server.Get(R"(/api/students/(\d+))",
        [&](const httplib::Request& req, httplib::Response& res) {

        string id = req.matches[1];

        string query =
            "SELECT student_id, name, email, department, phone "
            "FROM students WHERE student_id = " + id;

        if (mysql_query(conn, query.c_str()) != 0) {
            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Database query failed"})",
                "application/json"
            );
            return;
        }

        MYSQL_RES* result = mysql_store_result(conn);

        if (result == NULL) {
            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Could not read database result"})",
                "application/json"
            );
            return;
        }

        MYSQL_ROW row = mysql_fetch_row(result);

        if (row != NULL) {
            json student;

            student["success"] = true;
            student["student_id"] = row[0] ? row[0] : "";
            student["name"] = row[1] ? row[1] : "";
            student["email"] = row[2] ? row[2] : "";
            student["department"] = row[3] ? row[3] : "";
            student["phone"] = row[4] ? row[4] : "";

            res.set_content(student.dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(
                R"({"success":false,"message":"Student not found"})",
                "application/json"
            );
        }

        mysql_free_result(result);
    });

    
    server.Post("/api/complaints",
        [&](const httplib::Request& req, httplib::Response& res) {

        try {
            json data = json::parse(req.body);

            int studentId = data.at("student_id").get<int>();
            string description = data.at("description").get<string>();

            if (description.empty() || description.length() > 500) {
                res.status = 400;
                res.set_content(
                    R"({"success":false,"message":"Description must contain 1 to 500 characters"})",
                    "application/json"
                );
                return;
            }

            char escapedDescription[1001];

            mysql_real_escape_string(
                conn,
                escapedDescription,
                description.c_str(),
                description.length()
            );

            string query =
                "INSERT INTO complaints (student_id, description) VALUES (" +
                to_string(studentId) + ", '" + escapedDescription + "')";

            if (mysql_query(conn, query.c_str()) != 0) {
                cout << "Complaint insert error: "
                     << mysql_error(conn) << endl;

                res.status = 500;
                res.set_content(
                    R"({"success":false,"message":"Could not save complaint"})",
                    "application/json"
                );
                return;
            }

            res.set_content(
                R"({"success":true,"message":"Complaint submitted successfully","status":"Pending"})",
                "application/json"
            );

        } catch (const exception& e) {
            res.status = 400;
            res.set_content(
                R"({"success":false,"message":"Invalid complaint request"})",
                "application/json"
            );
        }
    });

    server.Get(R"(/api/complaints/(\d+))",
    [&](const httplib::Request& req, httplib::Response& res) {

    string studentId = req.matches[1];

    string query =
        "SELECT complaint_id, description, status, created_at "
        "FROM complaints WHERE student_id = " + studentId +
        " ORDER BY complaint_id DESC";

    if (mysql_query(conn, query.c_str()) != 0) {
        res.status = 500;
        res.set_content(
            R"({"success":false,"message":"Database query failed"})",
            "application/json"
        );
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (result == NULL) {
        res.status = 500;
        res.set_content(
            R"({"success":false,"message":"Could not read database result"})",
            "application/json"
        );
        return;
    }

    json complaints = json::array();
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != NULL) {
        json complaint;
        complaint["complaint_id"] = row[0] ? row[0] : "";
        complaint["description"] = row[1] ? row[1] : "";
        complaint["status"] = row[2] ? row[2] : "";
        complaint["created_at"] = row[3] ? row[3] : "";

        complaints.push_back(complaint);
    }

    mysql_free_result(result);

    json response;
    response["success"] = true;
    response["complaints"] = complaints;

    res.set_content(response.dump(), "application/json");
});

server.Get("/api/admin/dashboard",
    [&](const httplib::Request& req, httplib::Response& res) {

    json response;

    string query =
        "SELECT "
        "(SELECT COUNT(*) FROM students), "
        "(SELECT COUNT(*) FROM rooms), "
        "(SELECT COUNT(*) FROM allocations), "
        "(SELECT COUNT(*) FROM complaints WHERE status <> 'Resolved')";

    if (mysql_query(conn, query.c_str()) != 0) {
        res.status = 500;
        response["success"] = false;
        response["message"] = mysql_error(conn);
        res.set_content(response.dump(), "application/json");
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (result == NULL) {
        res.status = 500;
        response["success"] = false;
        response["message"] = mysql_error(conn);
        res.set_content(response.dump(), "application/json");
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    response["success"] = true;
    response["students"] = row[0] ? stoi(row[0]) : 0;
    response["totalRooms"] = row[1] ? stoi(row[1]) : 0;
    response["occupiedRooms"] = row[2] ? stoi(row[2]) : 0;
    response["unresolvedComplaints"] = row[3] ? stoi(row[3]) : 0;

    mysql_free_result(result);

    res.set_content(response.dump(), "application/json");
});
server.Get("/api/complaints",
    [&](const httplib::Request& req, httplib::Response& res) {

    string query =
        "SELECT complaint_id, student_id, description, status, created_at "
        "FROM complaints ORDER BY complaint_id DESC";

    if (mysql_query(conn, query.c_str()) != 0) {
        res.status = 500;
        res.set_content(
            R"({"success":false,"message":"Database query failed"})",
            "application/json"
        );
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (result == NULL) {
        res.status = 500;
        res.set_content(
            R"({"success":false,"message":"Could not read database result"})",
            "application/json"
        );
        return;
    }

    json complaints = json::array();
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)) != NULL) {
        json complaint;
        complaint["complaint_id"] = row[0] ? row[0] : "";
        complaint["student_id"] = row[1] ? row[1] : "";
        complaint["description"] = row[2] ? row[2] : "";
        complaint["status"] = row[3] ? row[3] : "";
        complaint["created_at"] = row[4] ? row[4] : "";

        complaints.push_back(complaint);
    }

    mysql_free_result(result);

    json response;
    response["success"] = true;
    response["complaints"] = complaints;

    res.set_content(response.dump(), "application/json");
});


    cout << "Server started on port 8080...\n";

    server.listen("0.0.0.0", 8080);

    mysql_close(conn);
    return 0;
}