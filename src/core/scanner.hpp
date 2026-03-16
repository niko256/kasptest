#pragma once

#include "entities.hpp"
#include <string_view>
#include <vector>

namespace kasptest::core {

/* @brief this component is responsible for scanning
 * the stream of content in order to find patterns.
 *
 * since input data stream is sliced into chunks, it has to handle
 * special cases when pattern is split between two chunks
 *
 * => it has to keep track of the last prefix_len_ := [max_pattern_len - 1] bytes
 * of the previous chunk
 */
class StreamScanner {
  private:
    std::vector<Infection> patterns_;
    std::vector<int> counts_;

    std::string prefix_;
    size_t prefix_len_;

  public:
    explicit StreamScanner(const std::vector<Infection>& patterns)
        : patterns_(patterns),
          counts_(patterns.size(), 0),
          prefix_len_(calculate_max_prefix_len(patterns)) {}

    void feed(std::string_view chunk) {
        if (chunk.empty()) {
            return;
        }

        std::string maybe_match = prefix_ + std::string(chunk);

        for (int i = 0; i < patterns_.size(); ++i) {
            counts_[i] += needle_count(maybe_match, patterns_[i].pattern_);
        }


        /*
         * now we want to prepare the prefix_ for the next call:
         *
         * if the total data in maybe_match is longer than prefix_len_,
         * => we take the tail of length prefix_len_ (this includes enough
         * bytes to cover any possible split pattern start).
         */
        if (prefix_len_ > 0 && maybe_match.size() > prefix_len_) {
            /*
             * if prefix_len_ = 8 and maybe_match = "oooinfectio"
             * => keep "infectio" for the next call
             */
            auto tail_start = maybe_match.size() - prefix_len_;
            // extract that tail substring
            // and
            //  store it as the new prefix
            prefix_ = maybe_match.substr(tail_start);
        } else {
            prefix_ = maybe_match;
        }
    }

    ScanResult summary() const {
        ScanResult res;

        for (int i = 0; i < patterns_.size(); ++i) {
            if (counts_[i] > 0) {
                res.threats_.push_back({patterns_[i].identifier_, counts_[i]});
            }
        }

        return res;
    }

  private:
    /* here we're go again... :)
     * https://leetcode.com/problems/find-the-index-of-the-first-occurrence-in-a-string/description/
     * */
    int needle_count(std::string_view haystack, std::string_view needle) {
        if (needle.empty()) {
            return 0;
        }

        size_t count = 0;
        size_t pos = 0;

        while (pos <= haystack.size() - needle.size()) {
            auto found_at = haystack.find(needle, pos);

            if (found_at != std::string_view::npos) {
                ++count;
                pos = needle.size() + found_at;
            } else {
                break;
            }
        }
        return count;
    }

    static size_t calculate_max_prefix_len(const std::vector<Infection>& patterns) {
        size_t max_len = 0;

        for (auto& patt : patterns) {
            max_len = std::max(max_len, patt.pattern_.size());
        }
        return max_len > 0 ? max_len - 1 : 0;
    }
};


}  // namespace kasptest::core
