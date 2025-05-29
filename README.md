# ethernet_client_examples

This repository contains ethernet_client, a static C++ library for communicating with SOMANET Integro devices, along with usage examples.

You can find more information about this library in our [official documentation](https://synapticon.github.io/motion_master/md_libs_ethernet_client_README.html).

## Building on Windows 11

**Required Software:**

- Git
- Visual Studio 2022
- CMake
- Ninja

To temporarily enable `cl.exe` in your PowerShell 7 (x64) session, follow these steps:

1. Open **PowerShell 7 (x64)**.
2. Run the following command to initialize the Visual Studio environment:

   ```pwsh
   cmd /c '"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 && powershell'
   ```

3. Once the new PowerShell session starts, verify that cl.exe is available by running:

   ```pwsh
   cl
   ```

> ℹ️ The previous steps are necessary because the Developer Command Prompt for VS 2022 may default to the 32-bit toolset, causing linker failures in 64-bit builds. You can verify this by running `cl` and checking if the output includes **for x86**.

### Steps

```pwsh
git clone https://github.com/synapticon/ethernet_client_examples.git
cd ethernet_client_examples
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
