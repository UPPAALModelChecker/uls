include(FetchContent)

FetchContent_Declare(json
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)