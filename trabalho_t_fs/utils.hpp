#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

template <typename T>
void split(const std::string &s, char delim, T result);
std::vector<std::string> split(const std::string &s, char delim);

#endif /* UTILS_HPP */