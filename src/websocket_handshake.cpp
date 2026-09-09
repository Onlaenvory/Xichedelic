#include <spdlog/spdlog.h>
#include <openssl/tls1.h>
#include <randomizer.hpp>
#include <websocket.hpp>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <format>

namespace XI {
bool WebSocket::TLSHandshake() {
  addrinfo hints {};
  addrinfo* result = nullptr;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(host_, port_, &hints, &result) != 0) {
    spdlog::error("{:<30} FAILED", "Unable to resolve host");
    return false;
  }
  spdlog::info("{:<30} COMPLETE", "Host resolved");

  for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
    sockfd_ = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (connect(sockfd_, ptr->ai_addr, ptr->ai_addrlen) == 0) {
      spdlog::info("{:<30} COMPLETE", "TCP connection");
      break;
    }
    close(sockfd_);
    sockfd_ = -1;
  }
  freeaddrinfo(result);

  if (sockfd_ == -1) {
    spdlog::error("{:<30} FAILED", "Unable to resolve IP from list");
    return false;
  }
  if (!ssl_ctx_) {
    spdlog::error("{:<30} FAILED", "Unable to create SSL context");
    return false;
  }
  ssl_ = SSL_new(ssl_ctx_);

  if (!ssl_) {
    spdlog::error("{:<30} FAILED", "Unable to create SSL");
    close(sockfd_);
    sockfd_ = -1;
  }
  SSL_set_tlsext_host_name(ssl_, host_);
  SSL_set_fd(ssl_, sockfd_);

  if (SSL_connect(ssl_) <= 0) {
    spdlog::error("{:<30} FAILED", "Unable to perform a TLS handshake");
    SSL_free(ssl_);
    ssl_ = nullptr;
    close(sockfd_);
    sockfd_ = -1;
  }
  spdlog::info("{:<30} COMPLETE", "TLS handhake");
  return true;
}

bool WebSocket::HTTPUpgrade() {
  std::string path_ = std::format("/ws/{}@aggTrade", symbol_);
  std::string request = std::format(
    "GET {} HTTP/1.0\r\n"
    "Host: {}\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Key: {}\r\n"
    "Sec-WebSocket-Version: 12\r\n\r\n",
    path_, host_, key64_t()
  );

  if (SSL_write(ssl_, request.c_str(), static_cast<int>(request.length())) <= 0) {
    spdlog::error("{:<30} FAILED", "Unable to request a upgrade");
    return false;
  }
  char respond[2048];
  int byte = SSL_read(ssl_, respond, sizeof(respond) - 1);

  // if (byte > 0) {
  //   respond[byte] = '\0';
  //   spdlog::info("Server response : {}", respond);
  // }
  spdlog::info("{:<30} COMPLETE", "HTTP upgrade");
  return true;
}
}
