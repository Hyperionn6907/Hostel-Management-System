// ==========================================
// ADMIN.JS
// ==========================================


// ==========================================
// LOGIN CHECK
// ==========================================

const userId = sessionStorage.getItem("userId");
const role = sessionStorage.getItem("role");

if (!userId || role !== "admin") {

    window.location.href = "index.html";

}


// ==========================================
// SECTION NAVIGATION
// ==========================================

function showSection(sectionId, button) {

    const sections =
        document.querySelectorAll(".content-section");

    sections.forEach(section => {
        section.classList.add("hidden");
    });


    document
        .getElementById(sectionId)
        .classList.remove("hidden");


    const buttons =
        document.querySelectorAll(".nav-btn");

    buttons.forEach(btn => {
        btn.classList.remove("active");
    });


    button.classList.add("active");


    const titles = {

        dashboard: "Dashboard",

        students: "Students",

        rooms: "Rooms",

        allocations: "Allocations",

        complaints: "Complaints"

    };

    document.getElementById("pageTitle").textContent =
        titles[sectionId];


    if (sectionId === "dashboard") {
        loadDashboard();
    }

    if (sectionId === "students") {
        loadStudents();
    }

    if (sectionId === "rooms") {
        loadRooms();
    }

    if (sectionId === "allocations") {
        loadAllocations();
    }

    if (sectionId === "complaints") {
        loadComplaints();
    }

}


// ==========================================
// DASHBOARD
// ==========================================

async function loadDashboard() {

    try {

        /*
         * C++ BACKEND
         *
         * GET /api/dashboard
         *
         * Example response:
         *
         * {
         *   "studentCount": 150,
         *   "roomCount": 60,
         *   "occupiedRooms": 48,
         *   "unresolvedComplaints": 7
         * }
         */

        const response =
            await fetch(
                "http://localhost:8080/api/dashboard"
            );

        const data =
            await response.json();

        updateDashboard(data);

    }
    catch (error) {

        // DEMO DATA

        updateDashboard({

            studentCount: 120,

            roomCount: 50,

            occupiedRooms: 42,

            unresolvedComplaints: 8

        });

    }

}


// ==========================================
// UPDATE DASHBOARD
// ==========================================

function updateDashboard(data) {

    document.getElementById("studentCount")
        .textContent = data.studentCount;

    document.getElementById("roomCount")
        .textContent = data.roomCount;

    document.getElementById("occupiedRoomCount")
        .textContent = data.occupiedRooms;

    document.getElementById("unresolvedCount")
        .textContent = data.unresolvedComplaints;

}


// ==========================================
// STUDENTS
// ==========================================

async function loadStudents() {

    try {

        const response =
            await fetch(
                "http://localhost:8080/api/students"
            );

        const students =
            await response.json();

        displayStudents(students);

    }
    catch (error) {

        displayStudents([

            {
                studentId: "STU001",
                name: "Rahul Kumar",
                rollNumber: "23CSE101",
                department: "CSE",
                phone: "9876543210",
                room: "A-101"
            },

            {
                studentId: "STU002",
                name: "Arun Kumar",
                rollNumber: "23CSE102",
                department: "CSE",
                phone: "9876543211",
                room: "A-102"
            },

            {
                studentId: "STU003",
                name: "Priya Sharma",
                rollNumber: "23ECE103",
                department: "ECE",
                phone: "9876543212",
                room: "B-201"
            }

        ]);

    }

}


// ==========================================
// DISPLAY STUDENTS
// ==========================================

function displayStudents(students) {

    const tbody =
        document.getElementById(
            "studentsTableBody"
        );

    tbody.innerHTML = "";

    students.forEach(student => {

        const row =
            document.createElement("tr");

        row.innerHTML = `

            <td>${student.studentId}</td>

            <td>${student.name}</td>

            <td>${student.rollNumber}</td>

            <td>${student.department}</td>

            <td>${student.phone}</td>

            <td>${student.room}</td>

        `;

        tbody.appendChild(row);

    });

}


// ==========================================
// ADD STUDENT MODAL
// ==========================================

function openStudentModal() {

    document
        .getElementById("studentModal")
        .classList.remove("hidden");

}


function closeStudentModal() {

    document
        .getElementById("studentModal")
        .classList.add("hidden");

}


// ==========================================
// ADD STUDENT
// ==========================================

document
    .getElementById("addStudentForm")
    .addEventListener("submit", async function(event) {

        event.preventDefault();


        const student = {

            studentId:
                document.getElementById(
                    "newStudentId"
                ).value,

            name:
                document.getElementById(
                    "newStudentName"
                ).value,

            rollNumber:
                document.getElementById(
                    "newRollNumber"
                ).value,

            email:
                document.getElementById(
                    "newStudentEmail"
                ).value,

            department:
                document.getElementById(
                    "newDepartment"
                ).value,

            phone:
                document.getElementById(
                    "newPhone"
                ).value,

            room:
                document.getElementById(
                    "newRoom"
                ).value,

            password:
                document.getElementById(
                    "newPassword"
                ).value

        };


        /*
         * C++ OOP BACKEND
         *
         * POST /api/students
         *
         * The C++ backend should create a Student object
         * and store it in the database.
         */

        try {

            const response =
                await fetch(
                    "http://localhost:8080/api/students",
                    {
                        method: "POST",

                        headers: {
                            "Content-Type":
                                "application/json"
                        },

                        body: JSON.stringify(student)
                    }
                );


            const data =
                await response.json();


            if (data.success) {

                alert(
                    "Student added successfully."
                );

                closeStudentModal();

                loadStudents();

                loadDashboard();

            }

        }
        catch (error) {

            /*
             * DEMO MODE
             */

            alert(
                "Student added successfully (Demo Mode)."
            );

            closeStudentModal();

            document
                .getElementById("addStudentForm")
                .reset();

            loadStudents();

        }

    });


// ==========================================
// ROOMS
// ==========================================

async function loadRooms() {

    try {

        const response =
            await fetch(
                "http://localhost:8080/api/rooms"
            );

        const rooms =
            await response.json();

        displayRooms(rooms);

    }
    catch (error) {

        displayRooms([

            {
                roomNumber: "A-101",

                students: [

                    {
                        rollNumber: "23CSE101",
                        department: "CSE"
                    },

                    {
                        rollNumber: "23CSE105",
                        department: "CSE"
                    }

                ]

            },

            {
                roomNumber: "A-102",

                students: [

                    {
                        rollNumber: "23ECE101",
                        department: "ECE"
                    }

                ]

            },

            {
                roomNumber: "B-201",

                students: []

            }

        ]);

    }

}


// ==========================================
// DISPLAY ROOMS
// ==========================================

function displayRooms(rooms) {

    const container =
        document.getElementById(
            "roomsContainer"
        );

    container.innerHTML = "";


    rooms.forEach(room => {

        const card =
            document.createElement("div");

        card.className = "room-card";


        let studentList = "";

        if (room.students.length === 0) {

            studentList =
                "<p>No students allocated</p>";

        }
        else {

            studentList = "<ul>";

            room.students.forEach(student => {

                studentList += `

                    <li>
                        ${student.rollNumber}
                        - ${student.department}
                    </li>

                `;

            });

            studentList += "</ul>";

        }


        card.innerHTML = `

            <h3>
                Room ${room.roomNumber}
            </h3>

            <p>
                Students:
                ${room.students.length}
            </p>

            ${studentList}

        `;


        container.appendChild(card);

    });

}


// ==========================================
// ALLOCATIONS
// ==========================================

async function loadAllocations() {

    try {

        const response =
            await fetch(
                "http://localhost:8080/api/allocations"
            );

        const allocations =
            await response.json();

        displayAllocations(allocations);

    }
    catch (error) {

        displayAllocations([

            {
                studentId: "STU001",
                name: "Rahul Kumar",
                room: "A-101",
                date: "15/08/2026"
            },

            {
                studentId: "STU002",
                name: "Arun Kumar",
                room: "A-102",
                date: "16/08/2026"
            }

        ]);

    }

}


// ==========================================
// DISPLAY ALLOCATIONS
// ==========================================

function displayAllocations(allocations) {

    const tbody =
        document.getElementById(
            "allocationTableBody"
        );

    tbody.innerHTML = "";


    allocations.forEach(allocation => {

        const row =
            document.createElement("tr");


        row.innerHTML = `

            <td>
                ${allocation.studentId}
            </td>

            <td>
                ${allocation.name}
            </td>

            <td>
                ${allocation.room}
            </td>

            <td>
                ${allocation.date}
            </td>

            <td>

                <button
                    class="edit-btn"
                    onclick="openAllocationModal(
                        '${allocation.studentId}',
                        '${allocation.room}'
                    )">

                    Edit

                </button>

            </td>

        `;


        tbody.appendChild(row);

    });

}


// ==========================================
// EDIT ALLOCATION
// ==========================================

function openAllocationModal(studentId, room) {

    document.getElementById(
        "editStudentId"
    ).value = studentId;

    document.getElementById(
        "editRoomNumber"
    ).value = room;

    document
        .getElementById("allocationModal")
        .classList.remove("hidden");

}


function closeAllocationModal() {

    document
        .getElementById("allocationModal")
        .classList.add("hidden");

}


// ==========================================
// SAVE ROOM ALLOCATION
// ==========================================

async function saveAllocation() {

    const studentId =
        document.getElementById(
            "editStudentId"
        ).value;

    const room =
        document.getElementById(
            "editRoomNumber"
        ).value;


    /*
     * C++ BACKEND
     *
     * PUT /api/allocations/STU001
     */

    try {

        const response =
            await fetch(
                `http://localhost:8080/api/allocations/${studentId}`,
                {
                    method: "PUT",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body: JSON.stringify({
                        room: room
                    })
                }
            );


        const data =
            await response.json();


        if (data.success) {

            alert(
                "Room allocation updated."
            );

            closeAllocationModal();

            loadAllocations();

            loadRooms();

            loadDashboard();

        }

    }
    catch (error) {

        alert(
            "Room allocation updated (Demo Mode)."
        );

        closeAllocationModal();

        loadAllocations();

    }

}


// ==========================================
// REMOVE STUDENT
// ==========================================

async function removeStudent() {

    const studentId =
        document.getElementById(
            "editStudentId"
        ).value;


    const confirmation =
        confirm(
            "Are you sure you want to remove this student?"
        );


    if (!confirmation) {
        return;
    }


    /*
     * C++ BACKEND
     *
     * DELETE /api/students/STU001
     */

    try {

        const response =
            await fetch(
                `http://localhost:8080/api/students/${studentId}`,
                {
                    method: "DELETE"
                }
            );


        const data =
            await response.json();


        if (data.success) {

            alert(
                "Student removed."
            );

            closeAllocationModal();

            loadStudents();

            loadAllocations();

            loadRooms();

            loadDashboard();

        }

    }
    catch (error) {

        alert(
            "Student removed (Demo Mode)."
        );

        closeAllocationModal();

        loadStudents();

    }

}


// ==========================================
// COMPLAINTS
// ==========================================

async function loadComplaints() {

    try {

        const response =
            await fetch(
                "http://localhost:8080/api/complaints"
            );

        const complaints =
            await response.json();

        displayComplaints(complaints);

    }
    catch (error) {

        displayComplaints([

            {
                complaintId: "C1",
                studentId: "STU001",
                complaint:
                    "Fan is not working",
                date: "01/10/2026",
                status: "Resolved"
            },

            {
                complaintId: "C2",
                studentId: "STU002",
                complaint:
                    "Water leakage",
                date: "30/09/2026",
                status: "Not Resolved"
            }

        ]);

    }

}


// ==========================================
// DISPLAY COMPLAINTS
// ==========================================

function displayComplaints(complaints) {

    const tbody =
        document.getElementById(
            "complaintsTableBody"
        );

    tbody.innerHTML = "";


    complaints.forEach(complaint => {

        const row =
            document.createElement("tr");


        const statusClass =
            complaint.status === "Resolved"
                ? "status-resolved"
                : "status-unresolved";


        const action =
            complaint.status === "Resolved"

                ? "-"

                : `

                    <button
                        class="primary-btn"
                        onclick="resolveComplaint(
                            '${complaint.complaintId}'
                        )">

                        Mark Resolved

                    </button>

                  `;


        row.innerHTML = `

            <td>
                ${complaint.complaintId}
            </td>

            <td>
                ${complaint.studentId}
            </td>

            <td>
                ${complaint.complaint}
            </td>

            <td>
                ${complaint.date}
            </td>

            <td class="${statusClass}">
                ${complaint.status}
            </td>

            <td>
                ${action}
            </td>

        `;


        tbody.appendChild(row);

    });

}


// ==========================================
// RESOLVE COMPLAINT
// ==========================================

async function resolveComplaint(complaintId) {

    /*
     * C++ BACKEND
     *
     * PUT /api/complaints/C1
     *
     * {
     *      "status": "Resolved"
     * }
     */

    try {

        const response =
            await fetch(
                `http://localhost:8080/api/complaints/${complaintId}`,
                {
                    method: "PUT",

                    headers: {
                        "Content-Type":
                            "application/json"
                    },

                    body: JSON.stringify({
                        status: "Resolved"
                    })
                }
            );


        const data =
            await response.json();


        if (data.success) {

            alert(
                "Complaint marked as resolved."
            );

            loadComplaints();

            loadDashboard();

        }

    }
    catch (error) {

        alert(
            "Complaint marked as resolved (Demo Mode)."
        );

        loadComplaints();

    }

}


// ==========================================
// LOGOUT
// ==========================================

function logout() {

    sessionStorage.clear();

    window.location.href = "index.html";

}


// ==========================================
// INITIAL DASHBOARD
// ==========================================

loadDashboard();