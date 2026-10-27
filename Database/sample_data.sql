-- ============================================================
-- HOSTEL MANAGEMENT SYSTEM — SAMPLE DATA SEED SCRIPT
-- Database: hostel_management
-- ============================================================

USE hostel_management;

-- Disable foreign key checks for clean truncation and insertion
SET FOREIGN_KEY_CHECKS = 0;
TRUNCATE TABLE complaints;
TRUNCATE TABLE allocations;
TRUNCATE TABLE rooms;
TRUNCATE TABLE students;
TRUNCATE TABLE admins;
SET FOREIGN_KEY_CHECKS = 1;

-- ------------------------------------------------------------
-- 1. Insert Admins
-- ------------------------------------------------------------
INSERT INTO admins (admin_id, username, password) VALUES
(1, 'admin', 'admin123'),
(2, 'warden_blockA', 'warden123'),
(3, 'chief_warden', 'hostel2026');

-- ------------------------------------------------------------
-- 2. Insert Students
-- ------------------------------------------------------------
INSERT INTO students (student_id, name, email, department, phone, password) VALUES
(101, 'Karthik Raj', 'karthik.student@example.com', 'Computer Science', '9876543210', 'student123'),
(102, 'Priya Sharma', 'priya.sharma@example.com', 'Electronics & Comm', '9876543211', 'student123'),
(103, 'Aarav Patel', 'aarav.patel@example.com', 'Mechanical Engineering', '9876543212', 'student123'),
(104, 'Ananya Iyer', 'ananya.iyer@example.com', 'Information Technology', '9876543213', 'student123'),
(105, 'Rohan Verma', 'rohan.verma@example.com', 'Civil Engineering', '9876543214', 'student123'),
(106, 'Sneha Reddy', 'sneha.reddy@example.com', 'Electrical Engineering', '9876543215', 'student123'),
(107, 'Vikram Malhotra', 'vikram.m@example.com', 'Computer Science', '9876543216', 'student123'),
(108, 'Neha Joshi', 'neha.joshi@example.com', 'Biotechnology', '9876543217', 'student123');

-- ------------------------------------------------------------
-- 3. Insert Rooms
-- ------------------------------------------------------------
INSERT INTO rooms (room_id, room_number, block, floor, capacity) VALUES
(1, '101', 'Block A', 1, 2),
(2, '102', 'Block A', 1, 2),
(3, '103', 'Block A', 1, 3),
(4, '201', 'Block B', 2, 2),
(5, '202', 'Block B', 2, 3),
(6, '203', 'Block B', 2, 2),
(7, '301', 'Block C', 3, 4),
(8, '302', 'Block C', 3, 2);

-- ------------------------------------------------------------
-- 4. Insert Allocations (Active and Completed Past Allocations)
-- ------------------------------------------------------------
-- Active Allocations (end_date IS NULL)
INSERT INTO allocations (allocation_id, student_id, room_id, allocation_date, end_date) VALUES
(1, 101, 1, '2026-08-01', NULL),
(2, 102, 1, '2026-08-01', NULL),
(3, 103, 2, '2026-08-05', NULL),
(4, 104, 3, '2026-08-10', NULL),
(5, 105, 4, '2026-08-12', NULL),
-- Historical/Completed Allocations (end_date IS NOT NULL)
(6, 106, 1, '2025-08-01', '2026-05-30'),
(7, 107, 2, '2025-08-01', '2026-05-30'),
(8, 108, 3, '2025-08-01', '2026-06-15');

-- ------------------------------------------------------------
-- 5. Insert Student Complaints / Grievances
-- ------------------------------------------------------------
INSERT INTO complaints (complaint_id, student_id, description, status, created_at) VALUES
(1, 101, 'Plumbing issue: Washroom tap leaking continuously in Room 101.', 'Pending', '2026-10-25 10:30:00'),
(2, 102, 'Electrical failure: Ceiling fan speed regulator is jammed and not functioning.', 'In Progress', '2026-10-24 14:15:00'),
(3, 103, 'Carpentry repair: Study table drawer lock is stuck and cannot be opened.', 'Resolved', '2026-10-20 09:00:00'),
(4, 104, 'Wi-Fi problem: Weak Wi-Fi signal coverage on Block A 1st floor corridor.', 'In Progress', '2026-10-23 16:45:00'),
(5, 105, 'Room cleaning request: Deep cleaning required for balcony area.', 'Pending', '2026-10-26 11:20:00'),
(6, 101, 'Electrical failure: Tube light in study area has been flickering.', 'Resolved', '2026-10-18 17:30:00');

-- ============================================================
-- VERIFICATION SUMMARY
-- ============================================================
SELECT 'Students Count' AS Metric, COUNT(*) AS Total FROM students
UNION ALL
SELECT 'Rooms Count', COUNT(*) FROM rooms
UNION ALL
SELECT 'Active Allocations', COUNT(*) FROM allocations WHERE end_date IS NULL
UNION ALL
SELECT 'Total Complaints', COUNT(*) FROM complaints
UNION ALL
SELECT 'Admins Count', COUNT(*) FROM admins;
