#pragma once

#include <boost/asio.hpp>
#include <chrono>
#include <string>

#include "common.h"

/**
 * @brief Enumeration of Ethernet message types used in the protocol.
 *
 * Each message type corresponds to a specific operation or request
 * within the Ethernet communication protocol.
 *
 * @note This is specified in the "Ethernet interface definition" document.
 */
enum class EthernetMessageType : uint8_t {
  SDO_READ = 0x01,   ///< Read a Service Data Object (SDO) value.
  SDO_WRITE = 0x02,  ///< Write a value to a Service Data Object (SDO).
  PDO_RXTX_FRAME =
      0x03,  ///< Transmit or receive a Process Data Object (PDO) frame.
  PDO_CONTROL = 0x04,      ///< Control or configure PDO behavior.
  PDO_MAP = 0x05,          ///< Map PDO entries or configurations.
  FIRMWARE_UPDATE = 0x0B,  ///< Perform firmware update operation.
  FILE_READ = 0x0C,        ///< Read a file from the device.
  FILE_WRITE = 0x0D,       ///< Write a file to the device.
  STATE_CONTROL =
      0x0E,           ///< Control the state of the device (e.g., INIT, PREOP).
  STATE_READ = 0x0F,  ///< Read the current state of the device.
  // PARAM_LIST = 0x10,  ///< Request a list of parameters.
  // PARAM_DESC = 0x11,  ///< Request a description of a parameter.
  // PARAM_SUB_DESC =
  //     0x12,  ///< Request a description of a parameter's sub-entries.
  PARAM_FULL_LIST = 0x13,  ///< Request a full list of parameters.
  SERVER_INFO = 0x20       ///< Request information about the server or device.
};

/**
 * @brief Represents the status of an Ethernet message during transmission or
 * processing.
 *
 * This enum is used to indicate whether a message is complete, part of a
 * segmented sequence, or contains an error. The values are bit flags that may
 * be interpreted by the protocol handler.
 */
enum class EthernetMessageStatus : uint8_t {
  OK = 0x00,      ///< Message is complete and valid.
  FIRST = 0x80,   ///< First segment of a multi-part message.
  MIDDLE = 0xC0,  ///< Middle segment of a multi-part message.
  LAST = 0x40,    ///< Last segment of a multi-part message.
  ERR = 0x28      ///< Message contains an error or failed validation.
};

/**
 * @brief Enum class representing the status codes for an Ethernet SQI reply.
 *
 * This enum class defines the possible reply statuses returned in response to
 * an SQI (Serial Quality Indicator) communication with a System on Chip (SoC).
 */
enum class EthernetSqiReplyStatus : uint8_t {
  BSY = 0x28,  ///< Device is busy and cannot process the request.
  ACK = 0x58,  ///< Acknowledgment received.
  ERR = 0x63   ///< Error occurred during processing.
};

/**
 * @struct EthernetMessage
 * @brief Structure representing a parsed Ethernet response message.
 *
 * This structure is used to hold the parsed information from an Ethernet
 * response message, including its type, sequence ID, status, size, and
 * payload data.
 */
struct EthernetMessage {
  /** The size of the message header. */
  static constexpr size_t kHeaderSize = 7;

  /** The buffer size used for communication, excluding the message header. */
  static constexpr size_t kBufferSize = 1500 - kHeaderSize;

  /**
   * @brief The type of the response message.
   * @details This is a one-byte field representing the type of the response.
   */
  EthernetMessageType type;

  /**
   * @brief The sequence ID of the response message.
   * @details This is a two-byte field that contains the unique sequence ID
   *          of the response message.
   */
  uint16_t id;

  /**
   * @brief The status of the response message.
   * @details This is a two-byte field that contains the status code of the
   *          response message.
   */
  EthernetMessageStatus status;

  /**
   * @brief The status of the SQI reply in the response message.
   *
   * This member stores the SQI reply status, indicating whether the
   * communication with the SoC over SQI was successful or if an error occurred
   * during the exchange. It reflects the outcome of the response message
   * received from the SoC.
   */
  EthernetSqiReplyStatus sqiStatus;

  /**
   * @brief The size of the buffer in the response message.
   * @details This is a two-byte field that specifies the size of the data
   *          buffer in the response message.
   */
  uint16_t size;

  /**
   * @brief The payload data of the response message.
   * @details This vector contains the raw data of the response message,
   *          excluding the header fields.
   */
  std::vector<uint8_t> data;
};

/**
 * @brief Parses a raw Ethernet message buffer into a structured
 * EthernetMessage.
 *
 * This function interprets the first 7 bytes of the buffer as the message
 * header, extracting the type, sequence ID, status, and payload size. If the
 * size field is greater than zero, it also extracts the payload data.
 *
 * @param buffer The raw byte buffer containing the Ethernet message.
 *               Must be at least 7 bytes long to contain the header.
 *
 * @return A structured representation of the parsed Ethernet message.
 *
 * @throws std::runtime_error If the buffer is smaller than the required header
 * size.
 *
 * @see EthernetMessage, EthernetMessageType
 */
EthernetMessage parseEthernetMessage(const std::vector<uint8_t>& buffer);

/**
 * @brief Serializes an EthernetMessage object into a byte buffer.
 *
 * This function converts the provided EthernetMessage into a byte array,
 * which can be sent over a network. The EthernetMessage type, ID, status,
 * size, and any associated data are serialized into the buffer in a specific
 * format expected by the receiving device.
 *
 * @param message The EthernetMessage object to serialize.
 * @return A std::vector<uint8_t> containing the serialized byte buffer.
 *
 * @see EthernetMessage, EthernetMessageType
 */
std::vector<uint8_t> serializeEthernetMessage(const EthernetMessage& message);

/**
 * @class EthernetDevice
 * @brief Handles TCP communication with SOMANET devices over Ethernet.
 *
 * This class provides methods for connecting to a remote server, sending
 * messages, and receiving responses over a TCP connection using Boost.Asio.
 * It manages the underlying socket, connection, and I/O operations required
 * for client-server communication.
 */
class EthernetDevice : common::Device {
 public:
  /**
   * @brief Constructs an EthernetDevice object with the specified IP address
   *        and port.
   *
   * Initializes the client with the target server's IP address and port
   * number.
   *
   * @param ip The IP address of the server to connect to.
   * @param port The port number to use for the connection.
   */
  EthernetDevice(const std::string& ip, unsigned short port);

  /**
   * @brief Destructor for the EthernetDevice class.
   *
   * Closes the socket and disconnects from the server if the socket is open.
   */
  ~EthernetDevice();

  /**
   * @brief Increments the sequence ID atomically and wraps it around at the
   *        maximum value.
   *
   * This function atomically increments the `seq_id` (a 16-bit unsigned
   * integer). When the value reaches the maximum value of `uint16_t` (0xFFFF),
   * it wraps around to 0. It ensures thread safety when used in multithreaded
   * environments.
   *
   * @return The updated sequence ID after incrementing, wrapped around if
   *         necessary.
   */
  uint16_t incrementSeqId();

  /**
   * @brief Establishes a connection to the remote server.
   *
   * Attempts to connect to the specified IP address and port provided during
   * construction. This method blocks until the connection is either
   * successfully established or fails.
   *
   * @return `true` if the connection was successfully established; `false`
   *         otherwise.
   */
  bool connect();

  /**
   * @brief Checks if the Ethernet socket is currently open.
   *
   * Returns true if the underlying socket is open, indicating an active
   * or initialized connection. Returns false otherwise.
   *
   * @return true if the socket is open, false otherwise.
   */
  bool isConnected();

  /**
   * @brief Closes the Ethernet socket connection.
   *
   * This function attempts to close the Ethernet socket. It handles any errors
   * that may occur during the process, logging the result. If the socket is
   * successfully closed, it returns `true`; otherwise, it returns `false`.
   *
   * @return `true` if the socket was closed successfully, `false` if an error
   *         occurred or an exception was thrown during the operation.
   */
  bool disconnect();

  /**
   * @brief Exchanges a message with a remote server and waits for a response
   * with a timeout.
   *
   * This method serializes the request message, sends it to the server using an
   * asynchronous write operation, and waits for a response with a timeout. If
   * the operation takes longer than the specified expiry time, the operation
   * will be canceled and an error will be thrown.
   *
   * @param requestMessage The message to send to the remote server.
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out.
   *
   * @return EthernetMessage The parsed response message from the server.
   *
   * @throws std::runtime_error If the write or read operation fails, or if the
   * operation times out.
   */
  EthernetMessage exchangeWithTimeout(
      const EthernetMessage& request,
      const std::chrono::steady_clock::duration expiryTime);

  /**
   * @brief Sends a request to read the state of the device and returns the
   * state value.
   *
   * This function sends a request to the device using the
   * `EthernetMessageType::STATE_READ` message type. It increments the sequence
   * ID, constructs the request message, serializes it, and sends it to the
   * device. After sending the request, the function waits for the response,
   * parses the response message, and returns the state value as a `uint8_t`.
   *
   * The returned state corresponds to the EtherCAT state machine states,
   * where:
   * - INIT: 1
   * - PREOP: 2
   * - SAFEOP: 4
   * - OP: 8
   * - BOOT: 3
   *
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out. Defaults to 3000 milliseconds.
   *
   * @return The state value of the device as a uint8_t.
   *
   * @throws boost::system::system_error if sending or receiving fails.
   * @throws std::runtime_error if the response buffer is too small to parse.
   */
  uint8_t getState(const std::chrono::steady_clock::duration expiryTime =
                       std::chrono::milliseconds(3000)) override;

  /**
   * @brief Sends a state control command to the Ethernet device and checks the
   *        response.
   *
   * This method constructs a `STATE_CONTROL` type `EthernetMessage` with the
   * given state, sends it over the socket, and waits for a response. It then
   * parses the received message and returns `true` if the operation was
   * acknowledged with `OK` status.
   *
   * @param state The new state to set on the remote device.
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out. Defaults to 3000 milliseconds.
   *
   * @return `true` if the response status is `OK`, `false` otherwise.
   *
   * @throws boost::system::system_error if sending or receiving fails.
   * @throws std::runtime_error if the response buffer is too small to parse.
   */
  bool setState(uint8_t state,
                const std::chrono::steady_clock::duration expiryTime =
                    std::chrono::milliseconds(3000)) override;

  /**
   * @brief Reads the contents of a file over Ethernet.
   *
   * This function sends a series of Ethernet messages to request file data in
   * segments. It uses a loop to send requests and receive responses from a
   * remote server until the entire file is read. The file content is
   * accumulated in a vector and returned.
   *
   * @param filename The name of the file to be read.
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out. Defaults to 3000 milliseconds.
   *
   * @return A `std::vector<uint8_t>` containing the file data received from
   * the remote server.
   *
   * @throws boost::system::system_error if sending or receiving fails.
   * @throws std::runtime_error if the response buffer is too small to parse.
   */
  std::vector<uint8_t> readFile(
      const std::string& filename,
      const std::chrono::steady_clock::duration expiryTime =
          std::chrono::milliseconds(5000)) override;

  /**
   * @brief Reads a list of file names from the device.
   *
   * Sends a request to read the file list using the "fs-getlist" command,
   * waits up to the specified timeout, and parses the result into individual
   * lines. Each line represents a file name. Handles both Unix (`\n`) and
   * Windows (`\r\n`) line endings.
   *
   * @param stripSizeSuffix If true, removes the ", size: <bytes>" suffix from
   * each line. Defaults to true.
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out. Defaults to 3000 milliseconds.
   *
   * @return A vector of strings, where each string is a file name from the
   * response.
   */
  std::vector<std::string> readFileList(
      const bool stripSizeSuffix = true,
      const std::chrono::steady_clock::duration expiryTime =
          std::chrono::milliseconds(3000));

  /**
   * @brief Removes a file from the device.
   *
   * Sends a file removal request via Ethernet by attempting to read
   * the file with a special prefix (`fs-remove=`). It verifies that
   * the response content begins with the expected success message.
   *
   * @param filename The name of the file to be removed.
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out. Defaults to 3000 milliseconds.
   *
   * @return `true` if the file was successfully removed, `false` otherwise.
   */
  bool removeFile(const std::string& filename,
                  const std::chrono::steady_clock::duration expiryTime =
                      std::chrono::milliseconds(3000));

  /**
   * @brief Sends a file in chunks to the remote server via Ethernet.
   *
   * This function splits the file data into smaller chunks and sends them to
   * the server in multiple segments. The file name is sent as the first
   * segment, followed by the file data in subsequent chunks.
   *
   * The server's response to each chunk is parsed, and the next chunk is sent
   * until the entire file is transmitted.
   *
   * @param filename The name of the file to be written.
   * @param data The data to be written to the file.
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out. Defaults to 5000 milliseconds.
   *
   * @return Returns true if the file was successfully written in chunks;
   * otherwise, false.
   *
   * @note This method assumes that the server is expecting the file in segments
   * and will respond accordingly.
   */
  bool writeFile(const std::string& filename, const std::vector<uint8_t>& data,
                 const std::chrono::steady_clock::duration expiryTime =
                     std::chrono::milliseconds(5000)) override;

  /**
   * @brief Sends a firmware update request to the connected Integro device.
   *
   * Constructs and transmits a firmware update request using the custom
   * Ethernet communication protocol. Waits for the device's response and
   * verifies whether the request was successfully acknowledged.
   *
   * This request must be issued after uploading one or both of the following
   * files:
   * - **app_firmware.bin**: Contains the SoC firmware.
   * - **com_firmware.bin**: Contains the communication chip firmware.
   *
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out. Defaults to 2000 milliseconds.
   *
   * @return true if the device responds with an OK status; false otherwise.
   */
  bool triggerFirmwareUpdate(const std::chrono::steady_clock::duration
                                 expiryTime = std::chrono::milliseconds(2000));

  /**
   * Retrieves a list of parameters from the device, potentially updating their
   * values.
   *
   * This function communicates with the device to request and retrieve
   * parameter data in multiple segments. It constructs request messages to
   * retrieve parameter data and processes the received responses, accumulating
   * all parameter data in the `content` buffer. Once all data has been
   * retrieved, it parses the content buffer into individual parameter objects
   * and returns them as a `std::vector<common::Parameter>`.
   *
   * @param readValues If true, the function will attempt to read parameter
   * values from the device. If false, it will use the existing data from the
   * `content` buffer.
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out. Defaults to 1000 milliseconds.
   *
   * @return A vector of parsed `common::Parameter` objects containing the
   * parameters retrieved from the device.
   *
   * @throws boost::system::system_error if sending or receiving fails.
   * @throws std::runtime_error if the response buffer is too small to parse.
   */
  std::vector<common::Parameter> getParameters(
      bool readValues = false,
      const std::chrono::steady_clock::duration expiryTime =
          std::chrono::milliseconds(1000));

  /**
   * @brief Reads an SDO (Service Data Object) from an Ethernet device.
   *
   * This function sends a request to read an SDO at a specified index and
   * subindex, then receives and parses the response. If the response indicates
   * a failure, it logs an error and returns an empty vector. Otherwise, it
   * returns the SDO data.
   *
   * @param index The index of the SDO to read.
   * @param subindex The subindex of the SDO to read.
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out. Defaults to 1000 milliseconds.
   *
   * @return A vector containing the data from the SDO response.
   *         An empty vector is returned if the operation fails.
   *
   * @throws boost::system::system_error if sending or receiving fails.
   * @throws std::runtime_error if the response buffer is too small to parse.
   */
  std::vector<uint8_t> readSdo(
      uint16_t index, uint8_t subindex,
      const std::chrono::steady_clock::duration expiryTime =
          std::chrono::milliseconds(1000));

  /**
   * @brief Writes an SDO (Service Data Object) to an Ethernet device.
   *
   * This function sends a request to write an SDO to a specified index and
   * subindex, along with the provided data. It then receives and parses the
   * response. If the response indicates an error, it logs an error message and
   * returns `false`. Otherwise, it returns `true` to indicate that the write
   * operation was successful.
   *
   * @param index The index of the SDO to write.
   * @param subindex The subindex of the SDO to write.
   * @param value A vector containing the data to be written to the SDO.
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out. Defaults to 1000 milliseconds.
   *
   * @return `true` if the write operation was successful, `false` otherwise.
   *
   * @throws boost::system::system_error if sending or receiving fails.
   * @throws std::runtime_error if the response buffer is too small to parse.
   */
  bool writeSdo(uint16_t index, uint8_t subindex,
                const std::vector<uint8_t>& data,
                const std::chrono::steady_clock::duration expiryTime =
                    std::chrono::milliseconds(1000));

  /**
   * @brief Loads parameters from the device and stores them locally.
   *
   * This function retrieves a list of parameters using the `getParameters`
   * method. It then stores each parameter in a map for later use, keyed by
   * a pair consisting of the parameter's index and subindex.
   *
   * @param readValues If true, the values of the parameters are read from the
   * device; otherwise, only the parameter metadata is retrieved.
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out. Defaults to 9000 milliseconds.
   */
  void loadParameters(bool readValues = false,
                      const std::chrono::steady_clock::duration expiryTime =
                          std::chrono::milliseconds(9000)) override;

  void clearParameters() override;

  common::Parameter& findParameter(uint16_t index, uint8_t subindex) override;

  common::Parameter& upload(
      const uint16_t index, const uint8_t subindex,
      const std::chrono::steady_clock::duration expiryTime =
          std::chrono::milliseconds(3000)) override;

  /**
   * @brief Uploads a parameter from the device and returns its value as the
   * specified type.
   *
   * This templated overload uploads a parameter using the given index and
   * subindex, updates the corresponding parameter in the local store, and
   * returns its value as type `T`.
   *
   * @tparam T The type to extract from the uploaded parameter value.
   * @param index The 16-bit index of the parameter in the object dictionary.
   * @param subindex The 8-bit subindex of the parameter.
   * @param expiryTime The maximum duration to wait for the SDO upload
   * operation. Defaults to 5000 milliseconds.
   *
   * @return The uploaded parameter value cast to type `T`.
   *
   * @throws std::runtime_error If the SDO upload fails or the value cannot be
   * extracted as type `T`.
   * @throws std::bad_variant_access If the internal variant does not hold the
   * requested type `T`.
   */
  template <typename T>
  T upload(uint16_t index, uint8_t subindex,
           const std::chrono::steady_clock::duration expiryTime =
               std::chrono::milliseconds(5000)) {
    common::Parameter& parameter = upload(index, subindex, expiryTime);
    return parameter.getValue<T>();
  }

  void download(uint16_t index, uint8_t subindex,
                const std::chrono::steady_clock::duration expiryTime =
                    std::chrono::milliseconds(5000)) override;

  void download(uint16_t index, uint8_t subindex,
                const common::ParameterValue& value,
                const std::chrono::steady_clock::duration expiryTime =
                    std::chrono::milliseconds(5000)) override;

  /**
   * @brief Sets and downloads a parameter value to the device using the
   * specified type.
   *
   * This templated overload wraps the given value in a
   * `common::ParameterValue`, updates the local parameter store, and performs
   * an SDO download to transfer the data to the device.
   *
   * @tparam T The type of the value to set and download.
   * @param index The 16-bit index of the parameter in the object dictionary.
   * @param subindex The 8-bit subindex of the parameter.
   * @param value The new parameter value to set and download.
   * @param expiryTime The maximum duration to wait for the SDO download
   * operation. Defaults to 5000 milliseconds.
   *
   * @throws std::runtime_error If the parameter data is empty after setting the
   * value, or if the SDO download operation fails.
   */
  template <typename T>
  void download(uint16_t index, uint8_t subindex, const T& value,
                const std::chrono::steady_clock::duration expiryTime =
                    std::chrono::milliseconds(5000)) {
    download(index, subindex, common::ParameterValue(value));
  }

  /**
   * @brief Sends process data to a remote device and receives the response.
   *
   * Sends a process data message over Ethernet to the remote device and waits
   * for a corresponding response. This method increments the sequence ID,
   * constructs and serializes the request message, transmits it via the socket,
   * and parses the received response.
   *
   * @param data The payload data to transmit to the device.
   * @param expiryTime The duration to wait for the socket operation
   * (read/write) before timing out. Defaults to 1000 milliseconds.
   *
   * @return A vector of bytes containing the response data from the device.
   * Returns an empty vector if the exchange fails (e.g., due to a non-OK
   * status).
   */
  std::vector<uint8_t> sendAndReceiveProcessData(
      const std::vector<uint8_t>& data,
      const std::chrono::steady_clock::duration expiryTime =
          std::chrono::milliseconds(1000));

  /**
   * @brief Exchanges process data with the remote device and updates local
   * parameters accordingly.
   *
   * This function prepares the process data to send by collecting the current
   * data from parameters mapped as RxPDOs. It then sends this data and receives
   * updated process data from the remote device. The received data is used to
   * update local parameters mapped as TxPDOs.
   *
   * The bitlength of each PDO entry is converted to byte size, rounding up to
   * handle non-byte-aligned bit lengths. If received data is smaller than
   * expected for a given PDO, an error is logged and the update for that PDO is
   * skipped. After processing all PDOs, if there is any extra received data not
   * mapped to parameters, a warning is logged.
   *
   * @throws std::runtime_error if findParameter throws (e.g., if parameter not
   * found).
   */
  void exchangeProcessDataAndUpdateParameters() override;

 private:
  boost::asio::io_context ioContext_;  ///< The Boost.Asio I/O context for
                                       ///< managing asynchronous operations.
  boost::asio::ip::tcp::endpoint
      endpoint_;  ///< The endpoint (IP address and port) to which the client
                  ///< will connect.
  boost::asio::ip::tcp::socket
      socket_;  ///< The TCP socket used for communication with the server.

  std::mutex mutex_;  ///< Mutex for synchronizing access to the socket
                      ///< read/write operations.

  /**
   * @brief Atomic sequence identifier (16-bit unsigned integer).
   *
   * This variable is used to store and increment the sequence ID atomically,
   * ensuring thread safety in a multithreaded environment. The value is
   * initialized to 0.
   */
  std::atomic<uint16_t> seqId_{0};  ///< Sequence ID for message tracking.

  std::unordered_map<common::ParameterKey, common::Parameter>
      parametersMap_;  ///< Map of parameters indexed by (index, subindex)
                       ///< pairs.
  common::PdoMappings pdoMappings_;  ///< Mapped PDO entries for the slave.
};
