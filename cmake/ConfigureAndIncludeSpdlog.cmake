option(SPDLOG_ENABLE_PCH "" ON)
option(SPDLOG_DISABLE_DEFAULT_LOGGER "" ON)
option(SPDLOG_FMT_EXTERNAL "" OFF)
option(SPDLOG_FMT_EXTERNAL_HO "" OFF)
option(SPDLOG_NO_THREAD_ID "" ON)
option(SPDLOG_WCHAR_FILENAMES "" ON)
option(SPDLOG_WCHAR_SUPPORT "" ON)

add_subdirectory(deps/spdlog)
set_target_properties(spdlog PROPERTIES FOLDER "Dependencies")

mark_as_advanced(
  SPDLOG_BUILD_ALL
  SPDLOG_BUILD_BENCH
  SPDLOG_BUILD_EXAMPLE
  SPDLOG_BUILD_EXAMPLE_HO
  SPDLOG_BUILD_SHARED
  SPDLOG_BUILD_TESTS
  SPDLOG_BUILD_TESTS_HO
  SPDLOG_BUILD_WARNINGS
  SPDLOG_CLOCK_COARSE
  SPDLOG_DISABLE_DEFAULT_LOGGER
  SPDLOG_ENABLE_PCH
  SPDLOG_FMT_EXTERNAL
  SPDLOG_FMT_EXTERNAL_HO
  SPDLOG_INSTALL
  SPDLOG_NO_ATOMIC_LEVELS
  SPDLOG_NO_EXCEPTIONS
  SPDLOG_NO_THREAD_ID
  SPDLOG_NO_TLS
  SPDLOG_PREVENT_CHILD_FD
  SPDLOG_SANITIZE_ADDRESS
  SPDLOG_TIDY
  SPDLOG_USE_STD_FORMAT
  SPDLOG_WCHAR_FILENAMES
  SPDLOG_WCHAR_SUPPORT
)
