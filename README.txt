Проект собирается с помощью утилиты CMake

Требования:
CMake (https://cmake.org/)
Make (choco install make)
Mingw (choco install mingw)*

*choco - утилита Chocolatey (https://chocolatey.org/)

Сборка:

1) Перейти в директорию Antivirus_Service/build, следующие шаги выполнять в этой директории
2) Написать в терминал команду:
cmake .. -G "MinGW Makefiles"
3) После конца конфигурации проекта написать команду:
cmake --build .
4) После выполнения команд получаем исполняемый файл Antivirus_Service/build/Antivirus_Service.exe, это и будет служба