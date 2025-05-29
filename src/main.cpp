#include "ethernet_client.h"

#include <cassert>
#include <string>

#include "loguru.h"

const std::string kIp = "192.168.100.5";
const int kPort = 8080;

int main() {
  auto ed = EthernetDevice{kIp, kPort};

  // Check if the socket is not connected
  assert(ed.isConnected() == false);

  LOG_F(INFO, "Connecting to %s:%d...", kIp.c_str(), kPort);
  auto connected = ed.connect();
  LOG_F(INFO, "done.");

  // Set the state to OP (0x08)
  ed.setState(0x08);

  // Check if the socket is now connected
  assert(connected == true);
  assert(ed.isConnected() == true);

  LOG_F(INFO, "Getting state...");
  auto state = ed.getState();

  LOG_F(INFO, "done. State: %d", state);

  // Expecting state to be OP (0x08)
  assert(state == 0x08);

  LOG_F(INFO, "Loading parameters and reading their values...");
  ed.loadParameters(true);
  LOG_F(INFO, "done. Number of parameters: %zu", ed.getParameters().size());

  auto productCode = ed.findParameter(0x1018, 0x02).getValue<std::uint32_t>();
  LOG_F(INFO, "Product Code: 0x%08X", productCode);

  auto manufacturerSoftwareVersion =
      ed.findParameter(0x100A, 0x00).getValue<std::string>();
  LOG_F(INFO, "Manufacturer Software Version: %s",
        manufacturerSoftwareVersion.c_str());

  LOG_F(INFO, "Reading .hardware_description file...");
  auto hardwareDescriptionBuffer =
      ed.readFile(".hardware_description", std::chrono::seconds(5));
  LOG_F(INFO, "done. Read %zu bytes from .hardware_description file",
        hardwareDescriptionBuffer.size());

  std::string hardwareDescription = std::string(
      hardwareDescriptionBuffer.begin(), hardwareDescriptionBuffer.end());

  LOG_F(INFO, "%s", hardwareDescription.c_str());

  LOG_F(INFO, "Disconnecting from %s:%d...", kIp.c_str(), kPort);
  ed.disconnect();
  LOG_F(INFO, "done.");
}
