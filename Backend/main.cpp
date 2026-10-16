
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

    cout << "Server started on port 8080...\n";

    server.listen("0.0.0.0", 8080);

    mysql_close(conn);
    return 0;
}