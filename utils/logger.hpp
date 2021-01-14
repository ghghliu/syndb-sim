#ifndef LOGGER_H
#define LOGGER_H

#include <string>

void log_debug(const std::string &msg);
void log_error(const std::string &msg);
void log_info(const std::string &msg);
void log_print_info(const std::string &msg);


#endif
