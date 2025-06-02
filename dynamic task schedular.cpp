#include <iostream>
#include <vector>//stores tasks and substasks
#include <queue>//for reminders
#include <stack>//for undo task
#include <map>//dependencies
#include <fstream>//file handling
#include <ctime>
#include <algorithm>
#include<windows.h>

using namespace std;

class Task {
public:
    int id;
    string title;
    string description;
    int priority;
    time_t deadline;
    bool completed;
    vector<int> subtasks;
    vector<int> dependencies;
    int rewardPoints;

    Task() {
        id = 0;
        priority = 0;
        deadline = 0;
        completed = false;
        rewardPoints = 0;
    }

    void input(int taskId) {
        id = taskId;
        cout << "Enter task title: ";
        cin.ignore();//to ignore \n character
        getline(cin, title);
        cout << "Enter task description: ";
        getline(cin, description);
        cout << "Enter priority (1-10): ";
        cin >> priority;

        int y, m, d, h, min;
        cout << "Enter deadline (YYYY MM DD HH MM): ";
        cin >> y >> m >> d >> h >> min;

        tm t = {};
        t.tm_year = y - 1900;
        t.tm_mon = m - 1;
        t.tm_mday = d;
        t.tm_hour = h;
        t.tm_min = min;
        deadline = mktime(&t);
        completed = false;
        rewardPoints = 0;
    }

    void display() const {
        cout << "\nTask ID: " << id << "\nTitle: " << title
             << "\nDescription: " << description
             << "\nDeadline: " << ctime(&deadline)
             << "Priority: " << priority
             << "\nReward Points: " << rewardPoints
             << "\nStatus: " << (completed ? "Completed" : "Pending") << "\n";
    }

    void save(ofstream& file) const {
        file << id << "\n" << title << "\n" << description << "\n"
             << priority << "\n" << deadline << "\n" << completed << "\n"
             << rewardPoints << "\n";

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

void saveTasks() {
    ofstream file("tasks.txt");
    file << tasks.size() << "\n";
    for (int i = 0; i < tasks.size(); i++) {
        tasks[i].save(file);
    }
    file.close();
}

void loadTasks() {
    ifstream file("tasks.txt");
    if (!file) return;

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
        // Skip if this task is a subtask of another task
        bool isSubtask = false;
        for (int j = 0; j < tasks.size(); j++) {
            for (int k = 0; k < tasks[j].subtasks.size(); k++) {
                if (tasks[j].subtasks[k] == tasks[i].id) {
                    isSubtask = true;
                    break;
                }
            }
            if (isSubtask) break;
        }

        if (!isSubtask) {
            // Display main task
            tasks[i].display();

            // Display subtasks
            if (!tasks[i].subtasks.empty()) {//if there exits a subtask
                cout << "\t--- Subtasks ---\n";
                for (int j = 0; j < tasks[i].subtasks.size(); j++) {
                    int subId = tasks[i].subtasks[j];
                    for (int k = 0; k < tasks.size(); k++) {
                        if (tasks[k].id == subId) {
                            cout << "\t(Subtask)\n";
                            cout << "\tTask ID: " << tasks[k].id << "\n\tTitle: " << tasks[k].title
                                 << "\n\tDescription: " << tasks[k].description
                                 << "\n\tDeadline: " << ctime(&tasks[k].deadline)
                                 << "\tPriority: " << tasks[k].priority
                                 << "\n\tReward Points: " << tasks[k].rewardPoints
                                 << "\n\tStatus: " << (tasks[k].completed ? "Completed" : "Pending") << "\n\n";
                            break;
                        }
                    }
                }
            }

            // Display dependencies
            if (!tasks[i].dependencies.empty()) {
                cout << "\t--- Dependencies ---\n";
                for (int j = 0; j < tasks[i].dependencies.size(); j++) {
                    int depId = tasks[i].dependencies[j];
                    for (int k = 0; k < tasks.size(); k++) {
                        if (tasks[k].id == depId) {
                            cout << "\t(Dependency)\n";
                            cout << "\tTask ID: " << tasks[k].id << "\n\tTitle: " << tasks[k].title
                                 << "\n\tDescription: " << tasks[k].description
                                 << "\n\tDeadline: " << ctime(&tasks[k].deadline)
                                 << "\tPriority: " << tasks[k].priority
                                 << "\n\tReward Points: " << tasks[k].rewardPoints
                                 << "\n\tStatus: " << (tasks[k].completed ? "Completed" : "Pending") << "\n\n";
                            break;
                        }
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
        }
    }
}

void markCompleted() {
    int id;
    cout << "Enter Task ID to mark as completed: ";
    cin >> id;

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

            tasks[i].completed = true;
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
    cout << "Enter parent task ID: ";
    cin >> parentId;

    for (int i = 0; i < tasks.size(); i++) {
        if (tasks[i].id == parentId) {
            Task sub;
            int newId = tasks.empty() ? 1 : tasks.back().id + 1;
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
    cout << "Enter Task Title: ";
    cin.ignore();
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
    cout << "Enter Task ID to edit: ";
    cin >> id;

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

            cout << "Enter new priority (current: " << tasks[i].priority << "): ";
            cin >> tasks[i].priority;

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


int main() {
    loadTasks();
    cout<<"========FOCUS TRACK========\n";

    int choice;
    while (true) {
        cout << "\n--- Dynamic Task Scheduler ---\n";
        cout << "1. Add Task\n2. Display Tasks\n3. Search Task\n4. Mark Task as Completed\n";
        cout << "5. Check Reminders\n6. Add Sub Tasks\n7. Add Dependencies\n";
        cout << "8. Undo Last Action\n9. Edit Task\n10. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

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
            case 10: {
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
