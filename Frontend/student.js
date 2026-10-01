// ==========================================
// STUDENT.JS
// ==========================================


// Check whether the user is logged in

const userId = sessionStorage.getItem("userId");
const role = sessionStorage.getItem("role");

if (!userId || role !== "student") {
    window.location.href = "index.html";
}


// ==========================================
// LOAD STUDENT DATA
// ==========================================

async function loadStudentData() {

    try {

        /*
         * C++ BACKEND:
         *
         * GET:
         * /api/students/STU001
         *
         * Expected response:
         *
         * {
         *   "studentId": "STU001",
         *   "name": "Rahul",
         *   "rollNumber": "23CS101",
         *   "roomNumber": "A-101",
         *   "complaintsRegistered": 4,
         *   "complaintsResolved": 2
         * }
         */

        const response =
            await fetch(`http://localhost:8080/api/students/${userId}`);

        const student = await response.json();

        displayStudent(student);

        loadComplaints();

    }
    catch (error) {

        /*
         * DEMO DATA
         *
         * Remove when C++ backend is connected.
         */

        const student = {
            studentId: "STU001",
            name: "Rahul Kumar",
            rollNumber: "23CSE101",
            roomNumber: "A-101",
            complaintsRegistered: 3,
            complaintsResolved: 2
        };

        displayStudent(student);

        loadDemoComplaints();
    }
}


// ==========================================
// DISPLAY STUDENT
// ==========================================

function displayStudent(student) {

    document.getElementById("studentName").textContent =
        student.name;

    document.getElementById("studentNameCard").textContent =
        student.name;

    document.getElementById("rollNumber").textContent =
        student.rollNumber;

    document.getElementById("roomNumber").textContent =
        student.roomNumber;

    document.getElementById("complaintsRegistered").textContent =
        student.complaintsRegistered;

    document.getElementById("complaintsResolved").textContent =
        student.complaintsResolved;
}


// ==========================================
// SUBMIT COMPLAINT
// ==========================================

document.getElementById("complaintForm")
    .addEventListener("submit", async function(event) {

        event.preventDefault();

        const complaintText =
            document.getElementById("complaintText").value.trim();

        const message =
            document.getElementById("complaintMessage");

        if (complaintText === "") {
            return;
        }


        /*
         * C++ BACKEND
         *
         * POST /api/complaints
         *
         * Data:
         *
         * {
         *    "studentId": "STU001",
         *    "complaint": "Fan is not working"
         * }
         */

        try {

            const response = await fetch(
                "http://localhost:8080/api/complaints",
                {
                    method: "POST",

                    headers: {
                        "Content-Type": "application/json"
                    },

                    body: JSON.stringify({
                        studentId: userId,
                        complaint: complaintText
                    })
                }
            );

            const data = await response.json();

            if (data.success) {

                message.textContent =
                    "Complaint registered successfully.";

                message.style.color = "green";

                document.getElementById("complaintText").value = "";

                loadStudentData();

            }

        }
        catch (error) {

            /*
             * DEMO MODE
             */

            const complaints =
                JSON.parse(
                    localStorage.getItem("complaints")
                ) || [];

            complaints.push({

                complaintId:
                    "C" + (complaints.length + 1),

                studentId: userId,

                complaint: complaintText,

                date: new Date().toLocaleDateString(),

                status: "Not Resolved"

            });

            localStorage.setItem(
                "complaints",
                JSON.stringify(complaints)
            );

            message.textContent =
                "Complaint registered successfully.";

            message.style.color = "green";

            document.getElementById("complaintText").value = "";

            loadDemoComplaints();
        }

    });


// ==========================================
// LOAD COMPLAINTS
// ==========================================

async function loadComplaints() {

    try {

        const response =
            await fetch(
                `http://localhost:8080/api/complaints/student/${userId}`
            );

        const complaints =
            await response.json();

        displayComplaints(complaints);

    }
    catch (error) {

        loadDemoComplaints();

    }
}


// ==========================================
// DEMO COMPLAINTS
// ==========================================

function loadDemoComplaints() {

    const stored =
        JSON.parse(
            localStorage.getItem("complaints")
        );

    if (stored) {

        displayComplaints(
            stored.filter(
                complaint =>
                    complaint.studentId === userId
            )
        );

        return;
    }


    const complaints = [

        {
            complaintId: "C1",
            studentId: "STU001",
            complaint: "Fan is not working",
            date: "01/10/2026",
            status: "Resolved"
        },

        {
            complaintId: "C2",
            studentId: "STU001",
            complaint: "Water leakage in bathroom",
            date: "28/09/2026",
            status: "Not Resolved"
        }

    ];

    displayComplaints(complaints);
}


// ==========================================
// DISPLAY COMPLAINTS
// ==========================================

function displayComplaints(complaints) {

    const tbody =
        document.getElementById(
            "studentComplaintsBody"
        );

    tbody.innerHTML = "";

    complaints.forEach(complaint => {

        const row = document.createElement("tr");

        const statusClass =
            complaint.status === "Resolved"
                ? "status-resolved"
                : "status-unresolved";

        row.innerHTML = `

            <td>${complaint.complaint}</td>

            <td>${complaint.date}</td>

            <td class="${statusClass}">
                ${complaint.status}
            </td>

        `;

        tbody.appendChild(row);

    });
}


// ==========================================
// LOGOUT
// ==========================================

function logout() {

    sessionStorage.clear();

    window.location.href = "index.html";
}


// ==========================================
// INITIAL LOAD
// ==========================================

loadStudentData();