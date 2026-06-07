#ifndef OASIS_LOGINCLIF_HPP
#define OASIS_LOGINCLIF_HPP

#include "../common/socket.hpp" // Ajustado e corrigido o caminho para a raiz comum!

// Mantém a assinatura nativa de 2 parâmetros para o motor de sockets aceitar
int parse_login(int fd, SessionData& session);

#endif // OASIS_LOGINCLIF_HPP