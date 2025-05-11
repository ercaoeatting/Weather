#include "logtime.h"
#include <iostream>
using namespace idc;

int main()
{
    FormatTime now = FormatTime::now();

    std::cout << now.format(FormatTime::Format::FULL) << "\n";
    std::cout << now.format(FormatTime::Format::DATE_ONLY) << "\n";
    std::cout << now.format(FormatTime::Format::TIME_ONLY) << "\n";
    std::cout << FormatTime::now().format(FormatTime::Format::COMPACT) << "\n";

    std::cout << FormatTime::now().format<Details::Use>(FormatTime::Format::FULL) << "  // 带小数精度\n";


    std::cout << FormatTime::now().format(FormatTime::Format::FULL) << "\n";
    std::cout << FormatTime::now().addDays(1).format(FormatTime::Format::FULL) << "\n";
}
