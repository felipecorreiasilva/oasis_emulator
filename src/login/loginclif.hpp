#ifndef OASIS_LOGINCLIF_HPP
#define OASIS_LOGINCLIF_HPP

#include "../common/socket.hpp"

// Declara a função para que o login.cpp consiga enxergá-la
int parse_login(int fd, SessionData& session);

#endif // OASIS_LOGINCLIF_HPP