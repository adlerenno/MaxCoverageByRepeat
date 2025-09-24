//
// Created by Enno Adler on 02.09.25.
//

#include <sdsl/int_vector.hpp>
#include <sdsl/suffix_arrays.hpp>
#include <sdsl/lcp.hpp>
#include "maxcoverage.h"

using namespace std;
using namespace sdsl;

uint64_t compute_overlap(int_vector<> sa, int_vector<> rst, int_vector<> q1, size_t pos, uint64_t lcp)
{
    // Ensure we don't go out of bounds
    if (pos + rst[pos] > sa.size())
        rst[pos] = sa.size() - pos;

    // Copy range [pos, pos+count)
    std::vector<size_t> subset(sa.begin() + q1[pos] - 1, sa.begin() + q1[pos] + rst[pos] - 1);

    // Sort the values
    std::sort(subset.begin(), subset.end());

    uint64_t overlaps = 0;
    for (size_t i = 0; i < subset.size()-1; i++)
    {
        int64_t overlap = subset[i] + lcp;
        overlap -= subset[i+1];
        overlaps += std::max(0LL, overlap);
    }

    return overlaps;
}

string compute_max_coverage(const string& test_string)
{
    cout << "Computing real substring of " << test_string << " that covers the most positions." << endl;
    //int_vector<> sa(test_string.size(), 0, bits::hi(test_string.size())+1);
    //algorithm::calculate_sa((const unsigned char*)test_string.data(), test_string.size(), sa);
    csa_bitcompressed sa;
    construct_im(sa, test_string, 1); // 1 for byte alphabet

    cout << "SA:" << endl;
    cout << "csa.size(): " << sa.size() << endl;
    cout << sa << endl;  // output CSA

    lcp_bitcompressed lcp;
    construct_im(lcp, test_string, 1); // 1 for byte alphabet

    cout << "LCP:" << endl;
    cout << "lcp.size(): " << lcp.size() << endl;
    cout << lcp << endl;  // output LCP

    int_vector<> rst(sa.size(), 0);
    int_vector<> q1(sa.size(), 0);

    // TODO: Wrong, as it not considers the equal prefixes before the exact match.
    // Stack entry: <<block begin Q1, position of max block height k>, lcp[k]>.
    std::stack<std::pair<std::pair<size_t, size_t>, uint64_t>> lcp_stack;
    // Fill stack with pairs
    for (size_t i = 0; i < lcp.size(); i++) {
        bool condition = true;
        size_t interval_start = i;
        std::pair<std::pair<size_t, size_t>, uint64_t> top;
        while (!lcp_stack.empty() and condition) {
            top = lcp_stack.top();
            if (top.second < lcp[i]){
                lcp_stack.push({{interval_start, i}, lcp[i]});
                condition = false;
            }
            if (top.second > lcp[i]) {
                rst[top.first.second] = i - top.first.first + 1;
                q1[top.first.second] = top.first.first;
                interval_start = top.first.first; // Before the block where higher lcp-blocks we need to count
                lcp_stack.pop();
            }
            if (top.second == lcp[i])
                condition = false;
        }
        if (lcp_stack.empty()) {
            lcp_stack.push({{interval_start, i}, lcp[i]});
        }
    }
    //Postprocess Stack
    while (!lcp_stack.empty())
    {
        std::pair<std::pair<size_t, size_t>, uint64_t> top;
        top = lcp_stack.top();
        rst[top.first.second] = lcp.size() - top.first.first + 1;
        lcp_stack.pop();
    }

    cout << "RST:" << endl;
    cout << "rst.size(): " << rst.size() << endl;
    cout << rst << endl;  // output RST

    // Heap storing (lcp * rst, position)
    using Pair = std::pair<uint64_t, size_t>;
    auto cmp = [](const Pair& a, const Pair& b) {
        return a.first < b.first;  // max-heap by LCP*RST value
    };
    std::priority_queue<Pair, std::vector<Pair>, decltype(cmp)> lcp_heap(cmp);

    // Fill heap
    uint64_t key;
    for (size_t i = 0; i < lcp.size(); ++i) {
        key = lcp[i] * rst[i];
        if (key > 0)
            lcp_heap.push({key, i});
    }

    if (lcp_heap.empty())
        throw -1;
    auto [max_coverage, max_cov_pos] = lcp_heap.top();
    max_coverage -= compute_overlap(sa.sa_sample, rst, q1, max_cov_pos, lcp[max_cov_pos]);

    cout << "Heap nodes (LCP[i] * RST[i], i):" << endl;
    // Process Heap
    while (!lcp_heap.empty()) {
        auto [val, pos] = lcp_heap.top();
        std::cout << "(" << val << ", " << pos << ")\n";
        if (val < max_coverage)
            break;
        max_coverage = val - compute_overlap(sa.sa_sample, rst, q1, max_cov_pos, lcp[max_cov_pos]);
        max_cov_pos = pos;
        lcp_heap.pop();
    }

    cout << "Result: " << test_string.substr(sa[max_cov_pos], lcp[max_cov_pos]) << " covers " << max_coverage << endl;
    return test_string.substr(sa[max_cov_pos], lcp[max_cov_pos]);
}