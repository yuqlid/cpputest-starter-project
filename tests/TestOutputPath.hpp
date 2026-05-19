#pragma once

#include <filesystem>
#include <string>

namespace TestOutputPath {

/**
 * @brief テスト実行時の出力ディレクトリ初期化
 *
 * 例:
 * test_output/2026-05-19_120301/
 */
void initialize();

/**
 * @brief CSV保存用フルパス生成
 *
 * 例:
 * test_output/2026-05-19_120301/LPF/step.csv
 */
std::filesystem::path makeCsvPath(const std::string& group,
                                  const std::string& file_name);

void copyToLatest(const std::string& group, const std::string& file_name);

}  // namespace TestOutputPath