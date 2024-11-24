# Antivirus Service Project

This project is built using the CMake utility.

## Requirements:
- [CMake](https://cmake.org/)
- Make (install via Chocolatey: `choco install make`)
- Mingw (install via Chocolatey: `choco install mingw`)

*Note: Chocolatey is a package manager for Windows. More information can be found [here](https://chocolatey.org/).*

## Build Instructions:

1. Navigate to the `Antivirus_Service/build` directory. The following steps should be executed in this directory.
2. Run the following command in the terminal:
   ```sh
   cmake .. -G "MinGW Makefiles"
   ```
3. After the project configuration is complete, run the following command:
   ```sh
   cmake --build .
   ```
4. Upon completion, the executable file __Antivirus_Service/build/Antivirus_Service.exe__ will be generated. This is the service executable.

## Service Management Instructions:

### Adding the Service
To add the compiled service to the system, run the following command in the terminal:
```sh
sc create AntivirusService binPath= "path\to\Antivirus_Service.exe"
```

### Starting the Service
To start the service, run the following command:
```sh
sc start AntivirusService
```

### Stopping the Service
To stop the service, run the following command:
```sh
sc stop AntivirusService
```

### Deleting the Service
To delete the service from the system, run the following command:
```sh
sc delete AntivirusService
```