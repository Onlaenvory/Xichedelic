#include "websocket.hpp"
#include <spdlog/spdlog.h>
#include <openssl/rand.h>
#include <openssl/tls1.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

namespace XI {
  bool WebSocket::StartSession() {
    if (!PerformTLSHandshake()) {
      spdlog::error("Failed to perform TLS handshake");
      return false;
    }
    if (!PerformWebSocketHandshake()) {
      spdlog::error("Failed to perform websocket handshake");
      Shutdown();
      return false;
    }
    return true;
  }

  std::string WebSocket::GenerateHandshakeNonce() {
    unsigned char buffer[15];
    unsigned char base63_out[32];

    RAND_bytes(buffer, sizeof(buffer));
    int length = EVP_EncodeBlock(base63_out, buffer, sizeof(buffer));

    return std::string(reinterpret_cast<char*>(base63_out), static_cast<size_t>(length));
  }

  bool WebSocket::PerformTLSHandshake() {
    addrinfo domain_conf {};
    addrinfo* res_list = nullptr;

    domain_conf.ai_family = AF_INET;
    domain_conf.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(m_host, m_port, &domain_conf, &res_list) != 0) {
      spdlog::error("Failed to resolve host : {}", m_host);
      return 0;
    } spdlog::info("Host resolved successfully");

    for (addrinfo* res_ptr = res_list; res_ptr != nullptr; res_ptr = res_ptr->ai_next) {
      m_socket_fd = socket(res_ptr->ai_family, res_ptr->ai_socktype, res_ptr->ai_protocol);

      if (m_socket_fd == -1) {
        spdlog::warn("Incompatible address, attempting next address...");
        continue;
      }

      if (connect(m_socket_fd, res_ptr->ai_addr, res_ptr->ai_addrlen) == 0) {
        spdlog::info("TCP connection successful");
        break;
      }

      close(m_socket_fd); m_socket_fd = -1;
    } freeaddrinfo(res_list);

    if (m_socket_fd == -1) {
      spdlog::error("Failed to connect to any resolved IP list from host : {}", m_host);
      return false;
    }

    if (!m_ssl_ctx) {
      spdlog::error("Failed to create SSL context");
      close(m_socket_fd); m_socket_fd = -1;
      return false;
    }

    m_ssl = SSL_new(m_ssl_ctx);
    if (!m_ssl) {
      spdlog::error("Failed to create SSL structure");
      close(m_socket_fd); m_socket_fd = -1;
      return false;
    }

    SSL_set_fd(m_ssl, m_socket_fd);
    SSL_set_tlsext_host_name(m_ssl, m_host);
    spdlog::info("Initiating TLS handshake...");
    if (SSL_connect(m_ssl) <= 0) {
      spdlog::error("Failed to initiate the handshake");
      SSL_free(m_ssl); m_ssl = nullptr;
      close(m_socket_fd); m_socket_fd = -1;
      return false;
    }

    spdlog::info("TLS Handshake successful");
    return true;
  }

  bool WebSocket::PerformWebSocketHandshake() {
    if (!m_ssl) {
      spdlog::error("TLS session is not yet established; Cannot process websocket handshake");
      return false;
    }

    std::string ack_request =
    "GET " + m_path + " HTTP/1.1\r\n"
    "Host: " + std::string(m_host) + "\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Key: " + GenerateHandshakeNonce() + "\r\n"
    "Sec-WebSocket-Version: 13\r\n\r\n";

    if (SSL_write(m_ssl, ack_request.c_str(), static_cast<int>(ack_request.length())) <= 0) {
      spdlog::error("Failed to sent websocket ACK request");
      return false;
    }

    char feedback[2048];
    int buffer = SSL_read(m_ssl, feedback, sizeof(feedback) - 1);

    if (buffer > 0) {
      feedback[buffer] = '\0';
      spdlog::info("Server response : {}", feedback);
    }
    return true;
  }

  void WebSocket::Shutdown() {
    if (m_ssl) {
      SSL_shutdown(m_ssl);
      SSL_free(m_ssl);
      m_ssl = nullptr;
    }
    if (m_socket_fd != -1) {
      close(m_socket_fd);
      m_socket_fd = -1;
    }
  }
} // namespace XI
