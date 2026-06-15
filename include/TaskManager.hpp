#pragma once

#include "Task.hpp"

#include <filesystem>
#include <string>
#include <vector>

class TaskManager {
public:
    explicit TaskManager(std::filesystem::path storagePath);

    void load();
    void save() const;

    int addTask(std::string title,
                std::string description,
                std::chrono::sys_days dueDate,
                Priority priority);

    bool editTask(int id,
                  const std::string& newTitle,
                  const std::string& newDescription,
                  std::chrono::sys_days newDueDate,
                  Priority newPriority,
                  Status newStatus);

    bool markDone(int id);
    bool removeTask(int id);

    Task* findById(int id);
    const Task* findById(int id) const;

    std::vector<Task> search(const std::string& query) const;
    std::vector<Task> listAll() const;
    std::vector<Task> listByStatus(Status status) const;
    std::vector<Task> sortedByDueDate() const;
    std::vector<Task> sortedByPriority() const;

    std::size_t size() const noexcept { return tasks_.size(); }

private:
    std::filesystem::path storagePath_;
    std::vector<Task> tasks_;
    int nextId_{1};

    void reindexNextId();
};