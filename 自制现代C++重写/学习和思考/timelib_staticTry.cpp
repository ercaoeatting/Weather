#include <chrono>
#include <format>
#include <iostream>
// c++14
// template <typename T>
// static constexpr bool dependent_false = false;

// c++17
template <auto>
static constexpr bool dependent_false = false;

class LogTime
{
public:
    enum class Format {
        FULL,      // 2025-05-10 15:05:35
        DATE_ONLY, // 2025-05-10
        TIME_ONLY, // 15:05:35
        COMPACT    // 20250510150535
    };

    static LogTime now()
    {
        return LogTime(std::chrono::system_clock::now());
    }

    explicit LogTime(std::chrono::system_clock::time_point tp) : time_(tp) {}

    template <Format fmt = Format::FULL, bool show_float = false>
    [[nodiscard]] std::string format() const
    {
        // 正确写法：用类型分发替代lambda
        auto const &tp = get_time_point<show_float>();

        if constexpr (fmt == Format::FULL) {
            return std::format("{:%Y-%m-%d %H:%M:%S}", tp);
        } else if constexpr (fmt == Format::DATE_ONLY) {
            return std::format("{:%Y-%m-%d}", tp);
        } else if constexpr (fmt == Format::TIME_ONLY) {
            return std::format("{:%H:%M:%S}", tp);
        } else if constexpr (fmt == Format::COMPACT) {
            return std::format("{:%Y%m%d%H%M%S}", tp);
        } else {
            // c++14
            // static_assert(dependent_false<std::integral_constant<Format, fmt>>, "Invalid Format");
            // c++17
            static_assert(dependent_false<fmt>, "Invalid Format");
        }
    }

    std::string format(Format fmt, bool show_float = false) const
    {
        auto const &tp = show_float ? time_ : std::chrono::floor<std::chrono::seconds>(time_);
        switch (fmt) {
        case Format::FULL:      return std::format("{:%Y-%m-%d %H:%M:%S}", tp);
        case Format::DATE_ONLY: return std::format("{:%Y-%m-%d}", tp);
        case Format::TIME_ONLY: return std::format("{:%H:%M:%S}", tp);
        case Format::COMPACT:   return std::format("{:%Y%m%d%H%M%S}", tp);
        default:                throw std::invalid_argument("Invalid format enum");
        }
    }

private:
    std::chrono::system_clock::time_point time_;

    template <bool show_float>
    auto get_time_point() const
    {
        if constexpr (show_float) {
            return time_;
        } else {
            return std::chrono::floor<std::chrono::seconds>(time_);
        }
    }

    // c++11
    // template <class T>
    // struct dependent_false : std::false_type {
    // };
};

int main()
{
    auto now = LogTime::now();
    
    // 编译期确定格式和精度
    std::cout << now.format<LogTime::Format::FULL>() << std::endl;      // 默认不带小数: "20250518024940"
    std::cout << now.format<LogTime::Format::TIME_ONLY>() << std::endl; // 默认不带小数: "20250518024940"
    std::cout << now.format<LogTime::Format::DATE_ONLY>() << std::endl; // 默认不带小数: "20250518024940"
    std::cout << now.format<LogTime::Format::COMPACT>() << std::endl;   // 默认不带小数: "20250518024940"

    // 快捷方法
    std::string full_precision = now.format<LogTime::Format::FULL, true>();
}
