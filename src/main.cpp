#include <iostream>
#include <websocket.hpp>

int main() {
  XI::WebSocket xichedelic;

  if (!xichedelic.Connect("btcusdt")) return 1;

  std::cin.get();
  return 0;
}
