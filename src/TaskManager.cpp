#include "TaskManager.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std::chrono;

namespace {
std::vector<std::string> splitEscaped(const std::string& line, char delimiter = '|') {
    std::vector<std::string> parts;
    std::string current;
    bool escaped = false;

    for (char c : line) {
        if (escaped) {
            current.push_back(c);
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == delimiter) {
            parts.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    parts.push_back(current);
    return parts;
}

std::string toStorageLine(const Task& task) {
    std::ostringstream out;
    out << task.id << '|'
        << escape_field(task.title) << '|'
        << escape_field(task.description) << '|'
        << format_date(task.dueDate) << '|'
        << static_cast<int>(task.priority) << '|'
        << static_cast<int>(task.status) << '|'
        << duration_cast<seconds>(task.createdAt.time_since_epoch()).count();
    return out.str();
}

std::optional<Task> fromStorageLine(const std::string& line) {
    const auto parts = splitEscaped(line);
    if (parts.size() != 7) {
        return std::nullopt;
    }

    Task task{};
    try {
        task.id = std::stoi(parts[0]);
        task.title = unescape_field(parts[1]);
        task.description = unescape_field(parts[2]);

        auto due = parse_date(parts[3]);
        if (!due) return std::nullopt;
        task.dueDate = *due;

        auto priority = priority_from_int(std::stoi(parts[4]));
        auto status = status_from_int(std::stoi(parts[5]));
        if (!priority || !status) return std::nullopt;
        task.priority = *priority;
        task.status = *status;

        const auto epochSeconds = std::stoll(parts[6]);
        task.createdAt = system_clock::time_point{seconds{epochSeconds}};
        return task;
    } catch (...) {
        return std::nullopt;
    }
}

bool containsInsensitive(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;

    auto lower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };
    std::string h(haystack.size(), '\0');
    std::string n(needle.size(), '\0');
    std::transform(haystack.begin(), haystack.end(), h.begin(), lower);
    std::transform(needle.begin(), needle.end(), n.begin(), lower);
    return h.find(n) != std::string::npos;
}
}

TaskManager::TaskManager(std::filesystem::path storagePath)
    : storagePath_(std::move(storagePath)) {}

void TaskManager::load() {
    tasks_.clear();
    nextId_ = 1;

    std::ifstream in(storagePath_);
    if (!in.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        auto task = fromStorageLine(line);
        if (task) {
            tasks_.push_back(*task);
        }
    }

    reindexNextId();
}

void TaskManager::save() const {
    std::filesystem::create_directories(storagePath_.parent_path());
    std::ofstream out(storagePath_, std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("Unable to open storage file for writing: " + storagePath_.string());
    }

    for (const auto& task : tasks_) {
        out << toStorageLine(task) << '\n';
    }
}

int TaskManager::addTask(std::string title,
                         std::string description,
                         std::chrono::sys_days dueDate,
                         Priority priority) {
    Task task{};
    task.id = nextId_++;
    task.title = std::move(title);
    task.description = std::move(description);
    task.dueDate = dueDate;
    task.priority = priority;
    task.status = Status::Open;
    task.createdAt = system_clock::now();

    tasks_.push_back(std::move(task));
    return tasks_.back().id;
}

bool TaskManager::editTask(int id,
                           const std::string& newTitle,
                           const std::string& newDescription,
                           std::chrono::sys_days newDueDate,
                           Priority newPriority,
                           Status newStatus) {
    auto* task = findById(id);
    if (!task) {
        return false;
    }

    task->title = newTitle;
    task->description = newDescription;
    task->dueDate = newDueDate;
    task->priority = newPriority;
    task->status = newStatus;
    return true;
}

bool TaskManager::markDone(int id) {
    auto* task = findById(id);
    if (!task) {
        return false;
    }
    task->status = Status::Done;
    return true;
}

bool TaskManager::removeTask(int id) {
    const auto before = tasks_.size();
    tasks_.erase(std::remove_if(tasks_.begin(), tasks_.end(), [id](const Task& task) {
        return task.id == id;
    }), tasks_.end());
    return tasks_.size() != before;
}

Task* TaskManager::findById(int id) {
    auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const Task& task) {
        return task.id == id;
    });
    return it == tasks_.end() ? nullptr : &*it;
}

const Task* TaskManager::findById(int id) const {
    auto it = std::find_if(tasks_.begin(), tasks_.end(), [id](const Task& task) {
        return task.id == id;
    });
    return it == tasks_.end() ? nullptr : &*it;
}

std::vector<Task> TaskManager::search(const std::string& query) const {
    std::vector<Task> result;
    for (const auto& task : tasks_) {
        if (containsInsensitive(task.title, query) || containsInsensitive(task.description, query)) {
            result.push_back(task);
        }
    }
    return result;
}

std::vector<Task> TaskManager::listAll() const {
    return tasks_;
}

std::vector<Task> TaskManager::listByStatus(Status status) const {
    std::vector<Task> result;
    for (const auto& task : tasks_) {
        if (task.status == status) {
            result.push_back(task);
        }
    }
    return result;
}

std::vector<Task> TaskManager::sortedByDueDate() const {
    auto result = tasks_;
    std::sort(result.begin(), result.end(), [](const Task& a, const Task& b) {
        if (a.dueDate == b.dueDate) {
            return a.priority > b.priority;
        }
        return a.dueDate < b.dueDate;
    });
    return result;
}

std::vector<Task> TaskManager::sortedByPriority() const {
    auto result = tasks_;
    std::sort(result.begin(), result.end(), [](const Task& a, const Task& b) {
        if (a.priority == b.priority) {
            return a.dueDate < b.dueDate;
        }
        return a.priority > b.priority;
    });
    return result;
}

void TaskManager::reindexNextId() {
    int maxId = 0;
    for (const auto& task : tasks_) {
        maxId = std::max(maxId, task.id);
    }
    nextId_ = maxId + 1;
}