#include <iostream>

#include "logtime.h"

int main() {
    using namespace idc;

    FormatTime now = FormatTime::now();
    std::cout << FormatTime::now().format() << "\n\n\n";
    std::cout << now.format(FormatTime::Format::FULL) << "\n";
    std::cout << now.format(FormatTime::Format::DATE_ONLY) << "\n";
    std::cout << now.format(FormatTime::Format::TIME_ONLY) << "\n";
    std::cout << FormatTime::now().format(FormatTime::Format::COMPACT) << "\n";

    std::cout << FormatTime::now().format<Details::Use>(FormatTime::Format::FULL) << "  // 带小数精度\n";

    std::cout << FormatTime::now().format(FormatTime::Format::FULL) << "\n";
    std::cout << FormatTime::now().addDays(2).format(FormatTime::Format::FULL) << "\n";
}
