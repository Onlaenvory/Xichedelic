#include "websocket.hpp"

int main() {
  XI::WebSocket xichedelic("btcusdt");

  xichedelic.StartSession();
  xichedelic.EstablishConnection();
  return 0;
}
