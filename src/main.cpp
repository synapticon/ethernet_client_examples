#include "ethernet_client/ethernet_client.h"

#include <iostream>

const std::string kIp = "192.168.100.5";
const int kPort = 8080;

int main() {
  std::cout << "Creating EthernetDevice with IP: " << kIp
            << " and Port: " << kPort << std::endl;

  auto ed = EthernetDevice{kIp, kPort};

  std::cout << "EthernetDevice created." << std::endl;

  std::cout << "Checking if device is connected: "
            << (ed.isConnected() ? "Yes" : "No") << std::endl;

  assert(ed.isConnected() == false);

  std::cout << "Attempting to connect..." << std::endl;

  ed.connect();

  std::cout << "Connection attempt finished." << std::endl;

  assert(ed.isConnected() == true);

  std::cout << "Connected to the device." << std::endl;

  return 0;
}
