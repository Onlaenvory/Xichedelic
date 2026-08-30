#include "spdlog/spdlog.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

// const char *url = "wss://stream.binance.com:9443/ws/bnbusd@aggTrade";
constexpr const char* host = "stream.binance.com";
constexpr const char* port = "9443";

int main() {
  addrinfo SY{};
  SY.ai_family = AF_INET;
  SY.ai_socktype = SOCK_STREAM;
  SY.ai_protocol = IPPROTO_TCP; // Optional : SOCK_STREAM does select TCP automatically

  addrinfo* node = nullptr;
  int status = getaddrinfo(host, port, &SY, &node);

  if (status != 0) {
    spdlog::error("Invalid getaddrinfo : {}", gai_strerror(status));
  };

  spdlog::info("Valid getaddrinfo");

  for(addrinfo* current_addr = node; current_addr != nullptr; current_addr->ai_next) {
    int sock_fdesc = socket(current_addr->ai_family, current_addr->ai_socktype, current_addr->ai_protocol);
    spdlog::info("Socket Created");

    if (sock_fdesc == -1) {
      spdlog::info("Incompatible address, Hoping address");
      continue;
    }

    spdlog::info("Compatible address found, Connecting session");
    if (connect(sock_fdesc, current_addr->ai_addr, current_addr->ai_addrlen)) {
      spdlog::info("Connected Succefully");
      break;
    }

    close(sock_fdesc);
  }

  return 0;
}
