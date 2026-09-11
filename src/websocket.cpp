#include <spdlog/spdlog.h>
#include <websocket.hpp>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <string_view>
#include <opcode.hpp>
#include <frame.hpp>
#include <unistd.h>
#include <cstdlib>
#include <key.hpp>
#include <cstdint>
#include <netdb.h>
#include <vector>
#include <format>

namespace XI {
void WebSocket::Connect(std::string_view currency) {
  symbol_ = currency;

  if (!TLSHandshake()) {
    return;
  }
  if (!HTTPUpgrade()) {
    return;
  }
  SentRequest();

  state_ = true;
  Listen();
}
void WebSocket::SentRequest() {
  std::string request = std::format(R"({{"method":"SUBSCRIBE","params":["{}@aggTrade"],"id":1}})", symbol_);
  std::vector<uint8_t> payload = { request.begin(), request.end() };

  HeaderFrame Frame;
  Frame.SetFin(true);
  Frame.SetOpcode(Opcode::Text);
  Frame.SetMask(true);
  Frame.SetPayloadSize(payload.size());
  Frame.SetMaskKey(Key32());

  MaskPayload(payload.data(), payload.size(), Frame.MaskKey());

  SSL_write(ssl_, Frame.byte, Frame.headerSize);
  SSL_write(ssl_, payload.data(), payload.size());
  spdlog::info("{:<30} COMPLETE", "SSL write");
}
void WebSocket::Listen() {
  uint8_t buffer[4096];

  while (state_) {
    int byte = SSL_read(ssl_, buffer, sizeof(buffer));

    if (byte <= 0) {
      spdlog::error("SSL disconnected");
      break;
    }
    size_t headerSize = 2;
    uint8_t opcode = buffer[0] & 0x0F;
    uint8_t length = buffer[1] & 0x7F;
    uint64_t payloadLength = length;

    if (length == 126) {
      payloadLength = (static_cast<uint64_t>(static_cast<uint8_t>(buffer[2]) << 8) | static_cast<uint8_t>(buffer[3]));
      headerSize += 2;
    }
    else if (length == 127) {
      for (int i = 2; i <= 9; i++) {
        payloadLength |= (static_cast<uint64_t>(static_cast<uint8_t>(buffer[i]) << ((9 - i) * 8)));
      }
      headerSize += 8;
    }

    if (opcode == static_cast<uint8_t>(Opcode::Text)) {
      std::string_view data(reinterpret_cast<char*>(&buffer[headerSize]), payloadLength);

      spdlog::info(data);
    }
    else if (opcode == static_cast<uint8_t>(Opcode::Ping)) {
      std::vector<uint8_t> respond;
      respond.reserve(10 + payloadLength);
      respond.push_back(0x8A);

      if (payloadLength <= 125) {
        respond.push_back(static_cast<uint8_t>(payloadLength));
      }
      else if (payloadLength <= 65535) {
        respond.push_back(0x7E);
        respond.push_back((payloadLength >> 8) & 0xFF);
        respond.push_back(payloadLength & 0xFF);
      }
      else {
        respond.push_back(0x7F);
        for (int i = 0; i <= 7; i++) {
          respond.push_back(static_cast<uint8_t>((payloadLength) >> ((7 - i) * 8)) & 0xFF);
        }
      }

      respond.insert(respond.end(), reinterpret_cast<uint8_t *>(&buffer[headerSize]), (reinterpret_cast<uint8_t *>(&buffer[headerSize]) + payloadLength));
      SSL_write(ssl_, respond.data(), respond.size());
    }
  }
}
void WebSocket::Close() {
  state_ = false;

  if (ssl_) {
    SSL_shutdown(ssl_);
  }

  if (sockfd_ != -1) {
    shutdown(sockfd_, SHUT_RDWR);
  }

  if (session_.joinable()) {
    session_.join();
  }

  if (ssl_) {
    SSL_free(ssl_);
    ssl_ = nullptr;
  }

  if (sockfd_ != -1) {
    close(sockfd_);
    sockfd_ = -1;
  }
}
} // namespace XI
