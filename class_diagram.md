# 📐 HostelOS — UML Class Diagram

This document provides the complete Object-Oriented UML Class Diagram and specifications extracted directly from `Backend/student.cpp` and `Backend/main.cpp`.

---

## 📊 Mermaid Class Diagram

```mermaid
classDiagram
    direction TB

    %% External Infrastructure Types
    class MySQLConnection {
        <<external C struct>>
        MYSQL*
    }

    class HttpServer {
        <<external : httplib::Server>>
        +Post(string pattern, Handler handler)
        +Get(string pattern, Handler handler)
        +Put(string pattern, Handler handler)
        +Options(string pattern, Handler handler)
        +listen(string host, int port)
    }

    %% Domain Entity Class (student.cpp)
    class Student {
        +int student_id
        +string name
        +string email
        +string department
        +string phone
        +string password
        +Student()
        +Student(int id, string n, string e, string d, string p, string pwd)
        +createStudent(conn, id, name, email, department, phone, password, errMsg)$ bool
        +addStudent(conn) void
        +viewStudent(conn) void
        +updateStudent(conn) void
        +deleteStudent(conn) void
    }

    %% API Controller Classes (main.cpp)
    class LoginAPI {
        -MYSQL* conn
        +LoginAPI(MYSQL* c)
        +handleLogin(Request req, Response res) void
    }

    class StudentAPI {
        -MYSQL* conn
        +StudentAPI(MYSQL* c)
        +handleGetStudentById(Request req, Response res) void
        +handleGetAllStudents(Request req, Response res) void
        +handleGetNextId(Request req, Response res) void
        +handleCreateStudent(Request req, Response res) void
    }

    class ComplaintAPI {
        -MYSQL* conn
        +ComplaintAPI(MYSQL* c)
        +handleCreateComplaint(Request req, Response res) void
        +handleGetStudentComplaints(Request req, Response res) void
        +handleGetAllComplaints(Request req, Response res) void
        +handleUpdateComplaintStatus(Request req, Response res) void
    }

    class DashboardAPI {
        -MYSQL* conn
        +DashboardAPI(MYSQL* c)
        +handleGetDashboard(Request req, Response res) void
    }

    class RoomAPI {
        -MYSQL* conn
        +RoomAPI(MYSQL* c)
        +handleGetRooms(Request req, Response res) void
    }

    class AllocationAPI {
        -MYSQL* conn
        +AllocationAPI(MYSQL* c)
        +handleGetAllocations(Request req, Response res) void
        +handleDeallocate(Request req, Response res) void
        +handleCreateAllocation(Request req, Response res) void
    }

    %% Relationships & Dependencies
    StudentAPI ..> Student : invokes createStudent()

    LoginAPI --> MySQLConnection : queries
    StudentAPI --> MySQLConnection : queries
    ComplaintAPI --> MySQLConnection : queries
    DashboardAPI --> MySQLConnection : queries
    RoomAPI --> MySQLConnection : queries
    AllocationAPI --> MySQLConnection : queries
    Student --> MySQLConnection : queries

    HttpServer *-- LoginAPI : dispatches /api/login
    HttpServer *-- StudentAPI : dispatches /api/students
    HttpServer *-- ComplaintAPI : dispatches /api/complaints
    HttpServer *-- DashboardAPI : dispatches /api/admin/dashboard
    HttpServer *-- RoomAPI : dispatches /api/rooms
    HttpServer *-- AllocationAPI : dispatches /api/allocations
```

---

## 📋 Class Specifications & Responsibilities

### 1. `Student` (Domain Entity — `Backend/student.cpp`)
Encapsulates individual student entity data and database persistence routines.

- **Attributes:**
  - `+int student_id`: Unique identifier for the student.
  - `+string name`: Full legal name.
  - `+string email`: University email address (unique).
  - `+string department`: Academic department/major.
  - `+string phone`: Contact phone number.
  - `+string password`: Authentication secret (defaults to `"student123"`).
- **Constructors:**
  - `+Student()`: Default parameterless constructor.
  - `+Student(int id, string n, string e, string d, string p, string pwd)`: Parameterized initialization.
- **Methods:**
  - `+createStudent(...) : bool`: Static persistence helper using SQL injection escaping (`mysql_real_escape_string`) to insert student records.
  - `+addStudent(MYSQL* conn) : void`: CLI helper to insert student via standard I/O.
  - `+viewStudent(MYSQL* conn) : void`: CLI helper to print all students.
  - `+updateStudent(MYSQL* conn) : void`: CLI helper to update student fields.
  - `+deleteStudent(MYSQL* conn) : void`: CLI helper to delete a student by ID.

---

### 2. `LoginAPI` (Controller — `Backend/main.cpp`)
Handles unified credential authentication for both Administrator accounts and Student accounts.

- **Attributes:**
  - `-MYSQL* conn`: Database connection handle.
- **Methods:**
  - `+LoginAPI(MYSQL* c)`: Constructor injecting database connection.
  - `+handleLogin(const httplib::Request& req, httplib::Response& res) : void`:
    - Sanitizes input credentials.
    - Checks `admins` table first (`username` + `password`).
    - Falls back to `students` table (`student_id` or `email` + `password`).
    - Returns JSON payload with user role and authentication token metadata.

---

### 3. `StudentAPI` (Controller — `Backend/main.cpp`)
Manages student profiles, rosters, and registration workflows.

- **Attributes:**
  - `-MYSQL* conn`: Database connection handle.
- **Methods:**
  - `+StudentAPI(MYSQL* c)`: Constructor.
  - `+handleGetStudentById(...) : void`: Returns detailed profile JSON for `:id`.
  - `+handleGetAllStudents(...) : void`: Returns full student roster for admin management.
  - `+handleGetNextId(...) : void`: Calculates `COALESCE(MAX(student_id), 100) + 1` for automated ID assignment.
  - `+handleCreateStudent(...) : void`: Validates input, verifies uniqueness of ID and email, and delegates insertion to `Student::createStudent()`.

---

### 4. `ComplaintAPI` (Controller — `Backend/main.cpp`)
Manages student grievance reporting, triage, and lifecycle resolution.

- **Attributes:**
  - `-MYSQL* conn`: Database connection handle.
- **Methods:**
  - `+ComplaintAPI(MYSQL* c)`: Constructor.
  - `+handleCreateComplaint(...) : void`: Creates a new grievance ticket with `Pending` status.
  - `+handleGetStudentComplaints(...) : void`: Retrieves ticket history filed by a specific student.
  - `+handleGetAllComplaints(...) : void`: Retrieves all tickets across hostel blocks for admin review.
  - `+handleUpdateComplaintStatus(...) : void`: Transition status between `Pending`, `In Progress`, and `Resolved`.

---

### 5. `DashboardAPI` (Controller — `Backend/main.cpp`)
Aggregates high-level facility performance metrics for the administrative interface.

- **Attributes:**
  - `-MYSQL* conn`: Database connection handle.
- **Methods:**
  - `+DashboardAPI(MYSQL* c)`: Constructor.
  - `+handleGetDashboard(...) : void`: Runs composite subqueries to compute student count, room count, active allocations, and unresolved complaints in a single atomic payload.

---

### 6. `RoomAPI` (Controller — `Backend/main.cpp`)
Manages room inventory and real-time occupancy calculations.

- **Attributes:**
  - `-MYSQL* conn`: Database connection handle.
- **Methods:**
  - `+RoomAPI(MYSQL* c)`: Constructor.
  - `+handleGetRooms(...) : void`: Executes `LEFT JOIN` between `rooms` and active `allocations` to return room capacity, occupied beds, and remaining availability.

---

### 7. `AllocationAPI` (Controller — `Backend/main.cpp`)
Coordinates assigning and vacating student bed spaces.

- **Attributes:**
  - `-MYSQL* conn`: Database connection handle.
- **Methods:**
  - `+AllocationAPI(MYSQL* c)`: Constructor.
  - `+handleGetAllocations(...) : void`: Returns active and past allocations.
  - `+handleCreateAllocation(...) : void`: Validates date formatting, ensures student exists, prevents double active allocations, verifies room capacity constraints, and persists new allocation.
  - `+handleDeallocate(...) : void`: Sets `end_date = CURDATE()` to check out a student from a room.

---

## 💡 Object-Oriented Principles Applied

1. **Encapsulation:** All internal database handles and SQL execution steps are kept private inside each API class. External consumers only interact via well-defined REST request handlers.
2. **Controller Pattern:** Web endpoints are decoupled from business logic; each controller manages a dedicated bounded context (Login, Student, Complaint, Room, Allocation, Dashboard).
3. **Dependency Injection:** Database connections are instantiated at the server level (`main()`) and injected into each controller via constructors.
4. **Modularity & Reusability:** The `Student` domain model provides reusable static and instance methods, consumed both by CLI routines and the modern REST API server.
