// Copyright (c) 2013 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/common/crash_reporter/crash_reporter.h"

#include "atom/browser/browser.h"
#include "atom/common/atom_version.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "content/public/common/content_switches.h"

namespace crash_reporter {

CrashReporter::CrashReporter() {
  auto cmd = base::CommandLine::ForCurrentProcess();
  is_browser_ = cmd->GetSwitchValueASCII(switches::kProcessType).empty();
}

CrashReporter::~CrashReporter() {
}

void CrashReporter::Start(const std::string& product_name,
                          const std::string& company_name,
                          const std::string& submit_url,
                          const std::string& temp_path,
                          bool auto_submit,
                          bool skip_system_crash_handler,
                          const StringMap& extra_parameters) {
  SetUploadParameters(extra_parameters);

  InitBreakpad(product_name, ATOM_VERSION_STRING, company_name, submit_url,
               temp_path, auto_submit, skip_system_crash_handler);
}

base::FilePath CrashReporter::GetCrashesDirectory(
    const std::string& product_name, const std::string& temp_dir) {
  return base::FilePath(temp_dir).Append(product_name + " Crashes");
}

void CrashReporter::SetUploadParameters(const StringMap& parameters) {
  upload_parameters_ = parameters;
  upload_parameters_["process_type"] = is_browser_ ? "browser" : "renderer";

  // Setting platform dependent parameters.
  SetUploadParameters();
}

std::vector<CrashReporter::UploadReportResult>
CrashReporter::GetUploadedReports(const std::string& product_name,
                                  const std::string& temp_path) {
  std::vector<CrashReporter::UploadReportResult> result;

  base::FilePath uploads_path =
      GetCrashesDirectory(product_name, temp_path).Append("uploads.log");
  std::string file_content;
  if (base::ReadFileToString(uploads_path, &file_content)) {
    std::vector<std::string> reports = base::SplitString(
        file_content, "\n", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
    for (const std::string& report : reports) {
      std::vector<std::string> report_item = base::SplitString(
          report, ",", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
      int report_time = 0;
      if (report_item.size() >= 2 && base::StringToInt(report_item[0],
            &report_time)) {
        result.push_back(CrashReporter::UploadReportResult(report_time,
            report_item[1]));
      }
    }
  }
  return result;
}

void CrashReporter::InitBreakpad(const std::string& product_name,
                                 const std::string& version,
                                 const std::string& company_name,
                                 const std::string& submit_url,
                                 const std::string& temp_path,
                                 bool auto_submit,
                                 bool skip_system_crash_handler) {
}

void CrashReporter::SetUploadParameters() {
}

#if defined(OS_MACOSX) && defined(MAS_BUILD)
// static
CrashReporter* CrashReporter::GetInstance() {
  static CrashReporter crash_reporter;
  return &crash_reporter;
}
#endif

}  // namespace crash_reporter
