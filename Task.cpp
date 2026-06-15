#pragma once

#include <chrono>
#include <optional>
#include <string>

enum class Priority {
    Low = 1,
    Medium = 2,
    High = 3
};

enum class Status {
    Open = 0,
    InProgress = 1,
    Done = 2
};

struct Task {
    int id{};
    std::string title;
    std::string description;
    std::chrono::sys_days dueDate{};
    Priority priority{Priority::Medium};
    Status status{Status::Open};
    std::chrono::system_clock::time_point createdAt{};
};

std::string to_string(Priority priority);
std::string to_string(Status status);

std::optional<Priority> priority_from_int(int value);
std::optional<Status> status_from_int(int value);

std::string format_date(std::chrono::sys_days date);
std::optional<std::chrono::sys_days> parse_date(const std::string& value);

std::string escape_field(const std::string& value);
std::string unescape_field(const std::string& value);