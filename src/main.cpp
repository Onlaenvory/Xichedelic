#include <sys/socket.h>
#include <netdb.h>
#include "spdlog/spdlog.h"

const char *url = "wss://stream.binance.com:9443/ws/bnbusd@aggTrade";
const char *host = "stream.binance.com";
const char *port = "9443";

int main() {
  addrinfo SY;
  addrinfo *receiver = nullptr;

  SY.ai_flags = AI_PASSIVE;
  SY.ai_socktype = SOCK_STREAM;
  SY.ai_family = AF_INET;

  int status = getaddrinfo(host, port, &SY, &receiver);

  spdlog::info("!");
  spdlog::warn("!");
  spdlog::error("!");

  return 0;
}
