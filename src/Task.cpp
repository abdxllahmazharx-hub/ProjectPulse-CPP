#include "Task.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

using namespace std::chrono;

std::string to_string(Priority priority) {
    switch (priority) {
        case Priority::Low: return "Low";
        case Priority::Medium: return "Medium";
        case Priority::High: return "High";
    }
    return "Unknown";
}

std::string to_string(Status status) {
    switch (status) {
        case Status::Open: return "Open";
        case Status::InProgress: return "In Progress";
        case Status::Done: return "Done";
    }
    return "Unknown";
}

std::optional<Priority> priority_from_int(int value) {
    switch (value) {
        case 1: return Priority::Low;
        case 2: return Priority::Medium;
        case 3: return Priority::High;
        default: return std::nullopt;
    }
}

std::optional<Status> status_from_int(int value) {
    switch (value) {
        case 0: return Status::Open;
        case 1: return Status::InProgress;
        case 2: return Status::Done;
        default: return std::nullopt;
    }
}

std::string format_date(sys_days date) {
    const auto ymd = year_month_day{date};
    std::ostringstream out;
    out << static_cast<int>(ymd.year()) << '-'
        << std::setw(2) << std::setfill('0') << unsigned(ymd.month()) << '-'
        << std::setw(2) << std::setfill('0') << unsigned(ymd.day());
    return out.str();
}

std::optional<sys_days> parse_date(const std::string& value) {
    int yr{}, mo{}, dy{};
    char dash1{}, dash2{};
    std::istringstream in(value);
    if (!(in >> yr >> dash1 >> mo >> dash2 >> dy) || dash1 != '-' || dash2 != '-') {
        return std::nullopt;
    }

    if (mo < 1 || mo > 12 || dy < 1 || dy > 31) {
        return std::nullopt;
    }

    const year_month_day ymd{year{yr} / month{static_cast<unsigned>(mo)} / day{static_cast<unsigned>(dy)}};
    if (!ymd.ok()) {
        return std::nullopt;
    }

    return sys_days{ymd};
}

std::string escape_field(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (char c : value) {
        if (c == '\\' || c == '|' || c == '\n') {
            out.push_back('\\');
        }
        out.push_back(c);
    }
    return out;
}

std::string unescape_field(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    bool escaped = false;
    for (char c : value) {
        if (escaped) {
            out.push_back(c);
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else {
            out.push_back(c);
        }
    }
    return out;
}