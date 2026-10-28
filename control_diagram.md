# 🔄 HostelOS — Control Flow Diagrams

[![Mermaid](https://img.shields.io/badge/Mermaid-Rendered-blue?logo=mermaid)](https://mermaid.js.org/)
[![C++17](https://img.shields.io/badge/Backend-C%2B%2B17-00599C?logo=c%2B%2B)](Backend/main.cpp)
[![MySQL](https://img.shields.io/badge/Database-MySQL%208.0-4479A1?logo=mysql)](Database/database_schema.sql)

This document details the **Control Flow Diagrams (CFD)** for **HostelOS (Hostel Management System)**. It models the operational sequences, decision branches, error conditions, and state transitions across both the Client Presentation Tier and the C++ Application Tier.

---

## 📌 Table of Contents

- [1. Master System Runtime Control Flow](#1-master-system-runtime-control-flow)
- [2. User Authentication & Session Routing Flow](#2-user-authentication--session-routing-flow)
- [3. Room Allocation & Capacity Validation Flow](#3-room-allocation--capacity-validation-flow)
- [4. Resident Checkout & Deallocation Flow](#4-resident-checkout--deallocation-flow)
- [5. Grievance Lodging & Status Triage Flow](#5-grievance-lodging--status-triage-flow)
- [6. Student Onboarding & Registration Flow](#6-student-onboarding--registration-flow)
- [7. Control Flow State & Decision Matrix](#7-control-flow-state--decision-matrix)

---

## 1. Master System Runtime Control Flow

The master control flow captures the server lifecycle from startup and database handshake to HTTP request dispatching and graceful shutdown.

```mermaid
flowchart TD
    Start([System Initialization / main.cpp]) --> LoadEnv[Load Environment Variables<br/>DB_HOST, DB_USER, DB_PASS, DB_NAME, PORT]
    LoadEnv --> ConnectDB[Initialize MySQL Connection via mysql_real_connect]
    
    ConnectDB --> DBCheck{Connection Successful?}
    DBCheck -- No --> LogDBErr[Log MySQL Connection Error to stderr]
    LogDBErr --> Terminate([Exit Failure 1])
    
    DBCheck -- Yes --> InitControllers["Instantiate API Controllers<br/>(LoginAPI, StudentAPI, RoomAPI,<br/>AllocationAPI, ComplaintAPI, DashboardAPI)"]
    
    InitControllers --> BindRoutes["Register REST Endpoints & CORS Headers<br/>httplib::Server (POST, GET, PUT, OPTIONS)"]
    
    BindRoutes --> StartListen["Listen on 0.0.0.0:8080<br/>(Multi-threaded HTTP Server)"]
    
    StartListen --> ReqWait{Incoming HTTP Request?}
    ReqWait -- Yes --> PreflightCheck{Method == OPTIONS?}
    
    PreflightCheck -- Yes --> SendCORS[Set CORS Headers 204 No Content] --> ReqWait
    
    PreflightCheck -- No --> MatchRoute{Match API Route?}
    MatchRoute -- No --> Return404[Return 404 Not Found] --> ReqWait
    
    MatchRoute -- Yes --> Dispatch[Dispatch Request to Controller Handler]
    Dispatch --> ExecHandler[Execute Controller Logic & DB Query]
    ExecHandler --> ReturnJSON[Serialize nlohmann::json Response & HTTP Status]
    ReturnJSON --> ReqWait
    
    StartListen -- SIGINT / SIGTERM --> Cleanup[Close MySQL Handle & Free Memory]
    Cleanup --> Stop([Server Shutdown])
```

---

## 2. User Authentication & Session Routing Flow

Handles role-based authentication (`POST /api/login`) by verifying credentials sequentially against administrators and students.

```mermaid
flowchart TD
    LoginStart([User Enters Credentials on login.html]) --> Submit["Submit Form (userId, password)"]
    Submit --> ClientValidation{Empty Inputs?}
    
    ClientValidation -- Yes --> ShowClientErr["Display 'Please enter credentials'"]
    ShowClientErr --> LoginStart
    
    ClientValidation -- No --> PostLogin["POST /api/login (JSON Body)"]
    PostLogin --> ParseJSON{Valid JSON Body?}
    
    ParseJSON -- No --> Err400[Return HTTP 400 Bad Request]
    
    ParseJSON -- Yes --> EscapeInput["mysql_real_escape_string(escapedUser, escapedPass)"]
    EscapeInput --> QueryAdmin["Query: SELECT admin_id FROM admins WHERE username = ..."]
    
    QueryAdmin --> CheckAdminMatch{Admin Found?}
    CheckAdminMatch -- Yes --> RespAdmin["Return HTTP 200 {success: true, role: 'admin'}"]
    RespAdmin --> StoreAdminSession["Client stores admin session in localStorage"]
    StoreAdminSession --> RedirectAdmin["Redirect to admin.html"]
    RedirectAdmin --> EndAdmin([Admin Command Center Ready])
    
    CheckAdminMatch -- No --> CheckNumeric{Is userId Numeric?}
    CheckNumeric -- Yes --> QueryStudId["Query: SELECT student_id, name FROM students<br/>WHERE student_id = ... AND password = ..."]
    CheckNumeric -- No --> QueryStudEmail["Query: SELECT student_id, name FROM students<br/>WHERE email = ... AND password = ..."]
    
    QueryStudId --> CheckStudentMatch{Student Found?}
    QueryStudEmail --> CheckStudentMatch
    
    CheckStudentMatch -- Yes --> RespStudent["Return HTTP 200<br/>{success: true, role: 'student', student_id, name}"]
    RespStudent --> StoreStudentSession["Client stores student_id & role in localStorage"]
    StoreStudentSession --> RedirectStudent["Redirect to student.html"]
    RedirectStudent --> EndStudent([Student Portal Ready])
    
    CheckStudentMatch -- No --> AuthFail["Return HTTP 401 Unauthorized<br/>{success: false, message: 'Invalid credentials'}"]
    AuthFail --> ShowAuthError["Display invalid credentials toast notification"]
    ShowAuthError --> LoginStart
```

---

## 3. Room Allocation & Capacity Validation Flow

Executes the atomic business rules for allocating a resident to a room (`POST /api/allocations`), preventing duplicate allocations and overbooking.

```mermaid
flowchart TD
    AllocStart([Admin selects Student and Room on admin.html]) --> SubmitAlloc["POST /api/allocations {student_id, room_id}"]
    SubmitAlloc --> CheckParams{student_id & room_id present?}
    
    CheckParams -- No --> Ret400[Return HTTP 400 Bad Request]
    
    CheckParams -- Yes --> QueryActiveAlloc["Query: SELECT allocation_id FROM allocations<br/>WHERE student_id = :id AND end_date IS NULL"]
    
    QueryActiveAlloc --> HasActiveAlloc{Already Allocated?}
    HasActiveAlloc -- Yes --> RetDupErr["Return HTTP 400<br/>{success: false, message: 'Student already has an active allocation'}"]
    RetDupErr --> ShowDupAlert["Display warning banner on Admin UI"]
    
    HasActiveAlloc -- No --> QueryRoomCap["Query: SELECT capacity, COUNT(allocation_id) AS current_occ<br/>FROM rooms LEFT JOIN allocations ... WHERE room_id = :id"]
    
    QueryRoomCap --> RoomExists{Room Exists?}
    RoomExists -- No --> Ret404[Return HTTP 404 Room Not Found]
    
    RoomExists -- Yes --> CheckCap{current_occ >= capacity?}
    CheckCap -- Yes --> RetFullErr["Return HTTP 400<br/>{success: false, message: 'Room has reached maximum capacity'}"]
    RetFullErr --> ShowFullAlert["Display 'Room Full' alert on Admin UI"]
    
    CheckCap -- No --> InsertAlloc["INSERT INTO allocations (student_id, room_id, allocation_date)<br/>VALUES (:student_id, :room_id, CURDATE())"]
    
    InsertAlloc --> ExecInsert{Insert Success?}
    ExecInsert -- No --> Ret500[Return HTTP 500 DB Transaction Failed]
    
    ExecInsert -- Yes --> Ret201["Return HTTP 201 Created<br/>{success: true, allocation_id: :new_id}"]
    Ret201 --> RefreshAdmin["Client triggers reload of Room Matrix & Allocations Table"]
    RefreshAdmin --> AllocEnd([Allocation Active & Room Capacity Incremented])
```

---

## 4. Resident Checkout & Deallocation Flow

Processes resident departure (`PUT /api/allocations/:id/deallocate`) by terminating active residency and freeing hostel vacancy.

```mermaid
flowchart TD
    DeallocStart([Admin clicks 'Deallocate' on active allocation]) --> ConfirmModal{User Confirms Action?}
    ConfirmModal -- Cancel --> DeallocCancel([Operation Cancelled])
    
    ConfirmModal -- Confirm --> SendDealloc["PUT /api/allocations/:id/deallocate"]
    SendDealloc --> VerifyAllocId{allocation_id valid integer?}
    
    VerifyAllocId -- No --> Err400[Return HTTP 400 Invalid ID]
    
    VerifyAllocId -- Yes --> CheckExisting["Query: SELECT allocation_id, end_date FROM allocations<br/>WHERE allocation_id = :id"]
    
    CheckExisting --> Exists{Record Exists?}
    Exists -- No --> Err404[Return HTTP 404 Allocation Record Not Found]
    
    Exists -- Yes --> AlreadyEnded{end_date IS NOT NULL?}
    AlreadyEnded -- Yes --> ErrAlreadyDone["Return HTTP 400 Already Deallocated"]
    
    AlreadyEnded -- No --> ExecUpdate["UPDATE allocations SET end_date = CURRENT_DATE()<br/>WHERE allocation_id = :id"]
    
    ExecUpdate --> UpdateSuccess{Update Successful?}
    UpdateSuccess -- No --> Err500[Return HTTP 500 Database Error]
    
    UpdateSuccess -- Yes --> RetSuccess["Return HTTP 200 {success: true, message: 'Deallocated successfully'}"]
    RetSuccess --> ClientUIUpdate["Admin UI updates allocation status badge to 'Completed'"]
    ClientUIUpdate --> RecalcVacancies["Dashboard recalculates available vacancies (+1)"]
    RecalcVacancies --> DeallocEnd([Checkout Complete])
```

---

## 5. Grievance Lodging & Status Triage Flow

Tracks the complaint lifecycle from resident submission on `student.html` through administrative status progression on `admin.html`.

```mermaid
flowchart TD
    subgraph ResidentFlow ["Student Grievance Submission (student.html)"]
        GrievStart([Student logs into portal]) --> OpenGrievModal[Open 'Lodge Grievance' Form]
        OpenGrievModal --> EnterDesc[Enter Complaint Category & Description]
        EnterDesc --> ValDesc{Description Length >= 5 chars?}
        
        ValDesc -- No --> PromptDesc[Show validation warning: 'Description too short']
        PromptDesc --> EnterDesc
        
        ValDesc -- Yes --> PostGriev["POST /api/complaints<br/>{student_id: :id, description: :desc}"]
        PostGriev --> InsertComplaint["INSERT INTO complaints (student_id, description, status, created_at)<br/>VALUES (:id, :desc, 'Pending', NOW())"]
        InsertComplaint --> GrievSaved[Return HTTP 201 Created]
        GrievSaved --> RefreshStudList[Reload Student Grievance Timeline with 'Pending' badge]
    end

    subgraph AdminTriageFlow ["Admin Grievance Triage (admin.html)"]
        AdminInspect[Warden inspects Grievance Triage board] --> SelectTicket[Select Complaint Ticket]
        SelectTicket --> PickNewStatus{Select New Status}
        
        PickNewStatus -->|Assign Technician| SetInProgress["Status: 'In Progress'"]
        PickNewStatus -->|Issue Resolved| SetResolved["Status: 'Resolved'"]
        PickNewStatus -->|Re-open| SetPending["Status: 'Pending'"]
        
        SetInProgress --> PutStatus["PUT /api/complaints/:id<br/>{status: 'In Progress'}"]
        SetResolved --> PutStatus["PUT /api/complaints/:id<br/>{status: 'Resolved'}"]
        SetPending --> PutStatus["PUT /api/complaints/:id<br/>{status: 'Pending'}"]
        
        PutStatus --> ValidateStatus{Status in allowed set?}
        ValidateStatus -- No --> RejectStatus[Return HTTP 400 Invalid Status]
        
        ValidateStatus -- Yes --> UpdateDB["UPDATE complaints SET status = :status WHERE complaint_id = :id"]
        UpdateDB --> Triage200["Return HTTP 200 {success: true}"]
        Triage200 --> UpdateBadge[Admin UI updates status badge color & counter]
        UpdateBadge --> NotificationSync([Status updated across Student & Admin views])
    end

    RefreshStudList -.-> AdminInspect
```

---

## 6. Student Onboarding & Registration Flow

Guides new resident creation (`POST /api/students`), auto-generating identifiers and ensuring contact uniqueness.

```mermaid
flowchart TD
    RegStart([Admin opens add-student.html]) --> FetchNextId["GET /api/students/next-id"]
    FetchNextId --> ComputeNextId["SELECT COALESCE(MAX(student_id), 100) + 1 FROM students"]
    ComputeNextId --> PopulateField[Pre-populate Student ID input field]
    
    PopulateField --> AdminFill[Admin inputs Name, Email, Dept, Phone, Password]
    AdminFill --> ClientVal{Form Fields Valid & Complete?}
    
    ClientVal -- No --> HighlightErrors[Highlight required input fields]
    HighlightErrors --> AdminFill
    
    ClientVal -- Yes --> SubmitNew["POST /api/students (Payload)"]
    SubmitNew --> CheckUniqueEmail["SELECT student_id FROM students WHERE email = :email"]
    
    CheckUniqueEmail --> EmailTaken{Email Exists?}
    EmailTaken -- Yes --> RetConflict["Return HTTP 409 Conflict<br/>{success: false, message: 'Email already registered'}"]
    RetConflict --> AlertAdminEmail[Display 'Email already exists' warning]
    
    EmailTaken -- No --> CallEntityMethod["Student::createStudent(conn, id, name, email, dept, phone, pwd)"]
    CallEntityMethod --> ExecInsertStud["INSERT INTO students (student_id, name, email, department, phone, password)<br/>VALUES (...)"]
    
    ExecInsertStud --> InsertStatus{Query Success?}
    InsertStatus -- No --> RetDBErr[Return HTTP 500 Registration Failed]
    
    InsertStatus -- Yes --> Ret201Registered["Return HTTP 201 Created<br/>{success: true, student_id: :id}"]
    Ret201Registered --> ShowSuccessModal[Display registration success confirmation]
    ShowSuccessModal --> Choice{Next Action?}
    Choice -- Allocate Now --> RedirectAlloc[Navigate to Room Allocation in admin.html]
    Choice -- Register Another --> ClearForm[Clear Form and fetch updated Next ID]
```

---

## 7. Control Flow State & Decision Matrix

| Subsystem / Operation | Trigger Event | Primary Decision | Success Branch | Failure / Exception Branch |
| :--- | :--- | :--- | :--- | :--- |
| **Server Engine** | `main()` execution | MySQL connection handshake | Starts HTTP listening on port 8080 | Emits error code to stderr and terminates process |
| **Login API** | Client submits credentials | Role verification order | 1. Admin matched ➔ `admin.html`<br/>2. Student matched ➔ `student.html` | Return 401 Unauthorized; show error notification |
| **Room Allocation** | Admin assigns student | 1. Duplicate check<br/>2. Room capacity check | Inserts record into `allocations`; increments occupancy | 1. Return 400 (Already allocated)<br/>2. Return 400 (Room capacity full) |
| **Room Deallocation** | Admin terminates residency | Record existence & active status | Sets `end_date = CURDATE()`; room vacancy increments | Return 400 / 404 (Invalid ID or already checked out) |
| **Grievance Lodging** | Student submits ticket | Field length & authentication | Ticket inserted with status `'Pending'` | Return 400 (Missing parameters or invalid student) |
| **Grievance Triage** | Warden changes status | Target status validity | Record updated; UI timeline refreshed | Return 400 (Unsupported status enumeration) |
| **Student Registration** | Admin onboards resident | Unique email/phone constraints | Entity persisted; Next ID computed | Return 409 Conflict (Email/Phone collision) |
| **Dashboard Metrics** | Admin loads `admin.html` | Aggregation queries across 5 tables | KPI metrics computed & visualized | Return 500 (SQL query aggregation error) |
