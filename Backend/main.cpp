//C++ OOP CODE//

#include <iostream>
#include <string>
#include <stdexcept>
#include <mysql/mysql.h>
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

/*
    HOSTEL MANAGEMENT SYSTEM - C++ OOP BACKEND

    HTTP library : cpp-httplib
    JSON library : nlohmann/json
    Database     : MySQL Connector/C

    API base URL:
        http://localhost:8080

    This backend matches the routes already used by:
        login.html
        admin.js
        student.js
*/

/* =========================================================
   DATABASE CLASS
   ========================================================= */
class Database {
private:
    MYSQL* conn = nullptr;

public:
    Database(const string& host,
             const string& user,
             const string& password,
             const string& db,
             unsigned int port = 3306) {

        conn = mysql_init(nullptr);

        if (!conn) {
            throw runtime_error("mysql_init() failed");
        }

        if (!mysql_real_connect(
                conn,
                host.c_str(),
                user.c_str(),
                password.c_str(),
                db.c_str(),
                port,
                nullptr,
                0)) {

            string error = mysql_error(conn);
            mysql_close(conn);
            conn = nullptr;

            throw runtime_error("MySQL connection failed: " + error);
        }

        mysql_set_character_set(conn, "utf8mb4");
    }

    ~Database() {
        if (conn) {
            mysql_close(conn);
        }
    }

    MYSQL* get() {
        return conn;
    }

    void execute(const string& sql) {
        if (mysql_query(conn, sql.c_str()) != 0) {
            throw runtime_error(mysql_error(conn));
        }
    }

    string escape(const string& value) {
        string result;
        result.resize(value.size() * 2 + 1);

        unsigned long length = mysql_real_escape_string(
            conn,
            result.data(),
            value.c_str(),
            static_cast<unsigned long>(value.size())
        );

        result.resize(length);
        return result;
    }

    void begin() {
        execute("START TRANSACTION");
    }

    void commit() {
        execute("COMMIT");
    }

    void rollback() {
        execute("ROLLBACK");
    }
};


/* =========================================================
   MODEL CLASSES
   ========================================================= */
class Student {
public:
    int studentId{};
    string name;
    string rollNumber;
    string email;
    string department;
    string phone;
    string password;
};

class Room {
public:
    int roomId{};
    string roomNumber;
    string block;
    int floor{};
    int capacity{};
};

class Allocation {
public:
    int allocationId{};
    int studentId{};
    string name;
    string room;
    string allocationDate;
};

class Complaint {
public:
    int complaintId{};
    int studentId{};
    string description;
    string status;
    string createdAt;
    string resolvedAt;
};


/* =========================================================
   HOSTEL SERVICE CLASS
   Application/business logic.
   ========================================================= */
class HostelService {
private:
    Database& db;

    static int parseStudentId(const string& id) {
        if (id.rfind("STU", 0) == 0) {
            return stoi(id.substr(3));
        }

        return stoi(id);
    }

    static int parseComplaintId(const string& id) {
        if (id.rfind("C", 0) == 0) {
            return stoi(id.substr(1));
        }

        return stoi(id);
    }

    static string studentCode(int id) {
        string number = to_string(id);

        if (number.length() < 3) {
            number = string(3 - number.length(), '0') + number;
        }

        return "STU" + number;
    }

    static string value(MYSQL_ROW row, unsigned int index) {
        return row[index] ? row[index] : "";
    }

public:
    explicit HostelService(Database& database)
        : db(database) {}


    /* =========================
       LOGIN
       ========================= */
    json login(const string& userId, const string& password) {

        string uid = db.escape(userId);
        string pass = db.escape(password);

        // Admin can log in using admin ID, email, or name.
        string sql =
            "SELECT admin_id FROM admins "
            "WHERE (email='" + uid +
            "' OR CAST(admin_id AS CHAR)='" + uid +
            "' OR name='" + uid +
            "') AND password='" + pass +
            "' LIMIT 1";

        db.execute(sql);

        MYSQL_RES* result = mysql_store_result(db.get());

        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            mysql_free_result(result);

            if (row) {
                return {
                    {"success", true},
                    {"role", "admin"}
                };
            }
        }

        // Student IDs are displayed by the frontend as STU001,
        // STU002, etc., while the database stores INT IDs.
        int studentId;

        try {
            studentId = parseStudentId(userId);
        }
        catch (...) {
            return {{"success", false}};
        }

        sql =
            "SELECT student_id FROM students "
            "WHERE student_id=" + to_string(studentId) +
            " AND password='" + pass +
            "' LIMIT 1";

        db.execute(sql);

        result = mysql_store_result(db.get());

        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            mysql_free_result(result);

            if (row) {
                return {
                    {"success", true},
                    {"role", "student"},
                    {"userId", studentCode(studentId)}
                };
            }
        }

        return {{"success", false}};
    }


    /* =========================
       DASHBOARD
       ========================= */
    json getDashboard() {

        json result;

        string sql = "SELECT COUNT(*) FROM students";
        db.execute(sql);

        MYSQL_RES* res = mysql_store_result(db.get());
        MYSQL_ROW row = mysql_fetch_row(res);

        result["studentCount"] =
            row ? stoi(value(row, 0)) : 0;

        mysql_free_result(res);


        sql = "SELECT COUNT(*) FROM rooms";
        db.execute(sql);

        res = mysql_store_result(db.get());
        row = mysql_fetch_row(res);

        result["roomCount"] =
            row ? stoi(value(row, 0)) : 0;

        mysql_free_result(res);


        sql =
            "SELECT COUNT(DISTINCT room_id) "
            "FROM allocations";

        db.execute(sql);

        res = mysql_store_result(db.get());
        row = mysql_fetch_row(res);

        result["occupiedRooms"] =
            row ? stoi(value(row, 0)) : 0;

        mysql_free_result(res);


        sql =
            "SELECT COUNT(*) FROM complaints "
            "WHERE status <> 'Resolved'";

        db.execute(sql);

        res = mysql_store_result(db.get());
        row = mysql_fetch_row(res);

        result["unresolvedComplaints"] =
            row ? stoi(value(row, 0)) : 0;

        mysql_free_result(res);

        return result;
    }


    /* =========================
       STUDENTS
       ========================= */
    json getStudents() {

        json output = json::array();

        string sql =
            "SELECT "
            "s.student_id, "
            "s.name, "
            "s.roll_number, "
            "s.email, "
            "s.department, "
            "s.phone, "
            "COALESCE(r.room_number,'') "
            "FROM students s "
            "LEFT JOIN allocations a "
            "ON s.student_id = a.student_id "
            "LEFT JOIN rooms r "
            "ON a.room_id = r.room_id "
            "ORDER BY s.student_id";

        db.execute(sql);

        MYSQL_RES* result = mysql_store_result(db.get());
        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result))) {

            int id = stoi(value(row, 0));

            output.push_back({
                {"studentId", studentCode(id)},
                {"name", value(row, 1)},
                {"rollNumber", value(row, 2)},
                {"email", value(row, 3)},
                {"department", value(row, 4)},
                {"phone", value(row, 5)},
                {"room", value(row, 6)}
            });
        }

        mysql_free_result(result);

        return output;
    }


    /* =========================
       SINGLE STUDENT
       ========================= */
    json getStudent(const string& idText) {

        int id = parseStudentId(idText);

        string sql =
            "SELECT "
            "s.student_id, "
            "s.name, "
            "s.roll_number, "
            "s.email, "
            "s.department, "
            "s.phone, "
            "COALESCE(r.room_number,'') "
            "FROM students s "
            "LEFT JOIN allocations a "
            "ON s.student_id = a.student_id "
            "LEFT JOIN rooms r "
            "ON a.room_id = r.room_id "
            "WHERE s.student_id=" + to_string(id) +
            " LIMIT 1";

        db.execute(sql);

        MYSQL_RES* result = mysql_store_result(db.get());
        MYSQL_ROW row = mysql_fetch_row(result);

        if (!row) {
            mysql_free_result(result);
            throw runtime_error("Student not found");
        }

        json student = {
            {"studentId", studentCode(id)},
            {"name", value(row, 1)},
            {"rollNumber", value(row, 2)},
            {"roomNumber", value(row, 6)}
        };

        mysql_free_result(result);


        sql =
            "SELECT COUNT(*) "
            "FROM complaints "
            "WHERE student_id=" + to_string(id);

        db.execute(sql);

        result = mysql_store_result(db.get());
        row = mysql_fetch_row(result);

        int registered =
            row ? stoi(value(row, 0)) : 0;

        mysql_free_result(result);


        sql =
            "SELECT COUNT(*) "
            "FROM complaints "
            "WHERE student_id=" + to_string(id) +
            " AND status='Resolved'";

        db.execute(sql);

        result = mysql_store_result(db.get());
        row = mysql_fetch_row(result);

        int resolved =
            row ? stoi(value(row, 0)) : 0;

        mysql_free_result(result);


        student["complaintsRegistered"] = registered;
        student["complaintsResolved"] = resolved;

        return student;
    }


    /* =========================
       ADD STUDENT
       ========================= */
    void addStudent(const json& data) {

        int studentId =
            parseStudentId(data.at("studentId").get<string>());

        string name =
            db.escape(data.at("name").get<string>());

        string roll =
            db.escape(data.at("rollNumber").get<string>());

        string email =
            db.escape(data.at("email").get<string>());

        string department =
            db.escape(data.at("department").get<string>());

        string phone =
            db.escape(data.at("phone").get<string>());

        string password =
            db.escape(data.at("password").get<string>());

        string roomNumber =
            db.escape(data.at("room").get<string>());


        db.begin();

        try {

            string sql =
                "INSERT INTO students "
                "(student_id,name,roll_number,email,"
                "department,phone,password) "
                "VALUES (" +
                to_string(studentId) + ",'" +
                name + "','" +
                roll + "','" +
                email + "','" +
                department + "','" +
                phone + "','" +
                password + "')";

            db.execute(sql);


            // Find requested room.
            sql =
                "SELECT room_id,capacity "
                "FROM rooms "
                "WHERE room_number='" +
                roomNumber +
                "' LIMIT 1";

            db.execute(sql);

            MYSQL_RES* result =
                mysql_store_result(db.get());

            MYSQL_ROW row =
                mysql_fetch_row(result);

            if (!row) {
                mysql_free_result(result);
                throw runtime_error("Room does not exist");
            }

            int roomId = stoi(value(row, 0));
            int capacity = stoi(value(row, 1));

            mysql_free_result(result);


            // Check room capacity.
            sql =
                "SELECT COUNT(*) "
                "FROM allocations "
                "WHERE room_id=" +
                to_string(roomId);

            db.execute(sql);

            result = mysql_store_result(db.get());
            row = mysql_fetch_row(result);

            int occupied =
                row ? stoi(value(row, 0)) : 0;

            mysql_free_result(result);

            if (occupied >= capacity) {
                throw runtime_error("Room is full");
            }


            // Create allocation.
            sql =
                "INSERT INTO allocations "
                "(student_id,room_id,allocation_date) "
                "VALUES (" +
                to_string(studentId) + "," +
                to_string(roomId) +
                ",CURRENT_DATE())";

            db.execute(sql);

            db.commit();
        }
        catch (...) {
            db.rollback();
            throw;
        }
    }


    /* =========================
       ROOMS
       ========================= */
    json getRooms() {

        json rooms = json::array();

        string sql =
            "SELECT room_id,room_number,block,floor,capacity "
            "FROM rooms "
            "ORDER BY room_number";

        db.execute(sql);

        MYSQL_RES* result =
            mysql_store_result(db.get());

        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result))) {

            int roomId =
                stoi(value(row, 0));

            json room = {
                {"roomId", roomId},
                {"roomNumber", value(row, 1)},
                {"block", value(row, 2)},
                {"floor", stoi(value(row, 3))},
                {"capacity", stoi(value(row, 4))},
                {"students", json::array()}
            };


            string sql2 =
                "SELECT s.roll_number,s.department "
                "FROM allocations a "
                "JOIN students s "
                "ON a.student_id=s.student_id "
                "WHERE a.room_id=" +
                to_string(roomId);

            db.execute(sql2);

            MYSQL_RES* result2 =
                mysql_store_result(db.get());

            MYSQL_ROW row2;

            while ((row2 = mysql_fetch_row(result2))) {

                room["students"].push_back({
                    {"rollNumber", value(row2, 0)},
                    {"department", value(row2, 1)}
                });
            }

            mysql_free_result(result2);

            rooms.push_back(room);
        }

        mysql_free_result(result);

        return rooms;
    }


    /* =========================
       ALLOCATIONS
       ========================= */
    json getAllocations() {

        json output = json::array();

        string sql =
            "SELECT "
            "a.allocation_id, "
            "a.student_id, "
            "s.name, "
            "r.room_number, "
            "DATE_FORMAT(a.allocation_date,'%d/%m/%Y') "
            "FROM allocations a "
            "JOIN students s "
            "ON a.student_id=s.student_id "
            "JOIN rooms r "
            "ON a.room_id=r.room_id "
            "ORDER BY a.allocation_id";

        db.execute(sql);

        MYSQL_RES* result =
            mysql_store_result(db.get());

        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result))) {

            output.push_back({
                {"allocationId", stoi(value(row, 0))},
                {"studentId", studentCode(stoi(value(row, 1)))},
                {"name", value(row, 2)},
                {"room", value(row, 3)},
                {"date", value(row, 4)}
            });
        }

        mysql_free_result(result);

        return output;
    }


    /* =========================
       UPDATE ALLOCATION
       ========================= */
    void updateAllocation(
        const string& idText,
        const string& roomNumber) {

        int studentId =
            parseStudentId(idText);

        string room =
            db.escape(roomNumber);


        string sql =
            "SELECT room_id,capacity "
            "FROM rooms "
            "WHERE room_number='" +
            room +
            "' LIMIT 1";

        db.execute(sql);

        MYSQL_RES* result =
            mysql_store_result(db.get());

        MYSQL_ROW row =
            mysql_fetch_row(result);

        if (!row) {
            mysql_free_result(result);
            throw runtime_error("Room does not exist");
        }

        int roomId =
            stoi(value(row, 0));

        int capacity =
            stoi(value(row, 1));

        mysql_free_result(result);


        // Do not count the current student's existing allocation.
        sql =
            "SELECT COUNT(*) "
            "FROM allocations "
            "WHERE room_id=" +
            to_string(roomId) +
            " AND student_id<>" +
            to_string(studentId);

        db.execute(sql);

        result =
            mysql_store_result(db.get());

        row =
            mysql_fetch_row(result);

        int occupied =
            row ? stoi(value(row, 0)) : 0;

        mysql_free_result(result);

        if (occupied >= capacity) {
            throw runtime_error("Room is full");
        }


        sql =
            "UPDATE allocations "
            "SET room_id=" +
            to_string(roomId) +
            ", allocation_date=CURRENT_DATE() "
            "WHERE student_id=" +
            to_string(studentId);

        db.execute(sql);

        if (mysql_affected_rows(db.get()) == 0) {
            throw runtime_error("Allocation not found");
        }
    }


    /* =========================
       REMOVE STUDENT
       ========================= */
    void removeStudent(const string& idText) {

        int id =
            parseStudentId(idText);

        string sql =
            "DELETE FROM students "
            "WHERE student_id=" +
            to_string(id);

        db.execute(sql);

        if (mysql_affected_rows(db.get()) == 0) {
            throw runtime_error("Student not found");
        }
    }


    /* =========================
       COMPLAINTS - ADMIN
       ========================= */
    json getComplaints() {

        json output = json::array();

        string sql =
            "SELECT "
            "complaint_id, "
            "student_id, "
            "description, "
            "DATE_FORMAT(created_at,'%d/%m/%Y'), "
            "status "
            "FROM complaints "
            "ORDER BY created_at DESC";

        db.execute(sql);

        MYSQL_RES* result =
            mysql_store_result(db.get());

        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result))) {

            string status =
                value(row, 4);

            output.push_back({
                {"complaintId",
                 "C" + to_string(stoi(value(row, 0)))},

                {"studentId",
                 studentCode(stoi(value(row, 1)))},

                {"complaint", value(row, 2)},
                {"date", value(row, 3)},

                {"status",
                 status == "Resolved"
                     ? "Resolved"
                     : "Not Resolved"}
            });
        }

        mysql_free_result(result);

        return output;
    }


    /* =========================
       COMPLAINTS - STUDENT
       ========================= */
    json getStudentComplaints(
        const string& idText) {

        int studentId =
            parseStudentId(idText);

        json output = json::array();

        string sql =
            "SELECT "
            "complaint_id, "
            "student_id, "
            "description, "
            "DATE_FORMAT(created_at,'%d/%m/%Y'), "
            "status "
            "FROM complaints "
            "WHERE student_id=" +
            to_string(studentId) +
            " ORDER BY created_at DESC";

        db.execute(sql);

        MYSQL_RES* result =
            mysql_store_result(db.get());

        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result))) {

            string status =
                value(row, 4);

            output.push_back({
                {"complaintId",
                 "C" + to_string(stoi(value(row, 0)))},

                {"studentId",
                 studentCode(stoi(value(row, 1)))},

                {"complaint", value(row, 2)},
                {"date", value(row, 3)},

                {"status",
                 status == "Resolved"
                     ? "Resolved"
                     : "Not Resolved"}
            });
        }

        mysql_free_result(result);

        return output;
    }


    /* =========================
       ADD COMPLAINT
       ========================= */
    void addComplaint(const json& data) {

        int studentId =
            parseStudentId(
                data.at("studentId").get<string>()
            );

        string description =
            db.escape(
                data.at("complaint").get<string>()
            );

        string sql =
            "INSERT INTO complaints "
            "(student_id,description,status) "
            "VALUES (" +
            to_string(studentId) +
            ",'" +
            description +
            "','Pending')";

        db.execute(sql);
    }


    /* =========================
       RESOLVE COMPLAINT
       ========================= */
    void resolveComplaint(
        const string& complaintIdText) {

        int complaintId =
            parseComplaintId(complaintIdText);

        string sql =
            "UPDATE complaints "
            "SET status='Resolved', "
            "resolved_at=CURRENT_TIMESTAMP "
            "WHERE complaint_id=" +
            to_string(complaintId);

        db.execute(sql);

        if (mysql_affected_rows(db.get()) == 0) {
            throw runtime_error("Complaint not found");
        }
    }
};


/* =========================================================
   HTTP SERVER CLASS
   ========================================================= */
class HostelServer {
private:
    httplib::Server server;
    HostelService& service;

    static void addCors(
        httplib::Response& response) {

        response.set_header(
            "Access-Control-Allow-Origin", "*");

        response.set_header(
            "Access-Control-Allow-Headers",
            "Content-Type");

        response.set_header(
            "Access-Control-Allow-Methods",
            "GET, POST, PUT, DELETE, OPTIONS");
    }

    static void sendJson(
        httplib::Response& response,
        const json& data,
        int status = 200) {

        addCors(response);

        response.status = status;

        response.set_content(
            data.dump(),
            "application/json");
    }

    static void sendError(
        httplib::Response& response,
        const string& message,
        int status = 400) {

        sendJson(
            response,
            {
                {"success", false},
                {"error", message}
            },
            status
        );
    }

    static json parseBody(
        const httplib::Request& request) {

        return json::parse(request.body);
    }

public:
    explicit HostelServer(
        HostelService& hostelService)
        : service(hostelService) {


        /* =========================
           CORS PRE-FLIGHT
           ========================= */
        server.Options(
            R"(.*)",
            [](const httplib::Request&,
               httplib::Response& response) {

                addCors(response);
                response.status = 204;
            }
        );


        /* =========================
           LOGIN
           POST /api/login
           ========================= */
        server.Post(
            "/api/login",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    auto body =
                        parseBody(request);

                    string userId =
                        body.at("userId").get<string>();

                    string password =
                        body.at("password").get<string>();

                    sendJson(
                        response,
                        service.login(
                            userId,
                            password)
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what()
                    );
                }
            }
        );


        /* =========================
           DASHBOARD
           GET /api/dashboard
           ========================= */
        server.Get(
            "/api/dashboard",
            [&](const httplib::Request&,
                httplib::Response& response) {

                try {
                    sendJson(
                        response,
                        service.getDashboard()
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what(),
                        500
                    );
                }
            }
        );


        /* =========================
           ALL STUDENTS
           GET /api/students
           ========================= */
        server.Get(
            "/api/students",
            [&](const httplib::Request&,
                httplib::Response& response) {

                try {
                    sendJson(
                        response,
                        service.getStudents()
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what(),
                        500
                    );
                }
            }
        );


        /* =========================
           ONE STUDENT
           GET /api/students/STU001
           ========================= */
        server.Get(
            R"(/api/students/(.+))",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    sendJson(
                        response,
                        service.getStudent(
                            request.matches[1]
                        )
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what(),
                        404
                    );
                }
            }
        );


        /* =========================
           ADD STUDENT
           POST /api/students
           ========================= */
        server.Post(
            "/api/students",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    service.addStudent(
                        parseBody(request)
                    );

                    sendJson(
                        response,
                        {
                            {"success", true},
                            {"message",
                             "Student added successfully"}
                        },
                        201
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what()
                    );
                }
            }
        );


        /* =========================
           DELETE STUDENT
           DELETE /api/students/STU001
           ========================= */
        server.Delete(
            R"(/api/students/(.+))",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    service.removeStudent(
                        request.matches[1]
                    );

                    sendJson(
                        response,
                        {
                            {"success", true},
                            {"message",
                             "Student removed"}
                        }
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what()
                    );
                }
            }
        );


        /* =========================
           ROOMS
           GET /api/rooms
           ========================= */
        server.Get(
            "/api/rooms",
            [&](const httplib::Request&,
                httplib::Response& response) {

                try {
                    sendJson(
                        response,
                        service.getRooms()
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what(),
                        500
                    );
                }
            }
        );


        /* =========================
           ALLOCATIONS
           GET /api/allocations
           ========================= */
        server.Get(
            "/api/allocations",
            [&](const httplib::Request&,
                httplib::Response& response) {

                try {
                    sendJson(
                        response,
                        service.getAllocations()
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what(),
                        500
                    );
                }
            }
        );


        /* =========================
           UPDATE ALLOCATION
           PUT /api/allocations/STU001
           ========================= */
        server.Put(
            R"(/api/allocations/(.+))",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    auto body =
                        parseBody(request);

                    string room =
                        body.at("room").get<string>();

                    service.updateAllocation(
                        request.matches[1],
                        room
                    );

                    sendJson(
                        response,
                        {
                            {"success", true},
                            {"message",
                             "Room allocation updated"}
                        }
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what()
                    );
                }
            }
        );


        /* =========================
           ALL COMPLAINTS
           GET /api/complaints
           ========================= */
        server.Get(
            "/api/complaints",
            [&](const httplib::Request&,
                httplib::Response& response) {

                try {
                    sendJson(
                        response,
                        service.getComplaints()
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what(),
                        500
                    );
                }
            }
        );


        /* =========================
           STUDENT COMPLAINTS
           GET /api/complaints/student/STU001
           ========================= */
        server.Get(
            R"(/api/complaints/student/(.+))",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    sendJson(
                        response,
                        service.getStudentComplaints(
                            request.matches[1]
                        )
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what(),
                        404
                    );
                }
            }
        );


        /* =========================
           ADD COMPLAINT
           POST /api/complaints
           ========================= */
        server.Post(
            "/api/complaints",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    service.addComplaint(
                        parseBody(request)
                    );

                    sendJson(
                        response,
                        {
                            {"success", true},
                            {"message",
                             "Complaint registered successfully"}
                        },
                        201
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what()
                    );
                }
            }
        );


        /* =========================
           RESOLVE COMPLAINT
           PUT /api/complaints/C1
           ========================= */
        server.Put(
            R"(/api/complaints/(.+))",
            [&](const httplib::Request& request,
                httplib::Response& response) {

                try {

                    auto body =
                        parseBody(request);

                    if (!body.contains("status") ||
                        body.at("status").get<string>()
                            != "Resolved") {

                        throw runtime_error(
                            "Only Resolved status is supported"
                        );
                    }

                    service.resolveComplaint(
                        request.matches[1]
                    );

                    sendJson(
                        response,
                        {
                            {"success", true},
                            {"message",
                             "Complaint marked as resolved"}
                        }
                    );
                }
                catch (const exception& e) {
                    sendError(
                        response,
                        e.what()
                    );
                }
            }
        );
    }


    void run(
        const string& host = "0.0.0.0",
        int port = 8080) {

        cout
            << "Hostel Management C++ Backend\n"
            << "Server: http://localhost:"
            << port << "\n";

        if (!server.listen(host.c_str(), port)) {
            throw runtime_error(
                "Could not start HTTP server"
            );
        }
    }
};


/* =========================================================
   MAIN
   ========================================================= */
int main() {

    try {

        /*
            CHANGE ONLY THESE VALUES
            -------------------------
            DB_USER     = your MySQL username
            DB_PASSWORD = your MySQL password
        */

        const string DB_HOST =
            "localhost";

        const string DB_USER =
            "root";

        const string DB_PASSWORD =
            "YOUR_MYSQL_PASSWORD";

        const string DB_NAME =
            "hostel_management";

        const unsigned int DB_PORT =
            3306;


        Database database(
            DB_HOST,
            DB_USER,
            DB_PASSWORD,
            DB_NAME,
            DB_PORT
        );

        HostelService service(database);

        HostelServer server(service);

        server.run(
            "0.0.0.0",
            8080
        );
    }
    catch (const exception& e) {

        cerr
            << "Backend error: "
            << e.what()
            << '\n';

        return 1;
    }

    return 0;
}
