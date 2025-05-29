#pragma once

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace common {

/**
 * @brief Parses a hexadecimal string into an unsigned integer of type T.
 *
 * This function converts a hex string (e.g., "0x1A3F" or "1A3F") to an
 * unsigned integer of the specified type T. It supports any unsigned
 * integer type such as uint16_t, uint32_t, uint64_t, etc.
 *
 * The function uses `std::stoul` for types up to the size of `unsigned long`,
 * and `std::stoull` for larger types (e.g., `uint64_t`).
 *
 * @tparam T The unsigned integer type to parse to. Must be an unsigned integral
 * type.
 * @param s The hexadecimal string to parse.
 * @return Parsed value as type T.
 *
 * @throws std::invalid_argument if the string is not a valid hex number.
 * @throws std::out_of_range if the parsed value is out of range for type T.
 */
template <typename T>
T parseHex(const std::string& s) {
  static_assert(std::is_unsigned<T>::value,
                "parseHex only supports unsigned integer types");

  // Use the appropriate std::stoul or std::stoull depending on size
  if constexpr (sizeof(T) <= sizeof(unsigned long)) {
    // Use std::stoul
    return static_cast<T>(std::stoul(s, nullptr, 16));
  } else {
    // Use std::stoull for larger types like uint64_t
    return static_cast<T>(std::stoull(s, nullptr, 16));
  }
}

/**
 * @brief Represents the PDO mapping for RX and TX channels.
 *
 * Contains two ordered maps:
 * - `rx`: Maps 16-bit keys to vectors of 32-bit unsigned integers for receive
 * PDO entries.
 * - `tx`: Maps 16-bit keys to vectors of 32-bit unsigned integers for transmit
 * PDO entries.
 */
struct UiPdoMapping {
  std::map<std::uint16_t, std::vector<std::uint32_t>> rx;
  std::map<std::uint16_t, std::vector<std::uint32_t>> tx;
};

/**
 * @brief Top-level UI configuration JSON structure.
 *
 * Contains the PDO mappings under the `pdoMapping` member.
 */
struct UiConfigJson {
  UiPdoMapping pdoMapping;
};

/**
 * @brief Deserialize a JSON object into a UiPdoMapping structure.
 *
 * This function parses the JSON object representing the PDO mappings,
 * extracting the "rx" and "tx" maps. The JSON keys are hex strings
 * (e.g., "0x1600") which are converted to uint16_t keys in the maps.
 * The values are arrays of hex strings representing uint32_t entries.
 *
 * @param j JSON object expected to contain "rx" and "tx" mappings.
 * @param p Reference to UiPdoMapping struct to populate with parsed data.
 */
void from_json(const nlohmann::json& j, UiPdoMapping& p);

/**
 * @brief Deserialize a JSON object into a UiConfigJson structure.
 *
 * Extracts the "pdoMapping" field from the JSON object and converts it
 * into a UiPdoMapping instance, which is assigned to the `pdoMapping` member.
 *
 * @param j JSON object expected to contain a "pdoMapping" field.
 * @param r Reference to UiConfigJson struct to populate.
 */
void from_json(const nlohmann::json& j, UiConfigJson& r);

/**
 * @brief Enum class representing EtherCAT vendor IDs.
 *
 * This enum class defines vendor IDs for EtherCAT devices, with each vendor ID
 * being represented by a unique 32-bit unsigned integer value. It is used to
 * identify different EtherCAT vendors in the system.
 */
enum class EtherCATVendorID : uint32_t {
  /**
   * @brief Vendor ID for Synapticon GmbH.
   *
   * Synapticon GmbH provides high-performance and compact servo drives.
   */
  SYNAPTICON = 0x000022d2,  ///< Vendor ID for Synapticon GmbH
};

/**
 * @brief Compares a uint32_t value with an EtherCATVendorID enum value.
 *
 * This operator allows comparison between a raw vendor ID (uint32_t) and an
 * EtherCATVendorID enum value. It converts the enum to its underlying uint32_t
 * type for the comparison.
 *
 * @param lhs The raw vendor ID as a uint32_t.
 * @param rhs The EtherCATVendorID enum value to compare.
 * @return true if the vendor ID matches the enum value, false otherwise.
 */
inline bool operator==(uint32_t lhs, EtherCATVendorID rhs) {
  return lhs == static_cast<uint32_t>(rhs);
}

/**
 * @brief Compares an EtherCATVendorID enum value with a uint32_t value.
 *
 * This operator allows comparison between an EtherCATVendorID enum value and
 * a raw vendor ID (uint32_t). It converts the enum value to its underlying
 * uint32_t type for the comparison.
 *
 * @param lhs The EtherCATVendorID enum value to compare.
 * @param rhs The raw vendor ID as a uint32_t.
 * @return true if the enum value matches the vendor ID, false otherwise.
 */
inline bool operator==(EtherCATVendorID lhs, uint32_t rhs) {
  return static_cast<uint32_t>(lhs) == rhs;
}

/**
 * @brief Represents a single PDO (Process Data Object) mapping entry.
 *
 * This structure defines a mapping for one object within a PDO. It includes:
 * - `pdoIndex`: The PDO index (e.g., 0x1600 for RxPDO, 0x1A00 for TxPDO).
 * - `index`: The object dictionary index inside the PDO (e.g., 0x607A).
 * - `subindex`: The subindex of the object within the PDO (e.g., 0x00).
 * - `bitlength`: The size of the mapped entry in bits (e.g., 16, 32, etc.).
 */
struct PdoMappingEntry {
  uint16_t pdoIndex;  ///< The PDO index (e.g., 0x1600 or 0x1A00)
  uint16_t index;     ///< The object dictionary index (e.g., 0x607A)
  uint8_t subindex;   ///< The subindex of the object (e.g., 0x00)
  uint8_t
      bitlength;  ///< The size of the mapped entry in bits (e.g., 16, 32, etc.)
};

/**
 * @brief Represents the mapped PDO entries for a slave device.
 *
 * This structure contains two vectors:
 * - `rxPdos`: Mapped entries for received PDOs (Slave inputs, Master outputs).
 * - `txPdos`: Mapped entries for transmitted PDOs (Slave outputs, Master
 * inputs).
 *
 * Each entry in these vectors holds detailed mapping information, including:
 * - `pdoIndex`: The PDO index (e.g., 0x1600 for RxPDO, 0x1A00 for TxPDO).
 * - `index`: The object dictionary index inside the PDO (e.g., 0x607A).
 * - `subindex`: The subindex of the object within the PDO (e.g., 0x00).
 * - `bitlength`: The size of the mapped entry in bits (e.g., 16, 32, etc.).
 */
struct PdoMappings {
  std::vector<PdoMappingEntry> rxPdos;  ///< Mapped entries for received PDOs
                                        ///< (Slave inputs, Master outputs)
  std::vector<PdoMappingEntry> txPdos;  ///< Mapped entries for transmitted PDOs
                                        ///< (Slave outputs, Master inputs)
};

/**
 * @brief Enum class representing various object flags.
 *
 * This enum class defines different flags used for controlling access,
 * mapping, and other attributes. It is represented as a 16-bit unsigned
 * integer.
 */
enum class ObjectFlags : uint16_t {
  None = 0x0000,

  // Read access
  PO_RD = 0x0001,
  SO_RD = 0x0002,
  OP_RD = 0x0004,
  ALL_RD = PO_RD | SO_RD | OP_RD,

  // Write access
  PO_WR = 0x0008,
  SO_WR = 0x0010,
  OP_WR = 0x0020,
  ALL_WR = PO_WR | SO_WR | OP_WR,

  // Read/Write combinations
  PO_RDWR = PO_RD | PO_WR,
  SO_RDWR = SO_RD | SO_WR,
  OP_RDWR = OP_RD | OP_WR,
  ALL_RDWR = PO_RDWR | SO_RDWR | OP_RDWR,

  // Mapping
  RXPDO_MAP = 0x0040,
  TXPDO_MAP = 0x0080,
  RXTXPDO_MAP = 0x00C0,

  // Other flags
  BACKUP = 0x0100,
  STARTUP = 0x0200,

  ALL_LIST_FLAGS = RXPDO_MAP | TXPDO_MAP | BACKUP | STARTUP
};

/**
 * @brief Bitwise OR operator for ObjectFlags enum class.
 *
 * This operator allows for combining two `ObjectFlags` values using the
 * bitwise OR operator.
 *
 * @param lhs Left-hand operand of the bitwise OR operation.
 * @param rhs Right-hand operand of the bitwise OR operation.
 * @return The result of the bitwise OR operation as an `ObjectFlags` value.
 */
inline ObjectFlags operator|(ObjectFlags lhs, ObjectFlags rhs) {
  return static_cast<ObjectFlags>(static_cast<uint16_t>(lhs) |
                                  static_cast<uint16_t>(rhs));
}

/**
 * @brief Bitwise AND operator for ObjectFlags enum class.
 *
 * This operator allows for checking the intersection of two `ObjectFlags`
 * values using the bitwise AND operator.
 *
 * @param lhs Left-hand operand of the bitwise AND operation.
 * @param rhs Right-hand operand of the bitwise AND operation.
 * @return The result of the bitwise AND operation as an `ObjectFlags` value.
 */
inline ObjectFlags operator&(ObjectFlags lhs, ObjectFlags rhs) {
  return static_cast<ObjectFlags>(static_cast<uint16_t>(lhs) &
                                  static_cast<uint16_t>(rhs));
}

/**
 * @brief Bitwise OR assignment operator for ObjectFlags enum class.
 *
 * This operator allows the modification of an `ObjectFlags` value by OR'ing it
 * with another.
 *
 * @param lhs Left-hand operand, which will be modified with the OR operation.
 * @param rhs Right-hand operand to OR with the left-hand operand.
 * @return The modified left-hand operand as an `ObjectFlags` value.
 */
inline ObjectFlags& operator|=(ObjectFlags& lhs, ObjectFlags rhs) {
  lhs = lhs | rhs;
  return lhs;
}

/**
 * @brief Combine multiple `ObjectFlags` values into one.
 *
 * This function is a convenience method that combines multiple `ObjectFlags`
 * values by OR'ing them together.
 *
 * @param b First `ObjectFlags` value.
 * @param s Second `ObjectFlags` value.
 * @param p Third `ObjectFlags` value.
 * @param a Fourth `ObjectFlags` value.
 * @return The result of combining the four `ObjectFlags` values as a single
 * `ObjectFlags` value.
 */
inline ObjectFlags SetObjectFlags(ObjectFlags b, ObjectFlags s, ObjectFlags p,
                                  ObjectFlags a) {
  return b | s | p | a;
}

/**
 * @enum ObjectDataType
 * @brief Enumerates the supported data types for parameters.
 *
 * This enumeration defines the various data types that parameters can use.
 * Each entry is explicitly mapped to a 16-bit unsigned integer value.
 * It includes basic types such as integers, floating-point numbers, strings,
 * time-related types, bitfields, and user-defined types.
 *
 * @note The values in this enumeration are defined in the ETG.1020 document,
 *       which outlines the EtherCAT protocol and its supported data types.
 */
enum class ObjectDataType : uint16_t {
  UNSPECIFIED = 0x0000,  ///< Undefined or unknown data type.

  BOOLEAN = 0x0001,  ///< Boolean value (true or false).
  BYTE = 0x001E,     ///< 8-bit unsigned integer.
  WORD = 0x001F,     ///< Two octets (16 bits) unsigned integer.
  DWORD = 0x0020,    ///< Four octets (32 bits) unsigned integer.

  BIT1 = 0x0030,   ///< 1-bit field.
  BIT2 = 0x0031,   ///< 2-bit field.
  BIT3 = 0x0032,   ///< 3-bit field.
  BIT4 = 0x0033,   ///< 4-bit field.
  BIT5 = 0x0034,   ///< 5-bit field.
  BIT6 = 0x0035,   ///< 6-bit field.
  BIT7 = 0x0036,   ///< 7-bit field.
  BIT8 = 0x0037,   ///< 8-bit field.
  BIT9 = 0x0038,   ///< 9-bit field.
  BIT10 = 0x0039,  ///< 10-bit field.
  BIT11 = 0x003A,  ///< 11-bit field.
  BIT12 = 0x003B,  ///< 12-bit field.
  BIT13 = 0x003C,  ///< 13-bit field.
  BIT14 = 0x003D,  ///< 14-bit field.
  BIT15 = 0x003E,  ///< 15-bit field.
  BIT16 = 0x003F,  ///< 16-bit field.

  BITARR8 = 0x002D,   ///< Array of 8 bits.
  BITARR16 = 0x002E,  ///< Array of 16 bits.
  BITARR32 = 0x002F,  ///< Array of 32 bits.

  INTEGER8 = 0x0002,   ///< 8-bit signed integer.
  INTEGER16 = 0x0003,  ///< 16-bit signed integer.
  INTEGER24 = 0x0010,  ///< 24-bit signed integer.
  INTEGER32 = 0x0004,  ///< 32-bit signed integer.
  INTEGER40 = 0x0012,  ///< 40-bit signed integer.
  INTEGER48 = 0x0013,  ///< 48-bit signed integer.
  INTEGER56 = 0x0014,  ///< 56-bit signed integer.
  INTEGER64 = 0x0015,  ///< 64-bit signed integer.

  UNSIGNED8 = 0x0005,   ///< 8-bit unsigned integer.
  UNSIGNED16 = 0x0006,  ///< 16-bit unsigned integer.
  UNSIGNED24 = 0x0016,  ///< 24-bit unsigned integer.
  UNSIGNED32 = 0x0007,  ///< 32-bit unsigned integer.
  UNSIGNED40 = 0x0018,  ///< 40-bit unsigned integer.
  UNSIGNED48 = 0x0019,  ///< 48-bit unsigned integer.
  UNSIGNED56 = 0x001A,  ///< 56-bit unsigned integer.
  UNSIGNED64 = 0x001B,  ///< 64-bit unsigned integer.

  REAL32 = 0x0008,  ///< 32-bit floating-point number.
  REAL64 = 0x0011,  ///< 64-bit floating-point number (double).

  GUID = 0x001D,  ///< 128-bit globally unique identifier.

  VISIBLE_STRING = 0x0009,  ///< Null-terminated ASCII string.
  OCTET_STRING = 0x000A,    ///< Array of raw bytes.
  UNICODE_STRING = 0x000B,  ///< Null-terminated Unicode string (UTF-16/UTF-32
                            ///< depending on platform).
  ARRAY_OF_INT = 0x0260,    ///< Sequence of INT.
  ARRAY_OF_SINT = 0x0261,   ///< Sequence of SINT.
  ARRAY_OF_DINT = 0x0262,   ///< Sequence of DINT.
  ARRAY_OF_UDINT = 0x0263,  ///< Sequence of UDINT.

  PDO_MAPPING = 0x0021,    ///< For PDO_MAPPING definition see ETG.1000
  IDENTITY = 0x0023,       ///< For IDENTITY definition see ETG.1000
  COMMAND_PAR = 0x0025,    ///< For COMMAND_PAR definition see ETG.1000
  PDO_PARAMETER = 0x0027,  ///< For PDO_PARAMETER definition see ETG.1020
  ENUM = 0x0028,           ///< For ENUM definition see ETG.1020
  SM_SYNCHRONIZATION =
      0x0029,       ///< For SM_SYNCHRONIZATION definition see ETG.1000
  RECORD = 0x002A,  ///< No pre-defined Record structure. Can be used as a
                    ///< general data type for Records.
  BACKUP_PARAMETER = 0x002B,  ///< For BACKUP_PARAMETER definition see ETG.1020
  MODULAR_DEVICE_PARAMETER = 0x002C,  ///< For MODULAR_DEVICE_PARAMETER
                                      ///< definition see ETG.5001
  ERROR_SETTING = 0x0281,  ///< For ERROR_SETTING definition see ETG.1020
  DIAGNOSIS_HISTORY =
      0x0282,  ///< For DIAGNOSIS_HISTORY definition see ETG.1020
  EXTERNAL_SYNC_STATUS = 0x0283,    ///< For EXTERNAL_SYNC_STATUS definition see
                                    ///< ETG.1020
  EXTERNAL_SYNC_SETTINGS = 0x0284,  ///< For EXTERNAL_SYNC_SETTINGS definition
                                    ///< see ETG.1020
  DEFTYPE_FSOEFRAME = 0x0285,       ///< For DEFTYPE_FSOEFRAME definition see
                                    ///< ETG.5120
  DEFTYPE_FSOECOMMPAR =
      0x0286,  ///< For
               ///< DEFTYPE_FSOECOMMPAR definition see ETG.5120

  TIME_OF_DAY = 0x000C,  ///< Time of day format (implementation-defined).
  TIME_DIFFERENCE =
      0x000D,            ///< Time difference format (implementation-defined).
  UTYPE_START = 0x0800,  ///< Start of user-defined type range.
  UTYPE_END = 0x0FFF     ///< End of user-defined type range.
};

/**
 * @brief Enum class representing different object types used in the EtherCAT
 * protocol.
 *
 * This enum class defines various object types that represent the **kind of
 * object** in an EtherCAT device. Each value corresponds to a specific type of
 * object, such as a definition, variable, array, or record, in the EtherCAT
 * protocol.
 */
enum class ObjectCode : uint16_t {
  DEFTYPE = 0x0005,    ///< Definition type object.
  DEFSTRUCT = 0x0006,  ///< Definition structure object.
  VAR = 0x0007,        ///< Variable object type.
  ARRAY = 0x0008,      ///< Array object type.
  RECORD = 0x0009,     ///< Record object type.
};

/**
 * @brief Alias for a pair of uint16_t and uint8_t representing a parameter key.
 *
 * This alias simplifies the use of a `std::pair<uint16_t, uint8_t>`, where the
 * first element (`uint16_t`) represents the index, and the second element
 * (`uint8_t`) represents the subindex in a parameter lookup.
 */
using ParameterKey = std::pair<uint16_t, uint8_t>;

/**
 * @brief Alias for a variant that can hold multiple types of values.
 *
 * This alias defines a `std::variant` that can hold one of the following
 * types:
 * - `bool`: Boolean value.
 * - `std::int8_t`: 8-bit signed integer.
 * - `std::int16_t`: 16-bit signed integer.
 * - `std::int32_t`: 32-bit signed integer.
 * - `std::int64_t`: 64-bit signed integer.
 * - `std::uint8_t`: 8-bit unsigned integer.
 * - `std::uint16_t`: 16-bit unsigned integer.
 * - `std::uint32_t`: 32-bit unsigned integer.
 * - `std::uint64_t`: 64-bit unsigned integer.
 * - `float`: Single-precision floating-point value.
 * - `double`: Double-precision floating-point value.
 * - `std::string`: String value.
 * - `std::vector<std::uint8_t>`: A vector of bytes (uint8_t).
 *
 * This `std::variant` type is used to store a value of one of the specified
 * types, providing flexibility in handling different data types in the same
 * container.
 */
using ParameterValue =
    std::variant<bool, std::int8_t, std::int16_t, std::int32_t, std::int64_t,
                 std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t,
                 float, double, std::string,
                 std::vector<std::uint8_t>>;  ///< Alias for a variant type
                                              ///< holding multiple value types.

/**
 * @class Parameter
 * @brief Represents a device parameter identified by index and subindex.
 *
 * The `Parameter` class models an object from a device's object dictionary,
 * uniquely identified by a 16-bit index and an 8-bit subindex. These parameters
 * typically appear in communication profiles such as CANopen or other
 * embedded protocols that use structured configuration and runtime data.
 *
 * Each `Parameter` holds metadata (name, data type, access rights, etc.) and
 * stores its actual value as a byte array. The value can be safely interpreted
 * and manipulated through variant-based and templated getter/setter functions,
 * ensuring proper type handling.
 *
 * The class also provides comparison operators for sorting or lookup based on
 * index/subindex and supports JSON serialization for configuration
 * export/import.
 *
 * @see ParameterValue, common::ObjectDataType, common::ObjectCode,
 * common::ObjectFlags
 */
class Parameter {
 public:
  /**
   * @brief Name of the parameter.
   */
  std::string name;

  /**
   * @brief Index of the parameter.
   */
  std::uint16_t index;

  /**
   * @brief Subindex of the parameter.
   */
  std::uint8_t subindex;

  /**
   * @brief Bit length of the parameter.
   */
  std::uint16_t bitLength;

  /**
   * @brief Byte length of the parameter.
   */
  int byteLength;

  /**
   * @brief The data type of the parameter, defined by
   * `common::ObjectDataType`.
   */
  common::ObjectDataType dataType;

  /**
   * @brief The object code for the parameter, defined by `common::ObjectCode`.
   */
  common::ObjectCode code;

  /**
   * @brief The object flags for the parameter, defined by
   * `common::ObjectFlags`.
   */
  common::ObjectFlags flags;

  /**
   * @brief The access flags for the parameter, defined by
   * `common::ObjectFlags`.
   */
  common::ObjectFlags access;

  /**
   * @brief Holds raw data as a vector of uint8_t elements.
   */
  std::vector<std::uint8_t> data;

  /**
   * @brief Retrieves the value of the parameter based on its data type.
   *
   * This function extracts the raw data from the parameter's internal storage
   * (`data`), converts it to the appropriate type based on the `dataType`, and
   * returns it as a `ParameterValue` (which is a `std::variant`). The supported
   * types include various integer, floating-point, and string types. For string
   * types (VISIBLE_STRING, OCTET_STRING, UNICODE_STRING), the function ensures
   * the string is null-terminated.
   *
   * @return ParameterValue The value of the parameter as a `ParameterValue`
   * (std::variant), which can be one of the following types:
   *         - `bool` for BOOLEAN
   *         - `std::int8_t` for INTEGER8
   *         - `std::int16_t` for INTEGER16
   *         - `std::int32_t` for INTEGER24 and INTEGER32
   *         - `std::int64_t` for INTEGER64
   *         - `std::uint8_t` for UNSIGNED8, PDO_MAPPING, IDENTITY, COMMAND_PAR,
   * and RECORD
   *         - `std::uint16_t` for UNSIGNED16
   *         - `std::uint32_t` for UNSIGNED24 and UNSIGNED32
   *         - `std::uint64_t` for UNSIGNED64
   *         - `float` for REAL32
   *         - `double` for REAL64
   *         - `std::string` for VISIBLE_STRING, OCTET_STRING, and
   * UNICODE_STRING
   *
   * @throws std::runtime_error If the `dataType` is not supported or is
   * unknown.
   */
  ParameterValue getValue() const;

  /**
   * @brief Retrieves the parameter value from the `ParameterValue` variant as
   * the specified type.
   *
   * This function attempts to extract the stored parameter value from the
   * `ParameterValue` variant, and if successful, it returns the value as the
   * specified type `T`. If the type `T` does not match the type stored in the
   * variant, a `std::bad_variant_access` exception is thrown.
   *
   * @tparam T The type to retrieve from the `ParameterValue`. This type must
   * match the type stored in the variant.
   *
   * @return T The value of the parameter, cast to the type `T`.
   *
   * @throws std::bad_variant_access If the requested type `T` does not match
   * the type stored in the `ParameterValue`.
   */
  template <typename T>
  T getValue() const {
    // Get the value as a ParameterValue
    const ParameterValue& val = getValue();

    // Try to get the value of type T from the variant
    if (auto ptr = std::get_if<T>(&val)) {
      return *ptr;  // Return the value of type T
    } else {
      throw std::bad_variant_access();  // If type T is not found in the variant
    }
  }

  /**
   * @brief Attempts to retrieve the parameter value from the `ParameterValue`
   * variant as the specified type.
   *
   * This function tries to extract the stored parameter value from the
   * `ParameterValue` variant and returns it as the specified type `T` in an
   * `std::optional<T>`. If the type `T` does not match the type stored in the
   * variant, the function returns `std::nullopt`.
   *
   * @tparam T The type to retrieve from the `ParameterValue`. This type must
   * match the type stored in the variant.
   *
   * @return std::optional<T> An optional containing the value of type `T` if
   * found; otherwise, `std::nullopt`.
   */
  template <typename T>
  std::optional<T> tryGetValue() const {
    const ParameterValue& val = getValue();
    if (auto ptr = std::get_if<T>(&val)) {
      return *ptr;
    } else {
      return std::nullopt;
    }
  }

  /**
   * @brief Sets the internal raw data representation from a given value.
   *
   * This function converts the provided value into a byte representation
   * and stores it in the internal `data` vector, based on the current
   * `dataType`. It supports all standard types defined in ObjectDataType.
   *
   * If the value is a vector of bytes (`std::vector<std::uint8_t>`), it is
   * directly copied into the internal buffer. Otherwise, the value is cast
   * to the expected type based on `dataType`, converted to bytes, and stored.
   *
   * For string types (`VISIBLE_STRING`, `OCTET_STRING`, `UNICODE_STRING`),
   * the string content is copied and null-terminated if not already.
   *
   * @param value The value to store, wrapped in a ParameterValue variant.
   *
   * @throws std::bad_variant_access If the value's type does not match the
   * expected type.
   * @throws std::runtime_error If the data type is unsupported.
   */
  void setValue(const ParameterValue& value);

  /**
   * @brief Sets the parameter value using a strongly-typed input.
   *
   * This templated overload constructs a ParameterValue variant from the
   * provided typed value and delegates to the main setValue function for
   * byte-level storage based on the current ObjectDataType.
   *
   * @tparam T The type of the input value. Must be compatible with
   * ParameterValue.
   * @param value The value to set.
   *
   * @throws std::bad_variant_access If the type T is incompatible with the
   * expected data type.
   * @throws std::runtime_error If the data type is unsupported during
   * conversion.
   */
  template <typename T>
  void setValue(const T& value) {
    // Convert the input value to ParameterValue (variant)
    ParameterValue parameterValue(value);

    // Reuse the existing setValue function to handle the assignment
    setValue(parameterValue);
  }

  /**
   * @brief Attempts to set the parameter value with a strongly-typed input.
   *
   * This templated method checks whether the type of the input value matches
   * the expected type based on the current ObjectDataType. If compatible, it
   * sets the value; otherwise, it fails silently.
   *
   * @tparam T The type of the input value to attempt setting.
   * @param value The value to attempt to assign to the parameter.
   * @return true if the type matches the expected data type and the value was
   * set; false otherwise.
   *
   * @note Supports raw byte input via std::vector<std::uint8_t> as a fallback.
   */
  template <typename T>
  bool trySetValue(const T& value) {
    auto expectedType = [this]() -> std::type_index {
      switch (dataType) {
        case ObjectDataType::BOOLEAN: {
          return typeid(bool);
        }
        case ObjectDataType::INTEGER8: {
          return typeid(std::int8_t);
        }
        case ObjectDataType::INTEGER16: {
          return typeid(std::int16_t);
        }
        case ObjectDataType::INTEGER24:
        case ObjectDataType::INTEGER32: {
          return typeid(std::int32_t);
        }
        case ObjectDataType::INTEGER64: {
          return typeid(std::int64_t);
        }
        case ObjectDataType::UNSIGNED8:
        case ObjectDataType::PDO_MAPPING:
        case ObjectDataType::IDENTITY:
        case ObjectDataType::COMMAND_PAR:
        case ObjectDataType::RECORD: {
          return typeid(std::uint8_t);
        }
        case ObjectDataType::UNSIGNED16: {
          return typeid(std::uint16_t);
        }
        case ObjectDataType::UNSIGNED24:
        case ObjectDataType::UNSIGNED32: {
          return typeid(std::uint32_t);
        }
        case ObjectDataType::UNSIGNED64: {
          return typeid(std::uint64_t);
        }
        case ObjectDataType::REAL32: {
          return typeid(float);
        }
        case ObjectDataType::REAL64: {
          return typeid(double);
        }
        case ObjectDataType::VISIBLE_STRING:
        case ObjectDataType::OCTET_STRING:
        case ObjectDataType::UNICODE_STRING: {
          return typeid(std::string);
        }
        default: {
          return typeid(void);
        }
      }
    };

    const std::type_index expected = expectedType();
    if (expected != typeid(T) &&
        typeid(T) != typeid(std::vector<std::uint8_t>)) {
      return false;
    }

    setValue(ParameterValue(value));
    return true;
  }

  /**
   * @brief Compares two Parameter objects for ordering.
   *
   * The comparison is first done by the `index` property. If the indices are
   * equal, the comparison is then done by the `subindex` property.
   *
   * @param other The other Parameter object to compare with.
   * @return True if the current object is less than the other object, false
   * otherwise.
   */
  bool operator<(const Parameter& other) const {
    if (index != other.index) {
      return index < other.index;  // Compare by index first
    }
    return subindex <
           other.subindex;  // Compare by subindex if indices are equal
  }

  /**
   * @brief Compares two Parameter objects for reverse ordering.
   *
   * The comparison is first done by the `index` property. If the indices are
   * equal, the comparison is then done by the `subindex` property.
   *
   * @param other The other Parameter object to compare with.
   * @return True if the current object is greater than the other object, false
   * otherwise.
   */
  bool operator>(const Parameter& other) const {
    if (index != other.index) {
      return index > other.index;  // Compare by index first
    }
    return subindex >
           other.subindex;  // Compare by subindex if indices are equal
  }

  /**
   * @brief Checks if two Parameter objects are equal.
   *
   * The equality check is performed by comparing the `index` and `subindex`
   * properties. Both properties must be equal for the objects to be considered
   * equal.
   *
   * @param other The other Parameter object to compare with.
   * @return True if both the `index` and `subindex` are equal, false otherwise.
   */
  bool operator==(const Parameter& other) const {
    return index == other.index && subindex == other.subindex;
  }

  /**
   * @brief Serializes a Parameter object to JSON.
   *
   * Converts the given `Parameter` object into a JSON representation,
   * storing its fields such as name, index, data type, etc. Enums are
   * cast to `uint16_t` to ensure compatibility with JSON output.
   *
   * @param j The JSON object to store serialized data.
   * @param p The `Parameter` instance to serialize.
   */
  static void to_json(nlohmann::json& j, const Parameter& p);

  /**
   * @brief Deserializes a Parameter object from JSON.
   *
   * Parses the JSON object and populates the fields of the given
   * `Parameter` instance. This assumes the JSON structure matches the
   * format produced by `to_json`.
   *
   * @param j The JSON object to deserialize.
   * @param p The `Parameter` instance to populate.
   */
  static void from_json(const nlohmann::json& j, Parameter& p);
};

/**
 * @brief Converts a ParameterValue to a string representation.
 *
 * This function uses `std::visit` to convert the given `ParameterValue`
 * (a std::variant of numeric and string types) to a human-readable string.
 * - `bool` values are converted to `"true"` or `"false"`.
 * - Integral and floating-point values are converted using `std::to_string()`.
 * - `std::string` values are returned as-is.
 *
 * @param value The parameter value to convert.
 * @return A string representation of the value.
 */
std::string convertParameterValueToString(const ParameterValue& value);

/**
 * @brief Logs the contents of a map of parameters with an option to sort them.
 *
 * This function takes a map of parameters indexed by (index, subindex) pairs,
 * optionally sorts them by index and subindex (based on the `sortParameters`
 * flag), and logs each parameter's details including its name, access type, bit
 * length, and other relevant information.
 *
 * If the `sortParameters` flag is true, the parameters will be sorted by index
 * and subindex before logging. If `sortParameters` is false, the parameters
 * will be logged in their original order as they appear in the map.
 *
 * @param parametersMap A map where each key is a pair of (index, subindex) and
 *                      the value is a Parameter object containing metadata.
 * @param sortParameters A boolean flag indicating whether to sort the
 * parameters before logging. Default is true. If true, the parameters are
 * sorted by index and subindex; if false, they are logged in their original
 * order.
 */
void logParametersMap(const std::unordered_map<std::pair<uint16_t, uint8_t>,
                                               Parameter>& parametersMap,
                      bool sortParameters = true);

/**
 * @brief Abstract interface representing a generic device.
 *
 * This class defines the virtual interface for device operations such as
 * state management, parameter handling, and file transfer.
 * All methods are pure virtual and must be implemented by derived classes.
 */
class Device {
 public:
  /**
   * @brief Virtual destructor for Device interface.
   *
   * Ensures derived class destructors are called properly when deleting
   * through a pointer to Device.
   *
   * The default destructor implementation is used.
   */
  virtual ~Device() = default;

  /**
   * @brief Reads and returns the current state of the device.
   *
   * This function sends a request to retrieve the device’s state, which follows
   * the EtherCAT state machine. The returned state is represented as a
   * `uint8_t` with the following possible values:
   * - INIT:   1
   * - PREOP:  2
   * - SAFEOP: 4
   * - OP:     8
   * - BOOT:   3
   *
   * @param expiryTime The maximum time to wait for the operation to complete.
   *
   * @return The current device state as a `uint8_t`.
   */
  virtual uint8_t getState(
      const std::chrono::steady_clock::duration expiryTime =
          std::chrono::milliseconds(3000)) = 0;

  /**
   * @brief Sets the device to a specified state.
   *
   * Attempts to change the remote device's state to the given value,
   * waiting up to the specified expiry time for the operation to complete.
   *
   * @param state The new state to set on the remote device.
   * @param expiryTime The maximum duration to wait for the operation to
   * complete.
   *
   * @return `true` if the state was successfully set; `false` otherwise.
   */
  virtual bool setState(uint8_t state,
                        const std::chrono::steady_clock::duration expiryTime =
                            std::chrono::milliseconds(3000)) = 0;

  /**
   * @brief Reads the contents of a file from the device.
   *
   * @param filename The name of the file to be read.
   * @param expiryTime The maximum duration to wait for the operation to
   * complete.
   *
   * @return A `std::vector<uint8_t>` containing the file data received from
   * the device.
   */
  virtual std::vector<uint8_t> readFile(
      const std::string& filename,
      const std::chrono::steady_clock::duration expiryTime =
          std::chrono::milliseconds(5000)) = 0;

  /**
   * @brief Sends a file to the device.
   *
   * @param filename The name of the file to be written.
   * @param data The data to be written to the file.
   * @param expiryTime The maximum duration to wait for the operation to
   * complete.
   *
   * @return Returns true if the file was successfully written; otherwise,
   * false.
   */
  virtual bool writeFile(const std::string& filename,
                         const std::vector<uint8_t>& data,
                         const std::chrono::steady_clock::duration expiryTime =
                             std::chrono::milliseconds(5000)) = 0;

  /**
   * @brief Loads parameters from the device and stores them locally.
   *
   * @param readValues If true, the values of the parameters are read from the
   * device; otherwise, only the parameter metadata is retrieved.
   * @param expiryTime The maximum duration to wait for the operation to
   * complete.
   */
  virtual void loadParameters(
      bool readValues, const std::chrono::steady_clock::duration expiryTime =
                           std::chrono::milliseconds(9000)) = 0;

  /**
   * @brief Clears all loaded object dictionary parameters.
   *
   * This function removes all entries from the internal parameter map,
   * effectively resetting the parameter list of the slave device.
   */
  virtual void clearParameters() = 0;

  /**
   * @brief Finds and returns a reference to a parameter by its index and
   * subindex.
   *
   * This function looks up a parameter in the internal parameter map using the
   * specified index and subindex. If the parameter is found, a reference to it
   * is returned. If not, a runtime exception is thrown.
   *
   * @param index The index of the parameter to find.
   * @param subindex The subindex of the parameter to find.
   * @return Reference to the found parameter.
   *
   * @throws std::runtime_error If the parameter with the specified index and
   * subindex is not found in the map.
   */
  virtual Parameter& findParameter(uint16_t index, uint8_t subindex) = 0;

  /**
   * @brief Uploads a parameter from the device via SDO and updates the local
   * parameter store.
   *
   * This function reads a parameter value from the device using SDO
   * communication, updates the corresponding local `Parameter` object with the
   * received value, and returns a reference to it.
   *
   * @param index The 16-bit parameter index in the object dictionary.
   * @param subindex The 8-bit subindex of the parameter.
   * @param expiryTime The maximum duration to wait for the SDO upload operation
   * to complete.
   *
   * @return Reference to the updated local `common::Parameter` object.
   *
   * @throws std::runtime_error If the upload fails or returns an empty payload.
   */
  virtual Parameter& upload(
      uint16_t index, uint8_t subindex,
      const std::chrono::steady_clock::duration expiryTime =
          std::chrono::milliseconds(5000)) = 0;

  /**
   * @brief Downloads a parameter to the device using SDO communication.
   *
   * This function sends the binary data of the specified parameter (retrieved
   * from the local store) to the device. The operation fails if the parameter's
   * data is empty.
   *
   * @param index The 16-bit index of the parameter in the object dictionary.
   * @param subindex The 8-bit subindex of the parameter.
   * @param expiryTime The maximum duration to wait for the SDO download
   * operation to complete.
   *
   * @throws std::runtime_error If the parameter data is empty or the SDO
   * download fails.
   */
  virtual void download(uint16_t index, uint8_t subindex,
                        const std::chrono::steady_clock::duration expiryTime =
                            std::chrono::milliseconds(5000)) = 0;

  /**
   * @brief Sets a parameter value in the local store and downloads it to the
   * device.
   *
   * This function updates the specified parameter in the local store with the
   * provided value, serializes it, and then performs an SDO download to
   * transfer the data to the device.
   *
   * @param index The 16-bit index of the parameter in the object dictionary.
   * @param subindex The 8-bit subindex of the parameter.
   * @param value The new parameter value to set and download.
   * @param expiryTime The maximum duration to wait for the SDO download
   * operation to complete.
   *
   * @throws std::runtime_error If the parameter data is empty after setting the
   * value, or if the SDO download operation fails.
   */
  virtual void download(uint16_t index, uint8_t subindex,
                        const ParameterValue& value,
                        const std::chrono::steady_clock::duration expiryTime =
                            std::chrono::milliseconds(5000)) = 0;

  /**
   * @brief Exchanges process data with the remote device and updates local
   * parameters accordingly.
   *
   * This function prepares the process data to send by collecting the current
   * data from parameters mapped as RxPDOs. It then sends this data and receives
   * updated process data from the remote device. The received data is used to
   * update local parameters mapped as TxPDOs.
   *
   * @throws std::runtime_error if findParameter throws (e.g., if parameter not
   * found).
   */
  virtual void exchangeProcessDataAndUpdateParameters() = 0;
};

/**
 * @brief Converts a string_view to a numeric type.
 *
 * This function uses std::from_chars to convert a given std::string_view into a
 * numeric value of type T. The conversion is done without allocating memory,
 * and the function returns true if the conversion succeeds, false otherwise.
 *
 * @tparam T The numeric type to convert the string_view to (e.g., int, long,
 * double).
 * @param sv The string_view representing the numeric string to be converted.
 * @param result The variable where the result of the conversion will be stored.
 * @return True if the conversion was successful, false otherwise.
 *
 * @note This function only supports arithmetic types (integers and
 * floating-point types).
 */
template <typename T>
bool stringViewToNumber(std::string_view sv, T& result) {
  static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");

  auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), result);
  return ec == std::errc();
}

#include <algorithm>  // for std::reverse
#include <bit>
#include <cstdint>
#include <type_traits>
#include <vector>

/**
 * @brief Converts a value of type T to a vector of bytes (uint8_t) in the
 * specified byte order.
 *
 * This function converts a trivially copyable value to its byte representation
 * and returns the result as a vector of uint8_t. The resulting byte order can
 * be controlled explicitly by passing either little-endian or big-endian.
 *
 * @tparam T The type of the value to convert. It must be trivially copyable.
 *
 * @param value The value to convert to bytes.
 * @param bigEndian If true, the returned byte vector will be in big-endian
 * order; if false, little-endian.
 *
 * @return A vector of uint8_t containing the byte representation of the value
 * in the specified byte order.
 *
 * @throws static_assert If the type T is not trivially copyable.
 */
template <typename T>
std::vector<uint8_t> toBytes(T value, bool bigEndian = false) {
  static_assert(std::is_trivially_copyable<T>::value,
                "T must be trivially copyable");

  std::vector<uint8_t> bytes(sizeof(T));
  uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&value);

  for (size_t i = 0; i < sizeof(T); ++i) {
    bytes[i] = bytePtr[i];
  }

  if (bigEndian) {
    std::reverse(bytes.begin(), bytes.end());
  }

  return bytes;
}

/**
 * @brief Formats the given index and subindex into a parameter identifier
 * string.
 *
 * This function takes an index and a subindex and formats them into a string
 * of the form "0xINDEX:SUBINDEX". The index is formatted as a 4-digit
 * hexadecimal number, and the subindex is formatted as a 2-digit hexadecimal
 * number.
 *
 * @param index The 16-bit index value (e.g., 0x2030).
 * @param subindex The 8-bit subindex value (e.g., 0x01).
 * @return std::string The formatted parameter ID string in the format
 * "0xINDEX:SUBINDEX".
 */
inline std::string makeParameterId(int index, int subindex) {
  std::stringstream oss;
  // Format index and subindex into "0xINDEX:SUBINDEX"
  oss << "0x" << std::setw(4) << std::setfill('0') << std::hex << std::uppercase
      << index << ":" << std::setw(2) << std::setfill('0') << std::hex
      << std::uppercase << subindex;

  return oss.str();
}

/**
 * @brief Converts a vector of bytes into a space-separated hexadecimal string.
 *
 * Each byte is formatted as "0xXX" using lowercase hexadecimal digits,
 * with two-digit zero-padded formatting. The resulting string has each
 * byte separated by a single space.
 *
 * Example:
 *   Input:  {0xFF, 0x01, 0x0a}
 *   Output: "0xff 0x01 0x0a"
 *
 * @param data The vector of bytes to convert.
 * @return A string representing the bytes in hexadecimal format.
 */
inline std::string bytesToHexString(const std::vector<uint8_t>& data) {
  std::ostringstream oss;
  for (uint8_t byte : data) {
    oss << "0x" << std::hex << std::setw(2) << std::setfill('0')
        << static_cast<int>(byte) << " ";
  }
  return oss.str();
}

/**
 * @brief Reads the contents of a binary file into a vector of uint8_t.
 *
 * This function opens the specified file in binary mode and reads its content
 * into a `std::vector<uint8_t>`. It uses `std::istreambuf_iterator` to read
 * the entire file efficiently. If the file cannot be opened, a
 * `std::runtime_error` is thrown.
 *
 * @param filename The path to the file to be read.
 * @return A `std::vector<uint8_t>` containing the file's content.
 * @throws std::runtime_error If the file cannot be opened.
 */
std::vector<uint8_t> readBinaryFile(const std::string& filename);

/**
 * @brief Joins a list of strings into a single string with a delimiter.
 *
 * Concatenates all elements in the given vector of strings, inserting the
 * specified delimiter between each element. If the list is empty, returns
 * an empty string.
 *
 * @param list The vector of strings to join.
 * @param delimiter The delimiter to insert between each string.
 * @return A single string composed of the input strings separated by the
 * delimiter.
 */
std::string joinStrings(const std::vector<std::string>& list,
                        const std::string& delimiter);

/**
 * @brief Formats a MAC address string to ensure each component is two digits
 * and uppercase.
 *
 * This function takes a MAC address string in formats such as "a-b-c-d-e-f" or
 * "a:b:c:d:e:f", and returns a standardized format like "0A:0B:0C:0D:0E:0F". It
 * ensures that each component is two characters long (adding leading zeros if
 * necessary), uses ':' as the delimiter, and converts all letters to uppercase.
 *
 * @param originalMacAddress The input MAC address string, using either ':' or
 * '-' as delimiter.
 * @return A formatted MAC address string with two-digit uppercase hexadecimal
 * components, separated by colons.
 */
std::string formatMacAddress(const std::string& originalMacAddress);

}  // namespace common

/**
 * @brief Specialization of std::hash for std::pair<uint16_t, uint8_t>.
 *
 * This specialization allows `std::unordered_map` to use `std::pair<uint16_t,
 * uint8_t>` as a key type. It combines the hash values of the `first` and
 * `second` elements of the pair to create a unique hash value.
 *
 * The hash is computed by XORing the hash values of the two elements, with the
 * second element's hash shifted to reduce collisions.
 */
namespace std {
template <>
struct hash<std::pair<uint16_t, uint8_t>> {
  /**
   * @brief Computes a hash for a given std::pair<uint16_t, uint8_t>.
   *
   * This function combines the hash values of the `first` and `second` members
   * of the pair to generate a hash suitable for use in unordered containers
   * like `std::unordered_map`.
   *
   * @param p The pair to be hashed.
   * @return The computed hash value.
   */
  size_t operator()(const std::pair<uint16_t, uint8_t>& p) const {
    return hash<uint16_t>()(p.first) ^ (hash<uint8_t>()(p.second) << 1);
  }
};
}  // namespace std
