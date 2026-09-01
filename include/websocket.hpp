#pragma once

#include <openssl/ssl.h>
#include <format>
#include <string>

namespace XI {
  class WebSocket {
    public:
      WebSocket(std::string_view symbol) : m_path(std::format("/ws/{}@aggTrade", symbol)) {}

      ~WebSocket() {
        Shutdown();
        if (m_ssl_ctx) {
          SSL_CTX_free(m_ssl_ctx);
          m_ssl_ctx = nullptr;
        }
      }

    private:
      const char* m_host = "data-stream.binance.vision";
      const char* m_port = "443";

      int m_socket_fd = -1;

      SSL_CTX* m_ssl_ctx = SSL_CTX_new(TLS_client_method());
      SSL* m_ssl = nullptr;

      std::string m_path;

    private:
      std::string GenerateHandshakeNonce();
      bool PerformTLSHandshake();
      bool PerformWebSocketHandshake();
      void Shutdown();

    public:
      bool StartSession();
  };
} // namespace XI
