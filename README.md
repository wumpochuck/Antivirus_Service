<h1 align=center>С++ Antivirus Service</h1>

## Project Structure

- `service`
  - `apps` 
    - `antimalware_service_app` - Основная логика проекта
    - `starter` - Стартер клиента
  - `include` - Заголовочные файлы проекта
  - `lib` - Статические библиотеки 


## Getting Started

1. Убедитесь, что у вас установлен CMake и Visual Studio.
2. Откройте командную строку и перейдите в директорию `service`:
    ```sh
    cd service
    ```
3. Создайте директорию для сборки и перейдите в нее:
    ```sh
    mkdir build
    cd build
    ```
4. Запустите CMake для генерации файлов сборки:
    ```sh
    cmake ..
    ```
5. Соберите проект с помощью Cmake:
   ```sh
   cmake --build . --configure Release
   ```
Готовый проект будет собран в директории `build`


## Settings:

- В файле `service/include.config.h` требуется указать:
1) IP-адрес и PORT сервера
2) Проверить корректность путей


## Запуск:
- Установить службу ("start= auto" нужен для того чтобы служба запускалась при входе в систему: 
```powershell
sc create AntivirusService binPath= "path/to/AntivirusService.exe" start= auto
```

- Запустить службу:
```powershell
sc start AntivirusService
```

- Удалить службу:
Остановить через диспетчер задач, затем:
```powershell
sc delete AntivirusService

```

## Technologies
- CMake 
- C/C++ 
- libcurl 
- Собственные библиотеки
- Perl 
- OpenSSL 
- Zlib 
- LDAP 
- Windows специфичные библиотеки 
- CPack 


## Contributors

<table>
    <tbody>
        <tr>
            <td>
                <img width=50 src="https://avatars.githubusercontent.com/u/130181963"/>
            </td>
            <td>
                <a href = "t.me/wumpochuck"><b>wumpochuck</b></a>
                <br>
            </td>
            <td>
                <img width=50 src="https://avatars.githubusercontent.com/u/85567113?v=4"/>
            </td>
            <td>
                <a href = "https://github.com/yokkochka"><b>yokkochka</b></a>
                <br>
            </td>
            <td>
                <img width=50 src="https://avatars.githubusercontent.com/u/153612706?v=4"/>
            </td>
            <td>
                <a href = "https://github.com/Na-Nd"><b>Na-Nd</b></a>
                <br>
            </td>
        </tr>
    </tbody>
</table>
