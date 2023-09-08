#include "tachyon/build/cc_writer.h"

#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/substitute.h"

#include "tachyon/base/strings/string_util.h"
#include "tachyon/build/generator_util.h"

namespace tachyon::build {

namespace {

base::FilePath BazelOutToHdrPath(const base::FilePath& out) {
  std::vector<std::string> components = out.GetComponents();
  base::FilePath header_path(absl::StrJoin(components.begin() + 3,
                                           components.end() - 1,
                                           base::FilePath::kSeparators));
  header_path = header_path.Append(
      absl::StrCat(out.BaseName().RemoveExtension().value(), ".h"));
  return header_path;
}

std::string BazelOutToHdrGuardMacro(const base::FilePath& out) {
  std::vector<std::string> components = out.GetComponents();
  base::FilePath header_path(absl::StrJoin(components.begin() + 3,
                                           components.end() - 1,
                                           base::FilePath::kSeparators));
  // In case of .cu.h, it removes extension twice.
  base::FilePath basename = out.BaseName().RemoveExtension().RemoveExtension();
  return base::ToUpperASCII(absl::StrCat(
      absl::StrJoin(components.begin() + 3, components.end() - 1, "_"),
      absl::Substitute("_$0_H_", basename.value())));
}

}  // namespace

base::FilePath CcWriter::GetHdrPath() const { return BazelOutToHdrPath(out); }

int CcWriter::WriteHdr(const std::string& content) const {
  std::string_view tpl[] = {
      "// This is generated by %{generator}",
      "#ifndef %{header_guard_macro}",
      "#define %{header_guard_macro}",
      "",
      "%{content}",
      "",
      "#endif  // %{header_guard_macro}",
      "",
  };
  std::string tpl_content = absl::StrJoin(tpl, "\n");

  std::string text = absl::StrReplaceAll(
      tpl_content, {
                       {"%{generator}", generator},
                       {"%{header_guard_macro}", BazelOutToHdrGuardMacro(out)},
                       {"%{content}", content},
                   });
  return Write(text);
}

int CcWriter::WriteSrc(const std::string& content) const {
  std::vector<std::string_view> tpl = {
      "// This is generated by %{generator}",
      "#include \"%{header_path}\"",
      "",
      "%{content}"
      "",
  };
  std::string tpl_content = absl::StrJoin(tpl, "\n");

  std::string text = absl::StrReplaceAll(
      tpl_content, {
                       {"%{generator}", generator},
                       {"%{header_path}", GetHdrPath().value()},
                       {"%{content}", content},
                   });
  return Write(text);
}

}  // namespace tachyon::build