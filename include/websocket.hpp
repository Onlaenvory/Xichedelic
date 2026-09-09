#pragma once

#include <openssl/ssl.h>
#include <string_view>
#include <thread>

namespace XI {
class WebSocket {
public:
  WebSocket() : ssl_ctx_(SSL_CTX_new(TLS_client_method())) {}
  ~WebSocket() {
    Close();
    if (ssl_ctx_) {
      SSL_CTX_free(ssl_ctx_);
      ssl_ctx_ = nullptr;
    }
  }

private:
  const char *host_ = "data-stream.binance.vision";
  const char *port_ = "443";
  std::string_view symbol_;

  int sockfd_ = -1;
  SSL_CTX *ssl_ctx_ = nullptr;
  SSL *ssl_ = nullptr;

  std::thread session_;
  std::atomic<bool> state_ {false};

private:
  bool TLSHandshake();
  bool HTTPUpgrade();
  void SentRequest();
  void Listen();
  void Close();

public:
  void Connect(std::string_view currency);
};
} // namespace XI
