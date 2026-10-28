# 📊 HostelOS — Data Flow Diagrams (DFD)

[![Mermaid](https://img.shields.io/badge/Mermaid-Rendered-blue?logo=mermaid)](https://mermaid.js.org/)
[![C++17](https://img.shields.io/badge/Backend-C%2B%2B17-00599C?logo=c%2B%2B)](Backend/main.cpp)
[![MySQL](https://img.shields.io/badge/Database-MySQL%208.0-4479A1?logo=mysql)](Database/database_schema.sql)

This document provides complete, multi-tiered **Data Flow Diagrams (DFD)** for the **HostelOS (Hostel Management System)** project. It documents the paths data takes through the system, transformation processes, and interactions with external actors and relational data stores.

---

## 📌 Table of Contents

- [1. DFD Architecture & Notation Standards](#1-dfd-architecture--notation-standards)
- [2. Level 0 DFD — Context Diagram](#2-level-0-dfd--context-diagram)
- [3. Level 1 DFD — Subsystem Decomposition](#3-level-1-dfd--subsystem-decomposition)
- [4. Level 2 DFD — Sub-Process Decompositions](#4-level-2-dfd--sub-process-decompositions)
  - [4.1 Sub-Process 3.0: Room Allocation & Deallocation Engine](#41-sub-process-30-room-allocation--deallocation-engine)
  - [4.2 Sub-Process 4.0: Grievance Lodging & Resolution Lifecycle](#42-sub-process-40-grievance-lodging--resolution-lifecycle)
  - [4.3 Sub-Process 2.0: Student Registration & Profile Management](#43-sub-process-20-student-registration--profile-management)
- [5. System Data Dictionary & Data Store Specifications](#5-system-data-dictionary--data-store-specifications)
- [6. REST API Data Payload Mapping](#6-rest-api-data-payload-mapping)

---

## 1. DFD Architecture & Notation Standards

The diagrams follow standard **Gane-Sarson / Yourdon** DFD conventions represented via Mermaid:

| Symbol / Element | Meaning | Representation in HostelOS |
| :--- | :--- | :--- |
| **External Entity** | Source or destination of data outside the system boundary | `Student`, `Administrator / Hostel Warden` |
| **Process** | Activity or operational module transforming data | `1.0 Authentication`, `3.0 Room Allocation`, etc. |
| **Data Store** | Persistent repository where records are stored | `D1: admins`, `D2: students`, `D3: rooms`, `D4: allocations`, `D5: complaints` |
| **Data Flow** | Directed movement of data packets/objects | Arrows with descriptive nouns/data packets |

---

## 2. Level 0 DFD — Context Diagram

The **Context Diagram** establishes the boundary of the HostelOS system, identifying external actors and high-level input/output data flows.

```mermaid
graph TD
    %% External Entities
    E_Student["👤 External Entity:<br/><b>Hostel Student / Resident</b>"]
    E_Admin["🛡️ External Entity:<br/><b>Admin / Hostel Warden</b>"]

    %% Central Process
    P_HostelOS(("<b>0.0</b><br/><b>Hostel Management System</b><br/>(HostelOS Enterprise)"))

    %% Data Stores (Conceptual persistence boundary)
    DS_MySQL[("🗄️ Relational Persistence Tier<br/><b>MySQL 8.0 Database</b><br/>(hostel_management)")]

    %% Flows from Student
    E_Student -->|"Login Credentials (ID / Email, Password)"| P_HostelOS
    E_Student -->|"Grievance Details (Category, Description)"| P_HostelOS
    E_Student -->|"Profile & Room Status Inquiries"| P_HostelOS

    %% Flows to Student
    P_HostelOS -->|"Authentication Token & Session State"| E_Student
    P_HostelOS -->|"Resident Profile & Assigned Room Info"| E_Student
    P_HostelOS -->|"Grievance Status Updates (Pending/In Progress/Resolved)"| E_Student

    %% Flows from Admin
    E_Admin -->|"Admin Credentials (Username, Password)"| P_HostelOS
    E_Admin -->|"Room Allocation Request (student_id, room_id)"| P_HostelOS
    E_Admin -->|"Checkout / Deallocation Trigger (allocation_id)"| P_HostelOS
    E_Admin -->|"New Student Registration Data"| P_HostelOS
    E_Admin -->|"Complaint Status Progression Update"| P_HostelOS
    E_Admin -->|"Dashboard KPI & Roster Requests"| P_HostelOS

    %% Flows to Admin
    P_HostelOS -->|"Admin Access Session & Authorization"| E_Admin
    P_HostelOS -->|"Real-Time KPI Metrics (Occupancy, Vacancies, Counts)"| E_Admin
    P_HostelOS -->|"Live Room Inventory & Allocation Statuses"| E_Admin
    P_HostelOS -->|"Consolidated Grievance Roster"| E_Admin
    P_HostelOS -->|"Student Directory & Next Available ID"| E_Admin

    %% Central Process <-> Database Flows
    P_HostelOS <-->|"Normalized SQL Queries & Relational Datasets<br/>(Students, Rooms, Allocations, Complaints, Admins)"| DS_MySQL
```

---

## 3. Level 1 DFD — Subsystem Decomposition

The **Level 1 DFD** decomposes HostelOS into five core operational processes, mapping data flows directly to the corresponding database stores.

```mermaid
graph TB
    %% External Entities
    Student["👤 Student / Resident"]
    Admin["🛡️ Admin / Warden"]

    %% Data Stores
    D1[("<b>D1</b> admins")]
    D2[("<b>D2</b> students")]
    D3[("<b>D3</b> rooms")]
    D4[("<b>D4</b> allocations")]
    D5[("<b>D5</b> complaints")]

    %% Processes
    P1(("<b>1.0</b><br/>Authentication<br/>& Session Mgmt"))
    P2(("<b>2.0</b><br/>Student Profile<br/>& Directory Mgmt"))
    P3(("<b>3.0</b><br/>Room Inventory<br/>& Allocation"))
    P4(("<b>4.0</b><br/>Grievance Lodging<br/>& Status Triage"))
    P5(("<b>5.0</b><br/>Dashboard Metrics<br/>& Analytics Aggregator"))

    %% ---------------- PROCESS 1.0 (Auth) ----------------
    Student -->|"Credentials (ID/Email, Pwd)"| P1
    Admin -->|"Admin Credentials (Username, Pwd)"| P1
    P1 -->|"Query admin record"| D1
    D1 -->|"Admin credential match"| P1
    P1 -->|"Query student record"| D2
    D2 -->|"Student credential match"| P1
    P1 -->|"Auth Response {role: 'admin'}"| Admin
    P1 -->|"Auth Response {role: 'student', id}"| Student

    %% ---------------- PROCESS 2.0 (Students) ----------------
    Admin -->|"New Student Payload (Name, Dept, Phone...)"| P2
    Admin -->|"Request Next ID"| P2
    Student -->|"Request Profile (student_id)"| P2
    P2 -->|"Insert / Fetch student records"| D2
    D2 -->|"Student profile details"| P2
    P2 -->|"Query assigned room info"| D4
    D4 -->|"Active allocation data"| P2
    P2 -->|"Student Profile & Room Details"| Student
    P2 -->|"Student Directory Roster & Next ID"| Admin

    %% ---------------- PROCESS 3.0 (Allocations & Rooms) ----------------
    Admin -->|"Allocation Request (student_id, room_id)"| P3
    Admin -->|"Deallocation Trigger (allocation_id)"| P3
    Admin -->|"Inspect Room Inventory"| P3
    P3 -->|"Check active student allocation"| D4
    P3 -->|"Fetch room capacity & current occupancy"| D3
    D3 -->|"Room capacity data"| P3
    P3 -->|"Insert new allocation / update end_date"| D4
    D4 -->|"Allocation records & history"| P3
    P3 -->|"Room Inventory Matrix & Allocation Responses"| Admin

    %% ---------------- PROCESS 4.0 (Complaints) ----------------
    Student -->|"Complaint Submission (Desc, Category)"| P4
    Student -->|"Request Personal Grievance History"| P4
    Admin -->|"Update Ticket Status (In Progress/Resolved)"| P4
    Admin -->|"Fetch All Hostel Grievances"| P4
    P4 -->|"Insert complaint / update status"| D5
    D5 -->|"Complaint records & statuses"| P4
    P4 -->|"Student Grievance Timeline"| Student
    P4 -->|"Grievance Triage Board & Update Status"| Admin

    %% ---------------- PROCESS 5.0 (Analytics) ----------------
    Admin -->|"Fetch Dashboard Metrics Request"| P5
    P5 -->|"Count total students"| D2
    D2 -->|"Total student count"| P5
    P5 -->|"Count rooms & total capacity"| D3
    D3 -->|"Rooms metadata"| P5
    P5 -->|"Count active resident allocations"| D4
    D4 -->|"Occupancy records"| P5
    P5 -->|"Count pending & active complaints"| D5
    D5 -->|"Pending complaint count"| P5
    P5 -->|"KPI Data Payload (Occupancy %, Vacancies, Counts)"| Admin
```

---

## 4. Level 2 DFD — Sub-Process Decompositions

### 4.1 Sub-Process 3.0: Room Allocation & Deallocation Engine

Deconstructs the room allocation transaction into validation, capacity checking, record persistence, and deallocation sub-processes.

```mermaid
graph TD
    Admin["🛡️ Admin / Warden"]

    %% Sub-processes
    P31(("<b>3.1</b><br/>Verify Student<br/>Eligibility"))
    P32(("<b>3.2</b><br/>Verify Room<br/>Capacity"))
    P33(("<b>3.3</b><br/>Persist Room<br/>Allocation"))
    P34(("<b>3.4</b><br/>Terminate Allocation<br/>(Checkout)"))
    P35(("<b>3.5</b><br/>Compute Room<br/>Occupancy Matrix"))

    %% Data Stores
    D3[("<b>D3</b> rooms")]
    D4[("<b>D4</b> allocations")]
    D2[("<b>D2</b> students")]

    %% Flows for Allocation
    Admin -->|"Allocation Request (student_id, room_id)"| P31
    P31 -->|"Check existing active allocation"| D4
    D4 -->|"Active allocation check result"| P31

    P31 -->|"Student verified eligible (student_id, room_id)"| P32
    P32 -->|"Fetch room capacity"| D3
    D3 -->|"Capacity limit"| P32
    P32 -->|"Count active allocations for room_id"| D4
    D4 -->|"Current resident count"| P32

    P32 -->|"Room vacancy confirmed"| P33
    P33 -->|"Write new record: {student_id, room_id, allocation_date}"| D4
    P33 -->|"Allocation Success Response (201 Created)"| Admin

    %% Flows for Deallocation
    Admin -->|"Checkout Trigger (allocation_id)"| P34
    P34 -->|"Fetch allocation record"| D4
    D4 -->|"Record state"| P34
    P34 -->|"Update end_date = CURRENT_DATE()"| D4
    P34 -->|"Deallocation Confirmation (200 OK)"| Admin

    %% Flows for Room Matrix Query
    Admin -->|"Fetch Room Roster"| P35
    P35 -->|"Read all rooms"| D3
    P35 -->|"Read active allocations"| D4
    P35 -->|"Room Matrix with Occupancy Status"| Admin
```

---

### 4.2 Sub-Process 4.0: Grievance Lodging & Resolution Lifecycle

Traces the data transformations from initial grievance logging to triage and resolution.

```mermaid
graph TD
    Student["👤 Student / Resident"]
    Admin["🛡️ Admin / Warden"]

    %% Sub-processes
    P41(("<b>4.1</b><br/>Validate & Format<br/>Grievance Input"))
    P42(("<b>4.2</b><br/>Persist New<br/>Complaint"))
    P43(("<b>4.3</b><br/>Retrieve Resident<br/>Grievance History"))
    P44(("<b>4.4</b><br/>Aggregate All<br/>Hostel Complaints"))
    P45(("<b>4.5</b><br/>Execute Status<br/>Transition"))

    %% Data Store
    D5[("<b>D5</b> complaints")]
    D2[("<b>D2</b> students")]

    %% Submission flow
    Student -->|"Complaint Submission {student_id, description}"| P41
    P41 -->|"Sanitized description & student_id"| P42
    P42 -->|"INSERT INTO complaints (student_id, desc, 'Pending', NOW())"| D5
    P42 -->|"Ticket Confirmation (201 Created)"| Student

    %% Student Read flow
    Student -->|"Request My Complaints (student_id)"| P43
    P43 -->|"Query complaints WHERE student_id = :id"| D5
    D5 -->|"Complaint records list"| P43
    P43 -->|"Formatted Grievance List with Status Badges"| Student

    %% Admin Triage flow
    Admin -->|"Request All Complaints Roster"| P44
    P44 -->|"JOIN complaints WITH students"| D5
    D5 -->|"All active & resolved complaints"| P44
    P44 -->|"Fetch student names & contact details"| D2
    D2 -->|"Student metadata"| P44
    P44 -->|"Consolidated Triage Board Data"| Admin

    Admin -->|"Status Update Request {complaint_id, new_status}"| P45
    P45 -->|"UPDATE complaints SET status = :new_status WHERE id = :id"| D5
    P45 -->|"Status Update Acknowledgment (200 OK)"| Admin
```

---

### 4.3 Sub-Process 2.0: Student Registration & Profile Management

Details the data transformations during student onboarding and profile visualization.

```mermaid
graph TD
    Admin["🛡️ Admin / Warden"]
    Student["👤 Student / Resident"]

    %% Sub-processes
    P21(("<b>2.1</b><br/>Generate Next<br/>Student ID"))
    P22(("<b>2.2</b><br/>Validate & Sanitize<br/>Registration Data"))
    P23(("<b>2.3</b><br/>Persist Student<br/>Entity"))
    P24(("<b>2.4</b><br/>Assemble Student<br/>Full Profile"))
    P25(("<b>2.5</b><br/>Generate Student<br/>Directory Roster"))

    %% Data Stores
    D2[("<b>D2</b> students")]
    D3[("<b>D3</b> rooms")]
    D4[("<b>D4</b> allocations")]

    %% Registration Flows
    Admin -->|"Request Next Available Student ID"| P21
    P21 -->|"SELECT MAX(student_id) FROM students"| D2
    D2 -->|"Max ID result"| P21
    P21 -->|"Computed Next ID (e.g., 106)"| Admin

    Admin -->|"Submit Form {id, name, email, dept, phone, pwd}"| P22
    P22 -->|"Check email/phone uniqueness"| D2
    D2 -->|"Collision check result"| P22
    P22 -->|"Validated Student Object"| P23
    P23 -->|"INSERT INTO students"| D2
    P23 -->|"Registration Confirmation (201 Created)"| Admin

    %% Profile Assembly Flow
    Student -->|"Request Profile Details (student_id)"| P24
    P24 -->|"Fetch student entity details"| D2
    D2 -->|"Name, department, contact"| P24
    P24 -->|"Query active allocation for student"| D4
    D4 -->|"allocation_id, room_id"| P24
    P24 -->|"Query room number, block, floor"| D3
    D3 -->|"Room metadata"| P24
    P24 -->|"Comprehensive Profile JSON Package"| Student

    %% Roster Flow
    Admin -->|"Request Complete Student Directory"| P25
    P25 -->|"SELECT * FROM students ORDER BY student_id"| D2
    D2 -->|"Student records list"| P25
    P25 -->|"Searchable Student Roster Table"| Admin
```

---

## 5. System Data Dictionary & Data Store Specifications

The five primary data stores defined in `Database/database_schema.sql` handle the following structured entity flows:

### Data Store D1: `admins`
Stores administrative authentication credentials and access control parameters.
- **Attributes:**
  - `admin_id` (INT, Primary Key, Auto-increment)
  - `username` (VARCHAR(50), Unique, Not Null)
  - `password` (VARCHAR(255), Not Null)
- **Inflows:** Initial database seed (`Database/sample_data.sql`).
- **Outflows:** Credential verification in `Process 1.0 (LoginAPI)`.

### Data Store D2: `students`
Stores resident demographic, academic, and authentication data.
- **Attributes:**
  - `student_id` (INT, Primary Key)
  - `name` (VARCHAR(100), Not Null)
  - `email` (VARCHAR(100), Unique, Not Null)
  - `department` (VARCHAR(50))
  - `phone` (VARCHAR(15))
  - `password` (VARCHAR(255), Not Null)
- **Inflows:** Student onboarding from `Process 2.0 (StudentAPI::handleCreateStudent)`.
- **Outflows:** Authentication in `Process 1.0`, profile queries in `Process 2.0`, KPI metrics in `Process 5.0`.

### Data Store D3: `rooms`
Maintains hostel infrastructure inventory, floor plans, and capacity allocations.
- **Attributes:**
  - `room_id` (INT, Primary Key, Auto-increment)
  - `room_number` (VARCHAR(10), Unique, Not Null)
  - `block` (VARCHAR(10), Not Null)
  - `floor` (INT, Not Null)
  - `capacity` (INT, Not Null, Default 2)
- **Inflows:** Facility configuration scripts.
- **Outflows:** Capacity verification in `Process 3.0 (AllocationAPI)`, room matrix visualization in `Process 3.0 (RoomAPI)`, facility KPI calculations in `Process 5.0 (DashboardAPI)`.

### Data Store D4: `allocations`
Captures historical and active relationships between students and rooms.
- **Attributes:**
  - `allocation_id` (INT, Primary Key, Auto-increment)
  - `student_id` (INT, Foreign Key references `students(student_id)`)
  - `room_id` (INT, Foreign Key references `rooms(room_id)`)
  - `allocation_date` (DATE, Not Null)
  - `end_date` (DATE, Nullable — `NULL` indicates active residency)
- **Inflows:** New room assignment in `Process 3.0 (AllocationAPI::handleCreateAllocation)`.
- **Outflows:** Occupancy validation in `Process 3.0`, active room lookup in `Process 2.0`, checkout updates in `Process 3.0 (AllocationAPI::handleDeallocate)`.

### Data Store D5: `complaints`
Records facility maintenance issues, student grievances, and resolution progress.
- **Attributes:**
  - `complaint_id` (INT, Primary Key, Auto-increment)
  - `student_id` (INT, Foreign Key references `students(student_id)`)
  - `description` (TEXT, Not Null)
  - `status` (VARCHAR(20), Default `'Pending'`; Enum: `'Pending'`, `'In Progress'`, `'Resolved'`)
  - `created_at` (TIMESTAMP, Default `CURRENT_TIMESTAMP`)
- **Inflows:** Grievance submissions from `Process 4.0 (ComplaintAPI::handleCreateComplaint)`.
- **Outflows:** Status triage updates from `Process 4.0 (ComplaintAPI::handleUpdateComplaintStatus)`, grievance timelines in `Process 4.0`, pending complaint counts in `Process 5.0`.

---

## 6. REST API Data Payload Mapping

| Process ID | REST Method & Endpoint | Source / Producer | Inbound Data Payload | Outbound Data Payload | Destination Store |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1.0** | `POST /api/login` | Client Portal | `{"userId":"101", "password":"..."}` | `{"success":true, "role":"student", "student_id":"101"}` | `D1`, `D2` |
| **2.1** | `GET /api/students/next-id` | Admin Portal | None | `{"success":true, "next_id":106}` | `D2` |
| **2.2** | `POST /api/students` | Admin Portal | `{"name":"...", "email":"...", "department":"...", "phone":"...", "password":"..."}` | `{"success":true, "message":"Created", "student_id":106}` | `D2` |
| **2.4** | `GET /api/students/:id` | Student Portal | URL parameter `:id` | `{"success":true, "student":{..., "room_number":"A-101", "block":"A"}}` | `D2`, `D3`, `D4` |
| **3.2** | `GET /api/rooms` | Admin Portal | None | `{"success":true, "rooms":[{"room_number":"A-101", "capacity":2, "occupancy":1, "status":"Available"}]}` | `D3`, `D4` |
| **3.3** | `POST /api/allocations` | Admin Portal | `{"student_id":101, "room_id":1}` | `{"success":true, "allocation_id":4}` | `D3`, `D4` |
| **3.4** | `PUT /api/allocations/:id/deallocate`| Admin Portal | URL parameter `:id` | `{"success":true, "message":"Deallocated successfully"}` | `D4` |
| **4.2** | `POST /api/complaints` | Student Portal | `{"student_id":101, "description":"Fan not working"}` | `{"success":true, "complaint_id":7}` | `D5` |
| **4.3** | `GET /api/complaints/:studentId` | Student Portal | URL parameter `:studentId` | `{"success":true, "complaints":[{"id":7, "description":"...", "status":"Pending"}]}` | `D5` |
| **4.5** | `PUT /api/complaints/:id` | Admin Portal | `{"status":"In Progress"}` | `{"success":true, "message":"Updated"}` | `D5` |
| **5.0** | `GET /api/admin/dashboard` | Admin Portal | None | `{"success":true, "total_students":5, "total_rooms":6, "occupancy_rate":50.0, "vacancies":6, "pending_complaints":2}` | `D2`, `D3`, `D4`, `D5` |
