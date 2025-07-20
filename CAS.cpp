#include <iostream>
#include <iomanip>
#include <vector>
#include <ctime>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#define CLEAR "CLS"
#else
#define CLEAR "clear"
#endif

using namespace std;

struct Attendance {
    string studentName;
    string subject;
    string parentNumber;
    string timeIn;
    string timeOut;
};

struct Student {
    string id;
    string name;
};

vector<Attendance> records;
vector<string> announcements;
vector<Student> studentList;

string getCurrentTime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
    return string(buffer);
}

void clearScreen() {
    system(CLEAR);
}

void pause() {
    cout << "\nPress Enter to return to menu...";
    cin.ignore();
    cin.get();
}

void sendSMS(const string& number, const string& message) {
    cout << "\n--- SMS TO PARENT [" << number << "] ---\n";
    cout << message << "\n";
    cout << "--------------------------------------\n";
}

void saveAttendanceToFile() {
    ofstream file("attendance_records.txt");
    for (const auto& r : records) {
        file << r.studentName << "|" << r.subject << "|" << r.parentNumber << "|"
             << r.timeIn << "|" << r.timeOut << "\n";
    }
    file.close();
}

void loadAttendanceFromFile() {
    ifstream file("attendance_records.txt");
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        Attendance r;
        getline(ss, r.studentName, '|');
        getline(ss, r.subject, '|');
        getline(ss, r.parentNumber, '|');
        getline(ss, r.timeIn, '|');
        getline(ss, r.timeOut, '|');
        records.push_back(r);
    }
    file.close();
}

void saveAnnouncementsToFile() {
    ofstream file("announcements.txt");
    for (const auto& ann : announcements) {
        file << ann << "\n";
    }
    file.close();
}

void loadAnnouncementsFromFile() {
    ifstream file("announcements.txt");
    string line;
    while (getline(file, line)) {
        announcements.push_back(line);
    }
    file.close();
}

string toLower(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

bool isValidNumber(const string& number) {
    return all_of(number.begin(), number.end(), ::isdigit) && number.length() >= 10;
}

void loadStudentsFromFile() {
    ifstream file("student_masterlist.txt");
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        Student s;
        getline(ss, s.id, '|');
        getline(ss, s.name, '|');
        studentList.push_back(s);
    }
    file.close();
}

void saveStudentsToFile() {
    ofstream file("student_masterlist.txt");
    for (const auto& s : studentList) {
        file << s.id << "|" << s.name << "|\n";
    }
    file.close();
}

void addStudent() {
    clearScreen();
    cout << "=== ADD STUDENT TO MASTERLIST ===\n";
    cin.ignore();
    string name;
    cout << "Enter Student Name: ";
    getline(cin, name);

    int nextId = studentList.empty() ? 1 : stoi(studentList.back().id) + 1;
    stringstream ss;
    ss << setfill('0') << setw(4) << nextId;

    studentList.push_back({ss.str(), name});
    saveStudentsToFile();

    cout << "\nStudent added with ID: " << ss.str() << "\n";
    pause();
}

void masterlist() {
    clearScreen();
    cout << "=== MASTER LIST ===\n\n";
    if (studentList.empty()) {
        cout << "No students found in the masterlist.\n";
    } else {
        cout << left << setw(10) << "ID" << "Name\n";
        cout << string(30, '-') << "\n";
        for (const auto& s : studentList) {
            cout << left << setw(10) << s.id << s.name << "\n";
        }
    }
    pause();
}

void logAttendanceIn() {
    clearScreen();
    Attendance entry;
    cout << "=== LOG IN ATTENDANCE ===\n";
    cout << "Enter Student Name: ";
    cin.ignore();
    getline(cin, entry.studentName);

    cout << "Enter Subject: ";
    getline(cin, entry.subject);
    do {
        cout << "Enter Parent's Contact Number: ";
        getline(cin, entry.parentNumber);
        if (!isValidNumber(entry.parentNumber)) {
            cout << "Invalid number. Please enter digits only and at least 10 characters.\n";
        }
    } while (!isValidNumber(entry.parentNumber));

    entry.timeIn = getCurrentTime();
    entry.timeOut = "Still in class...";

    records.push_back(entry);
    saveAttendanceToFile();

    string message = "Your child " + entry.studentName + " has entered the class '" +
                     entry.subject + "' at " + entry.timeIn + ".";
    sendSMS(entry.parentNumber, message);

    cout << "\nAttendance logged successfully.\n";
    pause();
}

void logAttendanceOut() {
    clearScreen();
    string name, subject;
    cout << "=== LOG OUT ATTENDANCE ===\n";
    cin.ignore();
    cout << "Enter Student Name: ";
    getline(cin, name);
    cout << "Enter Subject: ";
    getline(cin, subject);

    string lname = toLower(name);
    string lsubject = toLower(subject);

    bool found = false;
    for (auto& r : records) {
        if (toLower(r.studentName) == lname && toLower(r.subject) == lsubject && r.timeOut == "Still in class...") {
            r.timeOut = getCurrentTime();
            cout << "\nLogged out successfully at " << r.timeOut << ".\n";

            string message = "Your child " + r.studentName + " has exited the class '" +
                             r.subject + "' at " + r.timeOut + ".";
            sendSMS(r.parentNumber, message);
            found = true;
            break;
        }
    }
    if (!found)
        cout << "\nNo matching log-in found or already logged out.\n";

    saveAttendanceToFile();
    pause();
}

void viewAttendance() {
    clearScreen();
    cout << "=== ATTENDANCE RECORDS ===\n\n";
    if (records.empty()) {
        cout << "No attendance records found.\n";
    } else {
        cout << left << setw(20) << "Student"
             << setw(20) << "Subject"
             << setw(25) << "Time In"
             << setw(25) << "Time Out" << "\n";
        cout << string(90, '-') << "\n";

        for (const auto& r : records) {
            cout << left << setw(20) << r.studentName
                 << setw(20) << r.subject
                 << setw(25) << r.timeIn
                 << setw(25) << r.timeOut << "\n";
        }
    }
    pause();
}

void addAnnouncement() {
    clearScreen();
    cout << "=== ADD ANNOUNCEMENT ===\n";
    cout << "Enter announcement: ";
    cin.ignore();
    string announcementText;
    getline(cin, announcementText);
    announcements.push_back(announcementText);
    saveAnnouncementsToFile();
    cout << "\nAnnouncement added successfully.\n";
    pause();
}

void viewAnnouncements() {
    clearScreen();
    cout << "=== ANNOUNCEMENTS ===\n\n";
    if (announcements.empty()) {
        cout << "No announcements available.\n";
    } else {
        for (size_t i = 0; i < announcements.size(); ++i) {
            cout << i + 1 << ". " << announcements[i] << "\n";
        }
    }
    pause();
}

void showMenu() {
    int choice;
    do {
        clearScreen();
        cout << "=====================================\n";
        cout << "     CLASSROOM ATTENDANCE SYSTEM     \n";
        cout << "=====================================\n";
        cout << "1. Log IN Attendance\n";
        cout << "2. Log OUT Attendance\n";
        cout << "3. View Attendance Records\n";
        cout << "4. Add Announcement\n";
        cout << "5. View Announcements\n";
        cout << "6. View Master List\n";
        cout << "7. Add Student to Master List\n";
        cout << "8. Exit\n";
        cout << "-------------------------------------\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: logAttendanceIn(); break;
            case 2: logAttendanceOut(); break;
            case 3: viewAttendance(); break;
            case 4: addAnnouncement(); break;
            case 5: viewAnnouncements(); break;
            case 6: masterlist(); break;
            case 7: addStudent(); break;
            case 8: cout << "\nExiting system...\n"; break;
            default:
                cout << "\nInvalid choice! Try again.\n";
                pause();
        }
    } while (choice != 8);
}

bool login() {
    string username, password;
    cout << "=== LOGIN ===\n";
    cout << "Username: ";
    cin >> username;
    cout << "Password: ";
    cin >> password;

    if (username == "Group3" && password == "143778") {
        cout << "\nLogin successful!\n";
        return true;
    } else {
        cout << "\nInvalid username or password. Exiting program...\n";
        return false;
    }
}

int main() {
    if (login()) {
        loadAttendanceFromFile();
        loadAnnouncementsFromFile();
        loadStudentsFromFile();
        showMenu();
    }
    return 0;
}