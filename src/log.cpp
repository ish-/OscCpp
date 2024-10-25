#include <iostream>

bool _LOG = true;
template <typename T1, typename T2 = const char*>
void _log(T1 str1, T2 str2 = "") {
    if (_LOG)
        std::cout << "\x1b[31m" << str1 << " " << str2 << "\x1b[0m\n";
}
