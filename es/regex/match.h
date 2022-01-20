#ifndef ES_REGEX_MATCH
#define ES_REGEX_MATCH

#include <es/utils/helper.h>

namespace es {
namespace regex {

struct State {
  size_t end_index;
  std::vector<std::u16string> captures;
};

struct MatchResult {
 public:
  static MatchResult Failed() { return MatchResult(true, {}); }

  MatchResult(bool failed, State state) :
    failed(failed), state(state) {}

  bool failed;
  State state;
};

MatchResult SplitMatch(std::u16string S, size_t q, std::u16string R) {
  size_t r = R.size();
  size_t s = S.size();
  if (r + q > s) return MatchResult::Failed();
  if (S.substr(q, r) != R)
    return MatchResult::Failed();
  std::vector<std::u16string> cap;
  return MatchResult(false, {q+r, cap});
}

}  // regex
}  // namespace

#endif  // ES_REGEX_MATCH
