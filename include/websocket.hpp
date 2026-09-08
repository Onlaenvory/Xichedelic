#pragma once

#include <openssl/ssl.h>
#include <format>
#include <string>
#include <string_view>
#include <thread>

namespace XI {
  class WebSocket {
    public:
      WebSocket() : m_ssl_ctx(SSL_CTX_new(TLS_client_method())) {}
      ~WebSocket() {
        Close();
        if (m_ssl_ctx) {
          SSL_CTX_free(m_ssl_ctx);
          m_ssl_ctx = nullptr;
        }
      }

    private:
      const char* m_host = "data-stream.binance.vision";
      const char* m_port = "443";
      std::string_view  m_currency;

      int m_socket_fd = -1;

      SSL_CTX* m_ssl_ctx = nullptr;
      SSL* m_ssl = nullptr;

      std::string m_path = std::format("/ws/{}@aggTrade", m_currency);
      std::thread m_session;
      std::atomic<bool> m_state{false};
    private:
      std::string GenerateHandshakeNonce();
      bool PerformTLSHandshake();
      bool PerformWebSocketHandshake();
      void Close();
      void SentRequest();
      void Listen();
    public:
      void Connect(std::string_view currency);
  };
} // namespace XI
