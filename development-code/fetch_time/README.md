# Fetch Time

This program fetches the user's timezone from an external API, calculates the GMT and daylight savings time (DST) offsets, and displays the local time adjusted for these offsets.

## Features

- Fetches timezone data using the [ip-api.com](http://ip-api.com) API.
- Calculates GMT offset and DST adjustment based on the user's timezone.
- Displays the current local time in the appropriate timezone.

## Prerequisites

- **C++ Compiler**: Compatible with C++11 or higher.
- **Libraries**:
  - [libcurl](https://curl.se/libcurl/) for HTTP requests.
  - [nlohmann/json](https://github.com/nlohmann/json) for JSON parsing.

## Installation

1. **Clone the Repository**:
   ```bash
   git clone <repository-url>
   cd <repository-name>
   ```
2. **Install Dependencies**:
   Ensure `libcurl` and `nlohmann/json` are installed. On Ubuntu, you can install them with:
   ```bash
   sudo apt-get install libcurl4-openssl-dev
   ```
   For `nlohmann/json`, include the single-header file in your project:
   ```bash
   wget https://github.com/nlohmann/json/releases/latest/download/json.hpp -P include/
   ```

3. **Compile the Code**:
   Use `g++` or a compatible compiler:
   ```bash
   g++ -o fetch_time main.cpp -lcurl
   ```

## Usage

Run the compiled executable:
```bash
./fetch_time
```

The program will:
1. Fetch the user's timezone from the `ip-api.com` API.
2. Calculate GMT and DST offsets.
3. Display the local time adjusted for the timezone.

### Example Output

```
Fetching timezone and DST information...
Timezone: America/New_York
GMT Offset: -18000 seconds
Daylight Offset: 0 seconds

Local Time:
Current Time: 2024-12-01 14:45:30 (America/New_York)
```

## Debugging

To enable debugging output, uncomment the debug statements in the code.

## Limitations

- Requires internet connectivity to fetch timezone data.
- Assumes the `TZ` environment variable is compatible with the fetched timezone.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

---

Happy coding! 🎉
```
