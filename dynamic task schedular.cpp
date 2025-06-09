#include <iostream>
#include <vector>//stores tasks and substasks
#include <queue>//for reminders
#include <stack>//for undo task
#include <map>//dependencies
#include <fstream>//file handling
#include <sstream>
#include <ctime>
#include <algorithm>
#include<windows.h>
#include<limits>

using namespace std;
class Task {
public:
    int id;
    string title;
    string description;
    string category;
    int priority;
    time_t deadline;
    bool completed;
    vector<int> subtasks;
    vector<int> dependencies;
    int rewardPoints;
    enum RepeatType { NONE = 1, DAILY = 2, WEEKLY = 3 };
    RepeatType repeat;

    Task() {
        id = 0;
        priority = 0;
        deadline = 0;
        completed = false;
        rewardPoints = 0;
        repeat = NONE;
    }
    
    void safeinputint(const string& prompt, int& var, int minVal = INT_MIN, int maxVal = INT_MAX) {
         while (true) {
        cout << prompt;
        cin >> var;
        if (cin.fail() || var < minVal || var > maxVal) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number";
            if (minVal != INT_MIN && maxVal != INT_MAX) {
                cout << " between " << minVal << " and " << maxVal;
            }
            cout << ".\n";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
      }
    }

    void input(int taskId) {
        id = taskId;
        cout << "Enter task title: ";
        cin.ignore();//to ignore \n character
        getline(cin, title);
        cout << "Enter task description: ";
        getline(cin, description);
        cout << "Enter task category: ";
        getline(cin, category);
        while (true) {
            cout << "Enter priority (1-10): ";
            cin >> priority;
            if (cin.fail() || priority < 1 || priority > 10) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a number between 1 and 10.\n";
            } else {
                break;
            }
        }
        int y, m, d, h, min;
        while (true) {
            cout << "Enter deadline (YYYY MM DD HH MM): ";
            cin >> y >> m >> d >> h >> min;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter valid date and time values.\n";
            } else {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            }
        }
        tm t = {};
        t.tm_year = y - 1900;
        t.tm_mon = m - 1;
        t.tm_mday = d;
        t.tm_hour = h;
        t.tm_min = min;
        deadline = mktime(&t);
        cout << "Set recurrence type:\n";
        cout << "1. None\n2. Daily\n3. Weekly\n";
        int repChoice;
        safeinputint("Enter your choice: ", repChoice, 0, 2);
        repeat = static_cast<RepeatType>(repChoice);
        completed = false;
        rewardPoints = 0;
    }

    void display() const {
    	 HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (completed)
        SetConsoleTextAttribute(h, 10);  // Green
    else
        SetConsoleTextAttribute(h, 12);  // Red
        cout << "\nTask ID: " << id << "\nTitle: " << title
             << "\nDescription: " << description
             << "\nCategory: " << category
             << "\nDeadline: " << ctime(&deadline)
             << "Priority: " << priority
             << "\nReward Points: " << rewardPoints
             << "\nStatus: " << (completed ? "Completed" : "Pending") << "\n";
        SetConsoleTextAttribute(h, 7); // Reset to default color
    }
    
    void save(ofstream& file) const {
        file << id << "\n" << title << "\n" << description << "\n"
             << priority << "\n" << deadline << "\n" << completed << "\n"
             << rewardPoints << "\n";
             file << repeat << "\n";
        file << subtasks.size() << "\n";
        for (int i = 0; i < subtasks.size(); i++) file << subtasks[i] << " ";
        file << "\n" << dependencies.size() << "\n";
        for (int i = 0; i < dependencies.size(); i++) file << dependencies[i] << " ";
        file << "\n";
    }

    void load(ifstream& file) {
        int stsize, depsize, temp;
        file >> id;
        file.ignore();
        getline(file, title);
        getline(file, description);
        file >> priority >> deadline >> completed >> rewardPoints;
        int repVal;
        file >> repVal;
        repeat = static_cast<RepeatType>(repVal);
        file >> stsize;
        subtasks.clear();
        for (int i = 0; i < stsize; i++) {
            file >> temp;
            subtasks.push_back(temp);
        }
        file >> depsize;
        dependencies.clear();
        for (int i = 0; i < depsize; i++) {
            file >> temp;
            dependencies.push_back(temp);
        }
    }
};

vector<Task> tasks;
stack<vector<Task> > undoStack;
map<int, vector<int> > graph;
string currentUser = ""; // stores the currently logged-in username

void safeInputInt(const string& prompt, int& var, int minVal = INT_MIN, int maxVal = INT_MAX) {
    while (true) {
        cout << prompt;
        cin >> var;
        if (cin.fail() || var < minVal || var > maxVal) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number";
            if (minVal != INT_MIN && maxVal != INT_MAX) {
                cout << " between " << minVal << " and " << maxVal;
            }
            cout << ".\n";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
    }
}

void saveTasks() {
    if (currentUser.empty()) return; // no user, no saving
    string filename = "tasks_" + currentUser + ".txt";
    ofstream file(filename.c_str());
    file << tasks.size() << "\n";
    for (int i = 0; i < tasks.size(); i++) {
        tasks[i].save(file);
    }
    file.close();
}

void loadTasks() {
    if (currentUser.empty()) return;
    string filename = "tasks_" + currentUser + ".txt";
    ifstream file(filename.c_str());
    if (!file) {
        tasks.clear(); // no existing file
        return;
    }
    int size;
    file >> size;
    tasks.clear();
    for (int i = 0; i < size; i++) {
        Task t;
        t.load(file);
        tasks.push_back(t);
    }
    file.close();
}

void addTask() {
    undoStack.push(tasks);//push the task to stack
    Task t;
    int newId = tasks.empty() ? 1 : tasks.back().id + 1;
    t.input(newId);
    tasks.push_back(t);
    cout << "Task added successfully!\n";
    saveTasks();
}

void displayTasks() {
    if (tasks.empty()) {
        cout << "No tasks to display.\n";
        return;
    }
    for (int i = 0; i < tasks.size(); i++) {
        // Check if current task is a subtask
        bool isSubtask = false;
        for (int j = 0; j < tasks.size(); j++) {
            if (j == i) continue;
            for (int k = 0; k < tasks[j].subtasks.size(); k++) {
                if (tasks[j].subtasks[k] == tasks[i].id) {
                    isSubtask = true;
                    break;
                }
            }
            if (isSubtask) break;
        }
        // Only display main tasks
        if (!isSubtask) {
            tasks[i].display();
            // --- Display Subtasks ---
            if (!tasks[i].subtasks.empty()) {
                cout << "\t--- Subtasks ---\n";
                for (int j = 0; j < tasks[i].subtasks.size(); j++) {
                    int subId = tasks[i].subtasks[j];
                    bool found = false;
                    for (int k = 0; k < tasks.size(); k++) {
                        if (tasks[k].id == subId && subId != tasks[i].id) {
                            cout << "\t(Subtask)\n";
                            tasks[k].display();
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        cout << "\t(Subtask ID " << subId << " not found or invalid)\n";
                    }
                }
            }
            // --- Display Dependencies ---
            if (!tasks[i].dependencies.empty()) {
                cout << "\t--- Dependencies ---\n";
                for (int j = 0; j < tasks[i].dependencies.size(); j++) {
                    int depId = tasks[i].dependencies[j];
                    bool found = false;
                    for (int k = 0; k < tasks.size(); k++) {
                        if (tasks[k].id == depId && depId != tasks[i].id) {
                            cout << "\t(Dependency)\n";
                            tasks[k].display();
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        cout << "\t(Dependency ID " << depId << " not found or invalid)\n";
                    }
                }
            }
            cout << "-----------------------------\n";
        }
    }
}

void searchTask() {
    string keyword;
    cout << "Enter task to search: ";
    cin.ignore();
    getline(cin, keyword);
    for (int i = 0; i < tasks.size(); i++) {
        if (tasks[i].title.find(keyword) != string::npos || tasks[i].description.find(keyword) != string::npos) {
            tasks[i].display();
            break;
        }else{
        	cout<<"Enter valid task to serach."<<endl;
		}
    }
}

void markCompleted() {
    int id;
     safeInputInt("Enter Task ID to mark as completed: ", id);
    for (int i = 0; i < tasks.size(); i++) {
        if (tasks[i].id == id) {
            // Check if dependencies are completed
            bool allDepsDone = true;
            for (int j = 0; j < tasks[i].dependencies.size(); j++) {
                int depId = tasks[i].dependencies[j];
                bool found = false;
                for (int k = 0; k < tasks.size(); k++) {
                    if (tasks[k].id == depId) {
                        found = true;
                        if (!tasks[k].completed) {
                            allDepsDone = false;
                            cout << "Cannot complete this task. Dependency Task ID " << depId << " is still pending.\n";
                        }
                        break;
                    }
                }
                if (!found) {
                    allDepsDone = false;
                    cout << "Dependency Task ID " << depId << " not found.\n";
                }
            }
            if (!allDepsDone) return;
            undoStack.push(tasks);
            tasks[i].completed = true;
            if (tasks[i].repeat != Task::NONE) {
    Task newTask = tasks[i]; // copy existing task
    newTask.id = tasks.back().id + 1;
    // update deadline
    tm* deadlineTm = localtime(&newTask.deadline);
    if (newTask.repeat == Task::DAILY) {
        deadlineTm->tm_mday += 1;
    } else if (newTask.repeat == Task::WEEKLY) {
        deadlineTm->tm_mday += 7;
    }
    newTask.deadline = mktime(deadlineTm);
    newTask.completed = false;
    newTask.rewardPoints = 0;
    tasks.push_back(newTask);
    cout << "\nRecurring task created with new ID: " << newTask.id << "\n";
}
            tasks[i].rewardPoints += 10;
            cout << " +10 reward points!\n";
            cout<<"Yahooo!!!You have completed this task.Keep it up!!";
            saveTasks();
            return;
        }
    }
    cout << "Task not found.\n";
}

void checkReminders() {
    queue<Task> reminderQueue;
    time_t now = time(0);
    bool oneHourReminderShown = false;
    for (int i = 0; i < tasks.size(); i++) {
        if (!tasks[i].completed) {
            double secondsToDeadline = difftime(tasks[i].deadline, now);
            // 1-hour reminder
            if (!oneHourReminderShown && secondsToDeadline <= 3600 && secondsToDeadline > 0) {
                string message = "Reminder: Task \"" + tasks[i].title + "\" is due in 1 hour!";
                MessageBox(NULL, message.c_str(), "1 Hour Deadline Reminder", MB_OK | MB_ICONWARNING);
                oneHourReminderShown = true;
            }
            // 24-hour reminders
            if (secondsToDeadline <= 86400 && secondsToDeadline > 0) {
                reminderQueue.push(tasks[i]);
            }
        }
    }
    if (reminderQueue.empty()) {
        cout << "No upcoming tasks due within 24 hours.\n";
    } else {
        cout << "Upcoming Tasks (Due within 24 hours):\n";
        while (!reminderQueue.empty()) {
            Task t = reminderQueue.front();
            reminderQueue.pop();
            cout << "\nTask ID: " << t.id
                 << "\nTitle: " << t.title
                 << "\nDeadline: " << ctime(&t.deadline)
                 << "Priority: " << t.priority
                 << "\nStatus: " << (t.completed ? "Completed" : "Pending") << "\n";
        }
    }
}

void addSubtask() {
    int parentId;
    safeInputInt("Enter parent task ID: ", parentId);
    for (int i = 0; i < tasks.size(); i++) {
        if (tasks[i].id == parentId) {
            Task sub;
            int newId = tasks.empty() ? 1 : tasks.back().id + 1;
            undoStack.push(tasks);
			sub.input(newId);
            tasks.push_back(sub);
            tasks[i].subtasks.push_back(newId);
            cout << "New subtask added and linked successfully.\n";
            saveTasks();
            return;
        }
    }
    cout << "Parent task not found.\n";
}

void addDependency() {
    string taskTitle, depTitle;
    // Before getline(cin, taskTitle); check for leftovers
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter Task Title: ";
    getline(cin, taskTitle);
    cout << "Enter Dependency Task Title: ";
    getline(cin, depTitle);
    int taskId = -1, depId = -1;
    for (int i = 0; i < tasks.size(); i++) {
        if (tasks[i].title == taskTitle) {
            taskId = tasks[i].id;
        }
        if (tasks[i].title == depTitle) {
            depId = tasks[i].id;
        }
    }
    if (taskId == -1 || depId == -1) {
        cout << "One or both task titles not found.\n";
        return;
    }
    for (int i = 0; i < tasks.size(); i++) {
        if (tasks[i].id == taskId) {
        	undoStack.push(tasks);
            tasks[i].dependencies.push_back(depId);
            graph[depId].push_back(taskId);
            cout << "Dependency added successfully.\n";
            saveTasks();
            return;
        }
    }
}

void undoLastAction() {
    if (!undoStack.empty()) {
        tasks = undoStack.top();
        undoStack.pop();
        saveTasks();
        cout << "Undo successful.\n";
    } else {
        cout << "No actions to undo.\n";
    }
}

void editTask() {
    int id;
    safeInputInt("Enter Task ID to edit: ", id);
    for (int i = 0; i < tasks.size(); i++) {
        if (tasks[i].id == id) {
            cout << "Editing Task:\n";
            // Push current state for undo
            undoStack.push(tasks);
            cin.ignore();
            cout << "Enter new title (current: " << tasks[i].title << "): ";
            getline(cin, tasks[i].title);
            cout << "Enter new description (current: " << tasks[i].description << "): ";
            getline(cin, tasks[i].description);
            cout << "Enter new category (current: " << tasks[i].category << "): ";
            getline(cin, tasks[i].category);
            cout << "Enter new priority (current: " << tasks[i].priority << "): ";
            while (true) {
            cout << "Enter new priority (current: " << tasks[i].priority << "): ";
            cin >> tasks[i].priority;
            if (cin.fail() || tasks[i].priority < 1 || tasks[i].priority > 10) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number between 1 and 10.\n";
             }else {
               cin.ignore(numeric_limits<streamsize>::max(), '\n');
               break;
              }
            }   
            int y, m, d, h, min;
            cout << "Enter new deadline (YYYY MM DD HH MM): ";
            cin >> y >> m >> d >> h >> min;
            tm t = {};
            t.tm_year = y - 1900;
            t.tm_mon = m - 1;
            t.tm_mday = d;
            t.tm_hour = h;
            t.tm_min = min;
            tasks[i].deadline = mktime(&t);
            // Dependencies update
            char depChoice;
            cout << "Do you want to update dependencies? (y/n): ";
            cin >> depChoice;
            if (depChoice == 'y' || depChoice == 'Y') {
                tasks[i].dependencies.clear();
                cin.ignore();
                string depTitle;
                while (true) {
                    cout << "Enter dependency task title (or type 'done' to finish): ";
                    getline(cin, depTitle);
                    if (depTitle == "done") break;
                    bool found = false;
                    for (int j = 0; j < tasks.size(); j++) {
                        if (tasks[j].title == depTitle) {
                            tasks[i].dependencies.push_back(tasks[j].id);
                            graph[tasks[j].id].push_back(id);
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        cout << "Task with title \"" << depTitle << "\" not found.\n";
                    }
                }
            }
            saveTasks();
            cout << "Task updated successfully.\n";
            return;
        }
    }
    cout << "Task not found.\n";
}

void filterTasks() {
    int choice;
    cout << "\n--- Filter Options ---\n";
    cout << "1. Show Completed Tasks\n";
    cout << "2. Show Pending Tasks\n";
    cout << "3. Show High Priority Tasks (priority > 7)\n";
    cout << "4. Show Low Priority Tasks (priority <= 3)\n";
    safeInputInt("Enter your choice: ", choice, 1, 4);
    bool found = false;
    for (int i = 0; i < tasks.size(); i++) {
        bool match = false;
        if (choice == 1 && tasks[i].completed) {
            match = true;
        } else if (choice == 2 && !tasks[i].completed) {
            match = true;
        } else if (choice == 3 && tasks[i].priority > 7) {
            match = true;
        } else if (choice == 4 && tasks[i].priority <= 3) {
            match = true;
        }
        if (match) {
            tasks[i].display();
            found = true;
        }
    }
    if (!found) {
        cout << "No matching tasks found.\n";
    }
}

void deleteTask() {
    int id;
    safeInputInt("Enter Task ID to delete: ", id);
    bool found = false;
    for (int i = 0; i < tasks.size(); i++) {
        if (tasks[i].id == id) {
            undoStack.push(tasks);
            tasks.erase(tasks.begin() + i);
            saveTasks();
            cout << "Task deleted successfully.\n";
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "Task not found.\n";
    }
}

void sortTasks() {
    if (tasks.empty()) {
        cout << "No tasks available to sort.\n";
        return;
    }

    int choice;
    cout << "\n--- Sort Options ---\n";
    cout << "1. By Nearest Deadline\n";
    cout << "2. By Highest Priority\n";
    cout << "3. By Most Reward Points (includes completed tasks)\n";
    safeInputInt("Enter your choice: ", choice, 1, 3);
    vector<Task> sortedTasks;
    for (int i = 0; i < tasks.size(); i++) {
        if (choice == 3 || !tasks[i].completed) {
            sortedTasks.push_back(tasks[i]);
        }
    }
    if (sortedTasks.empty()) {
        cout << "No matching tasks to sort.\n";
        return;
    }
    for (int i = 0; i < sortedTasks.size() - 1; i++) {
        for (int j = i + 1; j < sortedTasks.size(); j++) {
            bool shouldSwap = false;
            if (choice == 1 && sortedTasks[i].deadline > sortedTasks[j].deadline) {
                shouldSwap = true;
            } else if (choice == 2 && sortedTasks[i].priority < sortedTasks[j].priority) {
                shouldSwap = true;
            } else if (choice == 3 && sortedTasks[i].rewardPoints < sortedTasks[j].rewardPoints) {
                shouldSwap = true;
            }
            if (shouldSwap) {
                swap(sortedTasks[i], sortedTasks[j]);
            }
        }
    }
    cout << "\n--- Sorted Tasks ---\n";
    for (int i = 0; i < sortedTasks.size(); i++) {
        sortedTasks[i].display();
    }
}

void suggestTask() {
    if (tasks.empty()) {
        cout << "No tasks to suggest.\n";
        return;
    }
    Task* earliest = NULL;
    Task* easiest = NULL;
    Task* quickWin = NULL;
    for (int i = 0; i < tasks.size(); i++) {
        if (tasks[i].completed) continue;
        // ?? Earliest Deadline
        if (earliest == NULL || tasks[i].deadline < earliest->deadline) {
            earliest = &tasks[i];
        }
        // ?? Least Dependencies
        if (easiest == NULL || tasks[i].dependencies.size() < easiest->dependencies.size()) {
            easiest = &tasks[i];
        }
        // ?? Lowest Reward (Quick win)
        if (quickWin == NULL || tasks[i].rewardPoints < quickWin->rewardPoints) {
            quickWin = &tasks[i];
        }
    }
    cout << "\n--- Suggested Tasks ---\n";
    if (earliest) {
        cout << "\n?? Task with Nearest Deadline:\n";
        earliest->display();
    }
    if (easiest) {
        cout << "\n?? Task with Least Dependencies:\n";
        easiest->display();
    }
    if (quickWin) {
        cout << "\n?? Quick Win Task (Lowest Reward):\n";
        quickWin->display();
    }
}

bool userExists(const string& username) {
    ifstream infile("users.txt");
    string u, p;
    while (infile >> u >> p) {
        if (u == username) {
            return true;
        }
    }
    return false;
}

bool checkCredentials(const string& username, const string& password) {
    ifstream infile("users.txt");
    string u, p;
    while (infile >> u >> p) {
        if (u == username && p == password) {
            return true;
        }
    }
    return false;
}

bool signup() {
    string username, password, confirm;
    cout << "\n=== Signup ===\n";
    cout << "Enter username: ";
    cin >> username;
    currentUser = username;
    if (userExists(username)) {
        cout << "Username already exists.\n";
        return false;
    }
    cout << "Enter password: ";
    cin >> password;
    cout << "Confirm password: ";
    cin >> confirm;
    if (password != confirm) {
        cout << "Passwords do not match.\n";
        return false;
    }
    ofstream outfile("users.txt", ios::app);
    outfile << username << " " << password << endl;
    outfile.close();
    cout << "Signup successful!\n";
    currentUser = username; // set logged in user
    return true;
}

bool login() {
    string username, password;
    cout << "\n=== Login ===\n";
    cout << "Enter username: ";
    cin >> username;
    currentUser = username;
    cout << "Enter password: ";
    cin >> password;
    if (checkCredentials(username, password)) {
        cout << "Login successful! Welcome, " << username << ".\n";
        currentUser = username; // set logged in user
        return true;
    } else {
        cout << "Invalid username or password.\n";
        return false;
    }
}

void calculateCompletionPercentage() {
    if (tasks.empty()) {
        cout << "No tasks to evaluate.\n";
        return;
    }
    int completedTasks = 0;
    for (int i = 0; i < tasks.size(); i++) {
        if (tasks[i].completed)
            completedTasks++;
    }
    float percentage = (static_cast<float>(completedTasks) / tasks.size()) * 100;
    cout << "\n--- Completion Summary ---\n";
    cout << "Total Tasks: " << tasks.size() << endl;
    cout << "Completed Tasks: " << completedTasks << endl;
    cout << "Completion Percentage: " << percentage << "%" << endl;
    if (percentage == 100)
        cout << "Outstanding! All tasks are done!\n";
    else if (percentage >= 75)
        cout << "Great job! Keep up the momentum.\n";
    else if (percentage >= 50)
        cout << "You're halfway there. Stay focused!\n";
    else
        cout << "Let's push harder. You can do it!\n";
}

int main() {
	int authChoice;
    bool loggedIn = false;
    while (!loggedIn) {
        cout << "\n1. Login\n2. Signup\n3. Exit\nEnter your choice: ";
        cin >> authChoice;
        if (cin.fail()) {
            cin.clear(); // clear error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard invalid input
            cout << "Invalid input. Please enter a number between 1 and 3.\n";
            continue;
        }
        switch (authChoice) {
            case 1: loggedIn = login(); break;
            case 2: loggedIn = signup(); break;
            case 3: cout << "Goodbye!\n"; return 0;
            default: cout << "Invalid choice.\n";
        }
    }
    if (loggedIn) {
    loadTasks(); // Load the user's tasks
}

    cout<<"========FOCUS TRACK========\n";
    int choice;
    while (true) {
        cout << "\n--- Dynamic Task Scheduler ---\n";
        cout << "1. Add Task\n2. Display Tasks\n3. Search Task\n4. Mark Task as Completed\n";
        cout << "5. Check Reminders\n6. Add Sub Tasks\n7. Add Dependencies\n";
        cout << "8. Undo Last Action\n9. Edit Task\n10. Filter Tasks by Status\n11. Delete Task\n12. Sort Tasks\n13. Suggest Task to Work On\n14. Percentage Calculation\n15. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        if (cin.fail()) {
            cin.clear(); // clear error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard invalid input
            cout << "Invalid input. Please enter a number between 1 and 10.\n";
            continue;
        } 
        switch (choice) {
            case 1: addTask(); break;
            case 2: displayTasks(); break;
            case 3: searchTask(); break;
            case 4: markCompleted(); break;
            case 5: checkReminders(); break;
            case 6: addSubtask(); break;
            case 7: addDependency(); break;
            case 8: undoLastAction(); break;
            case 9: editTask(); break;
            case 10: filterTasks(); break;
            case 11: deleteTask();break;
            case 12: sortTasks(); break;
            case 13: suggestTask(); break;
            case 14: {
    saveTasks(); // Save all tasks to file
    int totalPoints = 0;
    for (int i = 0; i < tasks.size(); i++) {
        totalPoints += tasks[i].rewardPoints;
    }
    cout << "You have gained a total of " << totalPoints << " reward points." << endl;
    if(totalPoints <= 20)
        cout << "Keep going\n";
    else if(totalPoints <= 50)
        cout << "Keep it up\n";
    else
        cout << "You are doing a great job. Awesome!!\n";
    calculateCompletionPercentage();  // ?? Add this line
    cout << "Goodbye!" << endl;
    break;
}
            case 15: {
    saveTasks(); // Save all tasks to file
    int totalPoints = 0;
    for (int i = 0; i < tasks.size(); i++) {
        totalPoints += tasks[i].rewardPoints; // Sum up reward points
    }
    cout << "You have gained a total of " << totalPoints << " reward points." << endl;
    if(totalPoints <=20)
    cout<<"Keep going\n";
    else if(totalPoints>20&&totalPoints<=50)
    cout<<"Keep it up\n";
    else
    cout<<"You are doing a great job.Awsome!!\n";
    cout << "Goodbye!" << endl;
    return 0;
}
            default: cout << "Invalid choice.\n"; break;
        }
    }
}
