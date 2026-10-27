-- ============================================================
-- HOSTEL MANAGEMENT SYSTEM — DATABASE SCHEMA
-- Database: hostel_management
-- ============================================================

CREATE DATABASE IF NOT EXISTS hostel_management;
USE hostel_management;

-- ------------------------------------------------------------
-- Table: students
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS students (
    student_id INT PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    department VARCHAR(50),
    phone VARCHAR(15),
    password VARCHAR(255) NOT NULL DEFAULT 'student123'
);

-- ------------------------------------------------------------
-- Table: rooms
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS rooms (
    room_id INT PRIMARY KEY AUTO_INCREMENT,
    room_number VARCHAR(10) UNIQUE NOT NULL,
    block VARCHAR(20) NOT NULL,
    floor INT NOT NULL,
    capacity INT NOT NULL
);

-- ------------------------------------------------------------
-- Table: allocations
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS allocations (
    allocation_id INT PRIMARY KEY AUTO_INCREMENT,
    student_id INT NOT NULL,
    room_id INT NOT NULL,
    allocation_date DATE NOT NULL,
    end_date DATE NULL,
    FOREIGN KEY (student_id) REFERENCES students(student_id),
    FOREIGN KEY (room_id) REFERENCES rooms(room_id)
);

-- ------------------------------------------------------------
-- Table: complaints
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS complaints (
    complaint_id INT PRIMARY KEY AUTO_INCREMENT,
    student_id INT NOT NULL,
    description VARCHAR(500) NOT NULL,
    status VARCHAR(20) DEFAULT 'Pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (student_id) REFERENCES students(student_id)
);

-- ------------------------------------------------------------
-- Table: admins
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS admins (
    admin_id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL
);

-- ============================================================
-- VERIFICATION & INSPECTION QUERIES
-- ============================================================
SHOW TABLES;
DESCRIBE students;
DESCRIBE rooms;
DESCRIBE allocations;
DESCRIBE complaints;
DESCRIBE admins;
