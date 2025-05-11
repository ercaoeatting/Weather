#pragma once
#include <chrono>
#include <format>
#include <stdexcept>
#include <string>

namespace idc
{
class FormatTime
{
public:
    enum class Format {
        FULL,      // 2025-05-10 15:05:35
        DATE_ONLY, // 2025-05-10
        TIME_ONLY, // 15:05:35
        COMPACT    // 20250510150535
    };
    enum class SecondsDetails {
        Use,
        No
    };

    static FormatTime now()
    {
        return FormatTime(std::chrono::system_clock::now());
    }

    explicit FormatTime(std::chrono::system_clock::time_point tp) : time_(tp) {}

    template <SecondsDetails show_float = SecondsDetails::No>
    std::string format(Format fmt = Format::FULL) const
    {
        auto const &tp = get_time_point<show_float>();
        switch (fmt) {
        case Format::FULL:      return std::format("{:%Y-%m-%d %H:%M:%S}", tp);
        case Format::DATE_ONLY: return std::format("{:%Y-%m-%d}", tp);
        case Format::TIME_ONLY: return std::format("{:%H:%M:%S}", tp);
        case Format::COMPACT:   return std::format("{:%Y%m%d%H%M%S}", tp);
        default:                throw std::invalid_argument("Invalid format enum");
        }
    }

    FormatTime &addDays(int days)
    {
        time_ += std::chrono::hours(days * 24);
        return *this;
    }

    FormatTime &addHours(int hours)
    {
        time_ += std::chrono::hours(hours);
        return *this;
    }

    FormatTime &addMinutes(int minutes)
    {
        time_ += std::chrono::minutes(minutes);
        return *this;
    }

    FormatTime &addSeconds(int seconds)
    {
        time_ += std::chrono::seconds(seconds);
        return *this;
    }
    FormatTime &addMilliseconds(int milliseconds)
    {
        time_ += std::chrono::milliseconds(milliseconds);
        return *this;
    }
    FormatTime &addMicroseconds(int microseconds)
    {
        time_ += std::chrono::microseconds(microseconds);
        return *this;
    }
    

private:
    std::chrono::system_clock::time_point time_;

    template <SecondsDetails show_float>
    auto get_time_point() const
    {
        if constexpr (show_float == SecondsDetails::Use) {
            return time_;
        } else {
            return std::chrono::floor<std::chrono::seconds>(time_);
        }
    }
};

using Details = FormatTime::SecondsDetails;
using LogTimeFormat = FormatTime::Format;

} // namespace idc
