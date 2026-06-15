#include "TaskManager.hpp"

#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

std::string statusBadge(Status status) {
    switch (status) {
        case Status::Open: return "OPEN";
        case Status::InProgress: return "WORKING";
        case Status::Done: return "DONE";
    }
    return "UNKNOWN";
}

void printTask(const Task& task) {
    std::cout << "[" << task.id << "] "
              << task.title << "  "
              << "(Due: " << format_date(task.dueDate)
              << ", Priority: " << to_string(task.priority)
              << ", Status: " << statusBadge(task.status) << ")\n";
    if (!task.description.empty()) {
        std::cout << "     " << task.description << "\n";
    }
}

void printTasks(const std::vector<Task>& tasks, const std::string& title) {
    std::cout << "\n== " << title << " ==\n";
    if (tasks.empty()) {
        std::cout << "No tasks found.\n";
        return;
    }
    for (const auto& task : tasks) {
        printTask(task);
    }
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

int readInt(const std::string& prompt) {
    while (true) {
        auto text = readLine(prompt);
        try {
            std::size_t pos = 0;
            int value = std::stoi(text, &pos);
            if (pos != text.size()) throw std::invalid_argument("extra text");
            return value;
        } catch (...) {
            std::cout << "Please enter a valid number.\n";
        }
    }
}

std::chrono::sys_days readDate(const std::string& prompt) {
    while (true) {
        const auto text = readLine(prompt);
        auto date = parse_date(text);
        if (date) {
            return *date;
        }
        std::cout << "Use the format YYYY-MM-DD.\n";
    }
}

Priority readPriority() {
    while (true) {
        std::cout << "Priority: 1=Low, 2=Medium, 3=High\n";
        const int value = readInt("Choose priority: ");
        auto p = priority_from_int(value);
        if (p) return *p;
        std::cout << "Invalid priority selection.\n";
    }
}

Status readStatus() {
    while (true) {
        std::cout << "Status: 0=Open, 1=In Progress, 2=Done\n";
        const int value = readInt("Choose status: ");
        auto s = status_from_int(value);
        if (s) return *s;
        std::cout << "Invalid status selection.\n";
    }
}

void showMenu() {
    std::cout
        << "\n==== ProjectPulse ====\n"
        << "1) Add task\n"
        << "2) List all tasks\n"
        << "3) List tasks by status\n"
        << "4) Search tasks\n"
        << "5) Sort by due date\n"
        << "6) Sort by priority\n"
        << "7) Edit task\n"
        << "8) Mark task done\n"
        << "9) Delete task\n"
        << "0) Save and exit\n";
}

} // namespace

int main() {
    try {
        const auto storage = std::filesystem::path("data") / "tasks.db";
        TaskManager manager(storage);
        manager.load();

        bool running = true;
        while (running) {
            showMenu();
            const int choice = readInt("Select option: ");

            switch (choice) {
                case 1: {
                    const auto title = readLine("Title: ");
                    const auto description = readLine("Description: ");
                    const auto dueDate = readDate("Due date (YYYY-MM-DD): ");
                    const auto priority = readPriority();
                    const int id = manager.addTask(title, description, dueDate, priority);
                    std::cout << "Task created with ID " << id << ".\n";
                    break;
                }
                case 2:
                    printTasks(manager.listAll(), "All Tasks");
                    break;
                case 3: {
                    const auto status = readStatus();
                    printTasks(manager.listByStatus(status), "Filtered Tasks");
                    break;
                }
                case 4: {
                    const auto query = readLine("Search text: ");
                    printTasks(manager.search(query), "Search Results");
                    break;
                }
                case 5:
                    printTasks(manager.sortedByDueDate(), "Sorted by Due Date");
                    break;
                case 6:
                    printTasks(manager.sortedByPriority(), "Sorted by Priority");
                    break;
                case 7: {
                    const int id = readInt("Task ID to edit: ");
                    const auto* existing = manager.findById(id);
                    if (!existing) {
                        std::cout << "Task not found.\n";
                        break;
                    }

                    std::cout << "Leave title empty to keep the existing value.\n";
                    const auto title = readLine("New title: ");
                    const auto description = readLine("New description: ");
                    const auto dueText = readLine("New due date (YYYY-MM-DD): ");

                    auto dueDate = parse_date(dueText);
                    if (!dueDate) {
                        std::cout << "Invalid date.\n";
                        break;
                    }

                    const auto priority = readPriority();
                    const auto status = readStatus();

                    const auto finalTitle = title.empty() ? existing->title : title;
                    const auto finalDescription = description.empty() ? existing->description : description;

                    if (manager.editTask(id, finalTitle, finalDescription, *dueDate, priority, status)) {
                        std::cout << "Task updated.\n";
                    } else {
                        std::cout << "Update failed.\n";
                    }
                    break;
                }
                case 8: {
                    const int id = readInt("Task ID to mark done: ");
                    if (manager.markDone(id)) {
                        std::cout << "Task marked complete.\n";
                    } else {
                        std::cout << "Task not found.\n";
                    }
                    break;
                }
                case 9: {
                    const int id = readInt("Task ID to delete: ");
                    if (manager.removeTask(id)) {
                        std::cout << "Task deleted.\n";
                    } else {
                        std::cout << "Task not found.\n";
                    }
                    break;
                }
                case 0:
                    manager.save();
                    running = false;
                    std::cout << "Saved. Goodbye.\n";
                    break;
                default:
                    std::cout << "Invalid option.\n";
                    break;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}