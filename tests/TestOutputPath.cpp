#include "TestOutputPath.hpp"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

constexpr const char* kRootDir = "Logs";

/**
 * @brief 実行時タイムスタンプ保存
 *
 * CppUTestリーク検出対策として
 * std::string static を避ける
 */
char g_run_timestamp[32] = {0};

void makeTimestamp(char* buffer, size_t size) {
  auto now = std::chrono::system_clock::now();
  auto t = std::chrono::system_clock::to_time_t(now);

  std::tm tm{};

#ifdef _WIN32
  localtime_s(&tm, &t);
#else
  localtime_r(&t, &tm);
#endif

  std::snprintf(buffer, size, "%04d-%02d-%02d_%02d%02d%02d", tm.tm_year + 1900,
                tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

}  // namespace

namespace TestOutputPath {

void initialize() {
  makeTimestamp(g_run_timestamp, sizeof(g_run_timestamp));

  fs::create_directories(fs::path(kRootDir) / g_run_timestamp);
}

fs::path makeCsvPath(const std::string& group, const std::string& file_name) {
  fs::path dir = fs::path(kRootDir) / g_run_timestamp / group;

  fs::create_directories(dir);

  return dir / file_name;
}

}  // namespace TestOutputPath