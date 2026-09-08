#include "websocket.hpp"
#include "randomizer.hpp"
#include "opcode.hpp"
#include <cstdint>
#include <cstdlib>
#include <format>
#include <frame.hpp>

#include <spdlog/spdlog.h>
#include <openssl/rand.h>
#include <openssl/tls1.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <vector>

namespace XI {
  void WebSocket::Connect(std::string_view currency) {
    m_currency = currency;

    if (!PerformTLSHandshake()) {
      spdlog::error("Failed to perform TLS handshake");
      Close();
    }
    if (!PerformWebSocketHandshake()) {
      spdlog::error("Failed to perform websocket handshake");
      Close();
    }

    SentRequest();

    m_state = true;
    Listen();
  }

  std::string WebSocket::GenerateHandshakeNonce() {
    unsigned char buffer[16];
    unsigned char base64_out[32];

    RAND_bytes(buffer, sizeof(buffer));
    int length = EVP_EncodeBlock(base64_out, buffer, sizeof(buffer));

    return std::string(reinterpret_cast<char*>(base64_out), static_cast<size_t>(length));
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
    spdlog::info("Successfully established the conenct to {}:{}{}", m_host,m_port,m_path);
    return true;
  }

  void WebSocket::SentRequest() {
    std::string request = std::format(R"({{"method":"SUBSCRIBE","params":["{}@aggTrade"],"id":1}})", m_currency);
    std::vector<uint8_t> payload = {request.begin(), request.end()};

    XI::HeaderFrame TEST;
    TEST.SetFin(true);
    TEST.SetOpcode(Opcode::Text);
    TEST.SetMask(true);
    TEST.SetPayloadSize(payload.size());
    TEST.SetMaskKey(XI::Randomizer::key32_t());

    XI::MaskPayload(payload.data(), payload.size(), TEST.MaskKey());

    SSL_write(m_ssl, TEST.byte, TEST.headerSize);
    SSL_write(m_ssl, payload.data(), payload.size());
    spdlog::info("Finishing SSL write");
  }

  void WebSocket::Listen() {
    uint8_t receiveBuffer[4096];

    while (m_state) {
      int readByte = SSL_read(m_ssl, receiveBuffer, sizeof(receiveBuffer));

      if (readByte <= 0) {
        spdlog::error("SSL disconnected");
        break;
      }

      uint8_t opcode = receiveBuffer[0] & 0x0F;
      uint8_t length = receiveBuffer[1] & 0x7F;
      size_t headerByteSize = 2;
      uint64_t payloadLength = length;

      if (length == 126) {
        payloadLength = (static_cast<uint64_t>(receiveBuffer[2]) << 8) | receiveBuffer[3];
        headerByteSize += 2;
      }
      else if (length == 127) {
        payloadLength = (
          static_cast<uint64_t>(receiveBuffer[2]) << 56 |
          static_cast<uint64_t>(receiveBuffer[3]) << 48 |
          static_cast<uint64_t>(receiveBuffer[4]) << 40 |
          static_cast<uint64_t>(receiveBuffer[5]) << 32 |
          static_cast<uint64_t>(receiveBuffer[6]) << 24 |
          static_cast<uint64_t>(receiveBuffer[7]) << 16 |
          static_cast<uint64_t>(receiveBuffer[8]) << 8  |
          static_cast<uint64_t>(receiveBuffer[9]));
        headerByteSize += 8;
      }

      if (opcode == static_cast<uint8_t>(Opcode::Text)) {
        std::string_view data(reinterpret_cast<char*>(&receiveBuffer[headerByteSize]), payloadLength);

        spdlog::info(data);
      }
      else if (opcode == static_cast<uint8_t>(Opcode::Ping)) {
        std::vector<uint8_t> respondFrame;
        respondFrame.reserve(10 + payloadLength);
        respondFrame.push_back(0x8A);

        if (payloadLength <= 125) {
          respondFrame.push_back(static_cast<uint8_t>(payloadLength));
        }
        else if (payloadLength <= 65535) {
          respondFrame.push_back(0x7E);
          respondFrame.push_back((payloadLength >> 8) & 0xFF);
          respondFrame.push_back(payloadLength & 0xFF);
        }
        else {
          respondFrame.push_back(0x7F);
          respondFrame.push_back((payloadLength >> 56) & 0xFF);
          respondFrame.push_back((payloadLength >> 48) & 0xFF);
          respondFrame.push_back((payloadLength >> 40) & 0xFF);
          respondFrame.push_back((payloadLength >> 32) & 0xFF);
          respondFrame.push_back((payloadLength >> 24) & 0xFF);
          respondFrame.push_back((payloadLength >> 16) & 0xFF);
          respondFrame.push_back((payloadLength >> 8) & 0xFF);
          respondFrame.push_back(payloadLength & 0xFF);
        }

        respondFrame.insert(respondFrame.end(), reinterpret_cast<uint8_t*>(&receiveBuffer[headerByteSize]), (reinterpret_cast<uint8_t*>(&receiveBuffer[headerByteSize]) + payloadLength));
        SSL_write(m_ssl, respondFrame.data(), respondFrame.size());
      }
    }
  }

  void WebSocket::Close() {
    m_state = false;

    if (m_ssl) {
      SSL_shutdown(m_ssl);
    }

    if (m_socket_fd != -1) {
      shutdown(m_socket_fd, SHUT_RDWR);
    }

    if (m_session.joinable()) {
      m_session.join();
    }

    if (m_ssl) {
      SSL_free(m_ssl);
      m_ssl = nullptr;
    }

    if (m_socket_fd != -1) {
      close(m_socket_fd);
      m_socket_fd = -1;
    }
  }
} // namespace XI
