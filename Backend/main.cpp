#ifdef _WIN32
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#include <iostream>
#include <mysql.h>
#include <cstdlib>

#include "httplib.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

#include "student.cpp"

// ============================================================================
// OOP CLASSES FOR EACH API
// ============================================================================

class LoginAPI {
private:
    MYSQL* conn;
public:
    LoginAPI(MYSQL* c) : conn(c) {}

    void handleLogin(const httplib::Request& req, httplib::Response& res) {
        try {
            json data = json::parse(req.body);

            string userId = data.at("userId").get<string>();
            string password = data.at("password").get<string>();

            cout << "Login request received for: "
                 << userId << endl;

            char* escapedPassword = new char[password.length() * 2 + 1];
            char* escapedUser = new char[userId.length() * 2 + 1];

            mysql_real_escape_string(
                conn,
                escapedPassword,
                password.c_str(),
                password.length()
            );

            mysql_real_escape_string(
                conn,
                escapedUser,
                userId.c_str(),
                userId.length()
            );

            // 1. First, check if credentials match an administrator in the admins table
            string adminQuery =
                "SELECT admin_id FROM admins WHERE username = '"
                + string(escapedUser)
                + "' AND password = '"
                + string(escapedPassword)
                + "'";

            if (mysql_query(conn, adminQuery.c_str()) == 0) {
                MYSQL_RES* adminResult = mysql_store_result(conn);
                if (adminResult != NULL) {
                    my_ulonglong adminRows = mysql_num_rows(adminResult);
                    mysql_free_result(adminResult);
                    if (adminRows > 0) {
                        cout << "Admin login successful for: " << userId << endl;
                        delete[] escapedPassword;
                        delete[] escapedUser;
                        res.set_content(
                            R"({"success":true,"role":"admin"})",
                            "application/json"
                        );
                        return;
                    }
                }
            }

            // 2. Check if credentials match a student (by numeric student_id or email)
            bool isNumeric = !userId.empty() && userId.find_first_not_of("0123456789") == string::npos;
            string studentQuery;
            if (isNumeric) {
                studentQuery =
                    "SELECT student_id, name FROM students WHERE student_id = "
                    + string(escapedUser)
                    + " AND password = '"
                    + string(escapedPassword)
                    + "'";
            } else {
                studentQuery =
                    "SELECT student_id, name FROM students WHERE email = '"
                    + string(escapedUser)
                    + "' AND password = '"
                    + string(escapedPassword)
                    + "'";
            }

            if (mysql_query(conn, studentQuery.c_str()) == 0) {
                MYSQL_RES* studentResult = mysql_store_result(conn);
                if (studentResult != NULL) {
                    my_ulonglong studentRows = mysql_num_rows(studentResult);
                    if (studentRows > 0) {
                        MYSQL_ROW row = mysql_fetch_row(studentResult);
                        string sid = (row && row[0]) ? row[0] : userId;
                        string sname = (row && row[1]) ? row[1] : "";
                        mysql_free_result(studentResult);

                        cout << "Student login successful for: " << userId << " (ID: " << sid << ")" << endl;
                        delete[] escapedPassword;
                        delete[] escapedUser;

                        json resp;
                        resp["success"] = true;
                        resp["role"] = "student";
                        resp["student_id"] = sid;
                        resp["name"] = sname;

                        res.set_content(resp.dump(), "application/json");
                        return;
                    }
                    mysql_free_result(studentResult);
                }
            }

            delete[] escapedPassword;
            delete[] escapedUser;

            cout << "Login failed for: " << userId << endl;
            res.set_content(
                R"({"success":false,"message":"Invalid User ID or Password"})",
                "application/json"
            );

        } catch (const exception& e) {
            cout << "Login error: " << e.what() << endl;

            res.status = 400;
            res.set_content(
                R"({"success":false,"message":"Invalid login request"})",
                "application/json"
            );
        }
    }
};

class StudentAPI {
private:
    MYSQL* conn;
public:
    StudentAPI(MYSQL* c) : conn(c) {}

    void handleGetStudentById(const httplib::Request& req, httplib::Response& res) {
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
    }

    void handleGetAllStudents(const httplib::Request& req, httplib::Response& res) {
        string query =
            "SELECT student_id, name, email, department, phone "
            "FROM students ORDER BY student_id";

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

        json response;
        response["success"] = true;
        response["students"] = json::array();

        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result)) != NULL) {
            json student;

            student["student_id"] = row[0] ? row[0] : "";
            student["name"] = row[1] ? row[1] : "";
            student["email"] = row[2] ? row[2] : "";
            student["department"] = row[3] ? row[3] : "";
            student["phone"] = row[4] ? row[4] : "";

            response["students"].push_back(student);
        }

        mysql_free_result(result);

        res.set_content(response.dump(), "application/json");
    }

    void handleGetNextId(const httplib::Request& req, httplib::Response& res) {
        string query = "SELECT COALESCE(MAX(student_id), 100) + 1 AS next_id FROM students";

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
        int nextId = 101;
        if (row != NULL && row[0] != NULL) {
            try {
                nextId = stoi(row[0]);
            } catch (...) {
                nextId = 101;
            }
        }
        mysql_free_result(result);

        json resp;
        resp["success"] = true;
        resp["next_id"] = nextId;
        res.set_content(resp.dump(), "application/json");
    }

    void handleCreateStudent(const httplib::Request& req, httplib::Response& res) {
        try {
            json data = json::parse(req.body);

            int studentId = 0;
            if (data.contains("student_id")) {
                if (data["student_id"].is_number()) {
                    studentId = data["student_id"].get<int>();
                } else if (data["student_id"].is_string()) {
                    string sidStr = data["student_id"].get<string>();
                    if (!sidStr.empty()) {
                        try {
                            studentId = stoi(sidStr);
                        } catch (...) {
                            studentId = 0;
                        }
                    }
                }
            }

            string name = data.value("name", "");
            string email = data.value("email", "");
            string department = data.value("department", "");
            string phone = data.value("phone", "");
            string password = data.value("password", "student123");
            if (password.empty()) {
                password = "student123";
            }

            // Trim whitespace
            auto trim = [](string& s) {
                while (!s.empty() && isspace(s.front())) s.erase(s.begin());
                while (!s.empty() && isspace(s.back())) s.pop_back();
            };
            trim(name);
            trim(email);
            trim(department);
            trim(phone);
            trim(password);

            if (name.empty()) {
                res.status = 400;
                res.set_content(
                    R"({"success":false,"message":"Student full name is required."})",
                    "application/json"
                );
                return;
            }

            if (email.empty() || email.find('@') == string::npos) {
                res.status = 400;
                res.set_content(
                    R"({"success":false,"message":"A valid email address is required."})",
                    "application/json"
                );
                return;
            }

            // Auto-assign next student ID if not provided
            if (studentId <= 0) {
                string nextIdQuery = "SELECT COALESCE(MAX(student_id), 100) + 1 FROM students";
                if (mysql_query(conn, nextIdQuery.c_str()) == 0) {
                    MYSQL_RES* nextRes = mysql_store_result(conn);
                    if (nextRes != NULL) {
                        MYSQL_ROW row = mysql_fetch_row(nextRes);
                        if (row != NULL && row[0] != NULL) {
                            try {
                                studentId = stoi(row[0]);
                            } catch (...) {
                                studentId = 101;
                            }
                        } else {
                            studentId = 101;
                        }
                        mysql_free_result(nextRes);
                    }
                }
            }

            // Check if student_id already exists
            string checkIdQuery = "SELECT student_id FROM students WHERE student_id = " + to_string(studentId);
            if (mysql_query(conn, checkIdQuery.c_str()) == 0) {
                MYSQL_RES* resId = mysql_store_result(conn);
                if (resId != NULL) {
                    my_ulonglong count = mysql_num_rows(resId);
                    mysql_free_result(resId);
                    if (count > 0) {
                        res.status = 409;
                        res.set_content(
                            R"({"success":false,"message":"Student ID already exists. Please choose a different ID."})",
                            "application/json"
                        );
                        return;
                    }
                }
            }

            // Check if email already exists
            char escEmailCheck[205];
            mysql_real_escape_string(conn, escEmailCheck, email.c_str(), min((int)email.length(), 100));
            string checkEmailQuery = "SELECT student_id FROM students WHERE email = '" + string(escEmailCheck) + "'";
            if (mysql_query(conn, checkEmailQuery.c_str()) == 0) {
                MYSQL_RES* resEmail = mysql_store_result(conn);
                if (resEmail != NULL) {
                    my_ulonglong count = mysql_num_rows(resEmail);
                    mysql_free_result(resEmail);
                    if (count > 0) {
                        res.status = 409;
                        res.set_content(
                            R"({"success":false,"message":"A student with this email address is already registered."})",
                            "application/json"
                        );
                        return;
                    }
                }
            }

            // Use Student class createStudent method
            string errMsg;
            bool ok = Student::createStudent(conn, studentId, name, email, department, phone, password, errMsg);

            if (!ok) {
                cout << "Error inserting student: " << errMsg << endl;
                res.status = 500;
                res.set_content(
                    R"({"success":false,"message":"Failed to save student in database."})",
                    "application/json"
                );
                return;
            }

            cout << "Successfully created new student #" << studentId << " (" << name << ")" << endl;

            json resp;
            resp["success"] = true;
            resp["message"] = "Student account created successfully!";
            resp["student"] = {
                {"student_id", studentId},
                {"name", name},
                {"email", email},
                {"department", department},
                {"phone", phone}
            };

            res.status = 201;
            res.set_content(resp.dump(), "application/json");

        } catch (const exception& e) {
            cout << "Create student exception: " << e.what() << endl;
            res.status = 400;
            res.set_content(
                R"({"success":false,"message":"Invalid request data format."})",
                "application/json"
            );
        }
    }
};

class ComplaintAPI {
private:
    MYSQL* conn;
public:
    ComplaintAPI(MYSQL* c) : conn(c) {}

    void handleCreateComplaint(const httplib::Request& req, httplib::Response& res) {
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
    }

    void handleGetStudentComplaints(const httplib::Request& req, httplib::Response& res) {
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
    }

    void handleGetAllComplaints(const httplib::Request& req, httplib::Response& res) {
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
    }

    void handleUpdateComplaintStatus(const httplib::Request& req, httplib::Response& res) {
        json response;

        try {
            json body = json::parse(req.body);

            string complaintId = req.matches[1].str();
            string status = body.at("status").get<string>();

            if (status != "Pending" && status != "In Progress" &&
                status != "Resolved") {
            res.status = 400;
            response["success"] = false;
            response["message"] = "Invalid complaint status";
            res.set_content(response.dump(), "application/json");
            return;
        }

        string query =
            "UPDATE complaints SET status = '" + status +
            "' WHERE complaint_id = " + complaintId;

        if (mysql_query(conn, query.c_str()) != 0) {
            res.status = 500;
            response["success"] = false;
            response["message"] = mysql_error(conn);
            res.set_content(response.dump(), "application/json");
            return;
        }

        if (mysql_affected_rows(conn) == 0) {
            res.status = 404;
            response["success"] = false;
            response["message"] = "Complaint not found or status unchanged";
            res.set_content(response.dump(), "application/json");
            return;
        }

        response["success"] = true;
        response["message"] = "Complaint status updated successfully";

        res.set_content(response.dump(), "application/json");
    }
    catch (const exception& e) {
        res.status = 400;
        response["success"] = false;
        response["message"] = "Invalid request";
        res.set_content(response.dump(), "application/json");
    }
}
};

class DashboardAPI {
private:
    MYSQL* conn;
public:
    DashboardAPI(MYSQL* c) : conn(c) {}

    void handleGetDashboard(const httplib::Request& req, httplib::Response& res) {
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
    }
};

class RoomAPI {
private:
    MYSQL* conn;
public:
    RoomAPI(MYSQL* c) : conn(c) {}

    void handleGetRooms(const httplib::Request& req, httplib::Response& res) {
        json response;
        json rooms = json::array();

         
        string query =
    "SELECT r.room_id, r.room_number, r.block, r.floor, r.capacity, "
    "COUNT(a.allocation_id) AS occupied, "
    "r.capacity - COUNT(a.allocation_id) AS available "
    "FROM rooms r "
    "LEFT JOIN allocations a ON r.room_id = a.room_id "
    "GROUP BY r.room_id, r.room_number, r.block, r.floor, r.capacity "
    "ORDER BY r.room_id";

        if (mysql_query(conn, query.c_str()) != 0) {
            res.status = 500;
            response["success"] = false;
            response["message"] = "Failed to fetch rooms";
            res.set_content(response.dump(), "application/json");
            return;
        }

        MYSQL_RES* result = mysql_store_result(conn);

        if (result == NULL) {
            res.status = 500;
            response["success"] = false;
            response["message"] = "Failed to read rooms";
            res.set_content(response.dump(), "application/json");
            return;
        }

        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result)) != NULL) {
            json room;
            room["room_id"] = row[0] ? row[0] : "";
            room["room_number"] = row[1] ? row[1] : "";
            room["block"] = row[2] ? row[2] : "";
            room["floor"] = row[3] ? row[3] : "";
            room["capacity"] = row[4] ? row[4] : "";
            room["occupied"] = row[5] ? row[5] : "0";
            room["available"] = row[6] ? row[6] : "0";

            rooms.push_back(room);
        }

        mysql_free_result(result);

        response["success"] = true;
        response["rooms"] = rooms;

        res.set_content(response.dump(), "application/json");
    }
};

class AllocationAPI {
private:
    MYSQL* conn;
public:
    AllocationAPI(MYSQL* c) : conn(c) {}

    void handleGetAllocations(const httplib::Request& req, httplib::Response& res) {
        MYSQL* conn = mysql_init(NULL);

        const char* dbHost = getenv("HOSTEL_DB_HOST");
        const char* dbUser = getenv("HOSTEL_DB_USER");
        const char* dbPassword = getenv("HOSTEL_DB_PASSWORD");
        const char* dbName = getenv("HOSTEL_DB_NAME");

        if (conn == NULL ||
            mysql_real_connect(conn, dbHost, dbUser, dbPassword,
                               dbName, 3306, NULL, 0) == NULL) {
            if (conn != NULL) {
                mysql_close(conn);
            }

            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Database connection failed"})",
                "application/json");
            return;
        }

        string query =
            "SELECT allocation_id, student_id, room_id, "
            "allocation_date, end_date "
            "FROM allocations ORDER BY allocation_id";

        if (mysql_query(conn, query.c_str()) != 0) {
            mysql_close(conn);
            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Allocation query failed"})",
                "application/json");
            return;
        }

        MYSQL_RES* result = mysql_store_result(conn);
        json allocations = json::array();

        if (result != NULL) {
            MYSQL_ROW row;

            while ((row = mysql_fetch_row(result)) != NULL) {
                json allocation;
                allocation["allocation_id"] = row[0] ? row[0] : "";
                allocation["student_id"] = row[1] ? row[1] : "";
                allocation["room_id"] = row[2] ? row[2] : "";
                allocation["allocation_date"] = row[3] ? row[3] : "";
                allocation["end_date"] = row[4] ? row[4] : "";

                allocations.push_back(allocation);
            }

            mysql_free_result(result);
        }

        mysql_close(conn);

        json response;
        response["success"] = true;
        response["allocations"] = allocations;

        res.set_content(response.dump(), "application/json");
    }

    void handleDeallocate(const httplib::Request& req, httplib::Response& res) {
        MYSQL* conn = mysql_init(NULL);

        const char* dbHost = getenv("HOSTEL_DB_HOST");
        const char* dbUser = getenv("HOSTEL_DB_USER");
        const char* dbPassword = getenv("HOSTEL_DB_PASSWORD");
        const char* dbName = getenv("HOSTEL_DB_NAME");

        if (conn == NULL ||
            mysql_real_connect(conn, dbHost, dbUser, dbPassword,
                               dbName, 3306, NULL, 0) == NULL) {
            if (conn != NULL) {
                mysql_close(conn);
            }

            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Database connection failed"})",
                "application/json");
            return;
        }

        int allocationId = stoi(req.matches[1].str());

        MYSQL_STMT* stmt = mysql_stmt_init(conn);

        if (stmt == NULL) {
            mysql_close(conn);
            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Statement initialization failed"})",
                "application/json");
            return;
        }

        const char* query =
            "UPDATE allocations SET end_date = CURDATE() "
            "WHERE allocation_id = ? AND end_date IS NULL";

        if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
            mysql_stmt_close(stmt);
            mysql_close(conn);
            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Failed to prepare deallocation"})",
                "application/json");
            return;
        }

        MYSQL_BIND bind[1]{};
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &allocationId;

        mysql_stmt_bind_param(stmt, bind);

        if (mysql_stmt_execute(stmt) != 0) {
            mysql_stmt_close(stmt);
            mysql_close(conn);
            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Deallocation failed"})",
                "application/json");
            return;
        }

        my_ulonglong affected = mysql_stmt_affected_rows(stmt);

        mysql_stmt_close(stmt);
        mysql_close(conn);

        if (affected == 0) {
            res.status = 400;
            res.set_content(
                R"({"success":false,"message":"Allocation not found or already deallocated"})",
                "application/json");
            return;
        }

        res.set_content(
            R"({"success":true,"message":"Room deallocated successfully"})",
            "application/json");
    }

    void handleCreateAllocation(const httplib::Request& req, httplib::Response& res) {
        json data;

        try {
            data = json::parse(req.body);
        } catch (...) {
            res.status = 400;
            res.set_content(
                R"({"success":false,"message":"Invalid JSON"})",
                "application/json");
            return;
        }

        if (!data.contains("student_id") ||
            !data.contains("room_id") ||
            !data.contains("allocation_date")) {
            res.status = 400;
            res.set_content(
                R"({"success":false,"message":"Missing required fields"})",
                "application/json");
            return;
        }

        int studentId, roomId;
        string allocationDate = data["allocation_date"].get<string>();

        try {
            studentId = stoi(data["student_id"].get<string>());
            roomId = stoi(data["room_id"].get<string>());
        } catch (...) {
            try {
                studentId = data["student_id"].get<int>();
                roomId = data["room_id"].get<int>();
            } catch (...) {
                res.status = 400;
                res.set_content(
                    R"({"success":false,"message":"Invalid student or room ID"})",
                    "application/json");
                return;
            }
        }

        MYSQL* conn = mysql_init(NULL);

        const char* dbHost = getenv("HOSTEL_DB_HOST");
        const char* dbUser = getenv("HOSTEL_DB_USER");
        const char* dbPassword = getenv("HOSTEL_DB_PASSWORD");
        const char* dbName = getenv("HOSTEL_DB_NAME");

        if (conn == NULL ||
            mysql_real_connect(conn, dbHost, dbUser, dbPassword,
                               dbName, 3306, NULL, 0) == NULL) {
            if (conn != NULL) mysql_close(conn);

            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Database connection failed"})",
                "application/json");
            return;
        }

        // Validate the allocation date.
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        const char* dateQuery =
            "SELECT STR_TO_DATE(?, '%Y-%m-%d') IS NOT NULL "
            "AND DATE_FORMAT(STR_TO_DATE(?, '%Y-%m-%d'), '%Y-%m-%d') = ?";

        bool validDate = false;

        if (stmt != NULL && mysql_stmt_prepare(stmt, dateQuery, strlen(dateQuery)) == 0) {
            MYSQL_BIND params[3] = {};
            unsigned long dateLength = allocationDate.length();

            for (int i = 0; i < 3; i++) {
                params[i].buffer_type = MYSQL_TYPE_STRING;
                params[i].buffer = &allocationDate[0];
                params[i].buffer_length = dateLength;
                params[i].length = &dateLength;
            }

            int dateValidResult = 0;
            MYSQL_BIND output[1] = {};
            output[0].buffer_type = MYSQL_TYPE_LONG;
            output[0].buffer = &dateValidResult;

            mysql_stmt_bind_param(stmt, params);
            mysql_stmt_bind_result(stmt, output);

            if (mysql_stmt_execute(stmt) == 0 &&
                mysql_stmt_fetch(stmt) == 0) {
                validDate = dateValidResult == 1;
            }

            mysql_stmt_close(stmt);
        }

        if (!validDate) {
            mysql_close(conn);
            res.status = 400;
            res.set_content(
                R"({"success":false,"message":"Invalid allocation date"})",
                "application/json");
            return;
        }

        // Check that the student exists.
        string checkStudent =
            "SELECT student_id FROM students WHERE student_id = " +
            to_string(studentId);

        if (mysql_query(conn, checkStudent.c_str()) != 0) {
            mysql_close(conn);
            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Student check failed"})",
                "application/json");
            return;
        }

        MYSQL_RES* result = mysql_store_result(conn);
        bool studentExists = result && mysql_num_rows(result) > 0;
        if (result) mysql_free_result(result);

        if (!studentExists) {
            mysql_close(conn);
            res.status = 404;
            res.set_content(
                R"({"success":false,"message":"Student not found"})",
                "application/json");
            return;
        }

        // Check that the room exists and get its capacity.
        string checkRoom =
            "SELECT capacity FROM rooms WHERE room_id = " +
            to_string(roomId);

        if (mysql_query(conn, checkRoom.c_str()) != 0) {
            mysql_close(conn);
            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Room check failed"})",
                "application/json");
            return;
        }

        result = mysql_store_result(conn);
        bool roomExists = result && mysql_num_rows(result) > 0;
        int capacity = 0;

        if (roomExists) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) capacity = atoi(row[0]);
        }

        if (result) mysql_free_result(result);

        if (!roomExists) {
            mysql_close(conn);
            res.status = 404;
            res.set_content(
                R"({"success":false,"message":"Room not found"})",
                "application/json");
            return;
        }

        // Prevent a second active allocation for this student.
        string checkAllocation =
            "SELECT allocation_id FROM allocations "
            "WHERE student_id = " + to_string(studentId) +
            " AND end_date IS NULL";

        if (mysql_query(conn, checkAllocation.c_str()) != 0) {
            mysql_close(conn);
            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Allocation check failed"})",
                "application/json");
            return;
        }

        result = mysql_store_result(conn);
        bool alreadyAllocated = result && mysql_num_rows(result) > 0;
        if (result) mysql_free_result(result);

        if (alreadyAllocated) {
            mysql_close(conn);
            res.status = 409;
            res.set_content(
                R"({"success":false,"message":"Student already has an active allocation"})",
                "application/json");
            return;
        }

        // Count current occupants of this room.
        string countQuery =
            "SELECT COUNT(*) FROM allocations WHERE room_id = " +
            to_string(roomId) + " AND end_date IS NULL";

        if (mysql_query(conn, countQuery.c_str()) != 0) {
            mysql_close(conn);
            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Room occupancy check failed"})",
                "application/json");
            return;
        }

        result = mysql_store_result(conn);
        int occupied = 0;

        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row && row[0]) occupied = atoi(row[0]);
            mysql_free_result(result);
        }

        if (occupied >= capacity) {
            mysql_close(conn);
            res.status = 409;
            res.set_content(
                R"({"success":false,"message":"Room is full"})",
                "application/json");
            return;
        }

        // Insert the allocation.
        MYSQL_STMT* insertStmt = mysql_stmt_init(conn);
        const char* insertQuery =
            "INSERT INTO allocations "
            "(student_id, room_id, allocation_date) VALUES (?, ?, ?)";

        if (insertStmt == NULL ||
            mysql_stmt_prepare(insertStmt, insertQuery, strlen(insertQuery)) != 0) {
            if (insertStmt) mysql_stmt_close(insertStmt);
            mysql_close(conn);
            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Could not prepare allocation"})",
                "application/json");
            return;
        }

        MYSQL_BIND insertParams[3] = {};
        insertParams[0].buffer_type = MYSQL_TYPE_LONG;
        insertParams[0].buffer = &studentId;
        insertParams[1].buffer_type = MYSQL_TYPE_LONG;
        insertParams[1].buffer = &roomId;
        unsigned long allocationDateLength = allocationDate.length();
        insertParams[2].buffer_type = MYSQL_TYPE_STRING;
        insertParams[2].buffer = &allocationDate[0];
        insertParams[2].buffer_length = allocationDateLength;
        insertParams[2].length = &allocationDateLength;

        mysql_stmt_bind_param(insertStmt, insertParams);

        if (mysql_stmt_execute(insertStmt) != 0) {
            mysql_stmt_close(insertStmt);
            mysql_close(conn);
            res.status = 500;
            res.set_content(
                R"({"success":false,"message":"Failed to save allocation"})",
                "application/json");
            return;
        }

        mysql_stmt_close(insertStmt);
        mysql_close(conn);

        res.set_content(
            R"({"success":true,"message":"Room allocated successfully"})",
            "application/json");
    }
};

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================
int main() {
    
const char* dbHost = getenv("HOSTEL_DB_HOST");
const char* dbUser = getenv("HOSTEL_DB_USER");
const char* dbPassword = getenv("HOSTEL_DB_PASSWORD");
const char* dbName = getenv("HOSTEL_DB_NAME");

if (dbHost == NULL || dbUser == NULL ||
    dbPassword == NULL || dbName == NULL) {
    cout << "Database environment variables are missing!\n";
    return 1;
}

MYSQL* conn = mysql_init(NULL);

if (conn == NULL) {
    cout << "MySQL initialization failed\n";
    return 1;
}

if (mysql_real_connect(conn, dbHost, dbUser, dbPassword,
                       dbName, 3306, NULL, 0) == NULL) {
    cout << "Database connection failed: "
         << mysql_error(conn) << endl;
    mysql_close(conn);
    return 1;
}


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

    // Initialize API Class Objects (OOP Pattern)
    LoginAPI loginApi(conn);
    StudentAPI studentApi(conn);
    ComplaintAPI complaintApi(conn);
    DashboardAPI dashboardApi(conn);
    RoomAPI roomApi(conn);
    AllocationAPI allocationApi(conn);

    // 1. Login API Endpoint
    server.Post("/api/login",
        [&](const httplib::Request& req, httplib::Response& res) {
            loginApi.handleLogin(req, res);
        });

    // 2. Student Profile API Endpoint
    server.Get(R"(/api/students/(\d+))",
        [&](const httplib::Request& req, httplib::Response& res) {
            studentApi.handleGetStudentById(req, res);
        });

    // 3. Complaint Submission API Endpoint
    server.Post("/api/complaints",
        [&](const httplib::Request& req, httplib::Response& res) {
            complaintApi.handleCreateComplaint(req, res);
        });

    // 4. Student Complaints History API Endpoint
    server.Get(R"(/api/complaints/(\d+))",
        [&](const httplib::Request& req, httplib::Response& res) {
            complaintApi.handleGetStudentComplaints(req, res);
        });

    // 5. Admin Dashboard Metrics API Endpoint
    server.Get("/api/admin/dashboard",
        [&](const httplib::Request& req, httplib::Response& res) {
            dashboardApi.handleGetDashboard(req, res);
        });

    // 6. Admin All Complaints API Endpoint
    server.Get("/api/complaints",
        [&](const httplib::Request& req, httplib::Response& res) {
            complaintApi.handleGetAllComplaints(req, res);
        });

    // 7. Update Complaint Status API Endpoint
    server.Put(R"(/api/complaints/(\d+))",
        [&](const httplib::Request& req, httplib::Response& res) {
            complaintApi.handleUpdateComplaintStatus(req, res);
        });

    // 8. Rooms List & Occupancy API Endpoint
    server.Get("/api/rooms",
        [&](const httplib::Request& req, httplib::Response& res) {
            roomApi.handleGetRooms(req, res);
        });

    // 9. Allocations List API Endpoint
    server.Get("/api/allocations",
        [&](const httplib::Request& req, httplib::Response& res) {
            allocationApi.handleGetAllocations(req, res);
        });

    // 10. Deallocate Room API Endpoint
    server.Put("/api/allocations/(\\d+)/deallocate",
        [&](const httplib::Request& req, httplib::Response& res) {
            allocationApi.handleDeallocate(req, res);
        });

    // 11. Create Allocation API Endpoint
    server.Post("/api/allocations",
        [&](const httplib::Request& req, httplib::Response& res) {
            allocationApi.handleCreateAllocation(req, res);
        });

    // 12. Admin All Students API Endpoint
    server.Get("/api/admin/students",
        [&](const httplib::Request& req, httplib::Response& res) {
            studentApi.handleGetAllStudents(req, res);
        });

    // 13. Get Next Student ID API Endpoint
    server.Get("/api/students/next-id",
        [&](const httplib::Request& req, httplib::Response& res) {
            studentApi.handleGetNextId(req, res);
        });

    // 14. Register Student API Endpoints
    server.Post("/api/students",
        [&](const httplib::Request& req, httplib::Response& res) {
            studentApi.handleCreateStudent(req, res);
        });

    server.Post("/api/admin/students",
        [&](const httplib::Request& req, httplib::Response& res) {
            studentApi.handleCreateStudent(req, res);
        });
                                                                                                                                                            
    cout << "Server started on port 8080...\n";

    server.listen("0.0.0.0", 8080);

    mysql_close(conn);
    return 0;
}