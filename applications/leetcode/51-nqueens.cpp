#include <sstream>
#include <vector>
#include <array>
#include <numeric>

using namespace std;

class Solution {
public:
    template <typename T>
    using arr = array<T, 96>;
    template <typename T>
    using arr_short = array<T, 16>;

    vector<string> answer(const arr<bool> &sol, uint8_t n) {
        vector<string> ans;

        for (size_t ri = 0; ri < n; ++ri) {
            stringstream ss;
            for (size_t ci = 0; ci < n; ++ci) {
                uint8_t index = ri * n + ci;
                ss << (sol[index] ? 'Q' : '.');
            }
            ans.push_back(ss.str());
        }

        return ans;
    }

    void solveNQueensRec(uint8_t left, uint8_t size, const arr_short<bool> &row_wise,
                         const arr<bool> &queens, const arr<bool> &restricted,
                         vector<vector<string>> &ans) {

        if (left == 0) {
            if (std::accumulate(queens.begin(), queens.end(), 0) != size) {
                return;
            }

            ans.push_back(answer(queens, size));
            return;
        }

        for (uint8_t ri = 0; ri < size; ++ri) {
            if (row_wise[ri]) {
                return;
            }

            for (uint8_t ci = 0; ci < size; ++ci) {

                if (restricted[ri * size + ci]) {
                    continue;
                }

                auto res_copy = restricted;

                for (uint8_t cii = 0; cii < size; ++cii) {
                    res_copy[ri * size + cii] = true;
                }

                for (uint8_t rii = 0; rii < size; ++rii) {
                    res_copy[rii * size + ci] = true;
                }

                int mdr = ri, mdc = ci;
                while (mdr < size && mdc < size) {
                    res_copy[mdr * size + mdc] = true;
                    ++mdr;
                    ++mdc;
                }

                mdr = ri, mdc = ci;
                while (mdr >= 0 && mdc >= 0) {
                    res_copy[mdr * size + mdc] = true;
                    --mdr;
                    --mdc;
                }

                mdr = ri, mdc = ci;
                while (mdr < size && mdc < size && mdr >= 0 && mdc >= 0) {
                    res_copy[mdr * size + mdc] = true;
                    ++mdr;
                    --mdc;
                }

                mdr = ri, mdc = ci;
                while (mdr < size && mdc < size && mdr >= 0 && mdc >= 0) {
                    res_copy[mdr * size + mdc] = true;
                    --mdr;
                    ++mdc;
                }

                auto queens_copy = queens;
                auto rw_copy = row_wise;

                queens_copy[ri * size + ci] = true;
                rw_copy[ri] = true;

                solveNQueensRec(left - 1, size, rw_copy, queens_copy, res_copy, ans);
            }
        }
    }

    vector<vector<string>> solveNQueens(int n) {
        arr<bool> q, r;
        q.fill(false);
        r.fill(false);

        arr_short<bool> rw;
        rw.fill(false);

        vector<vector<string>> answer;

        solveNQueensRec(n, n, rw, q, r, answer);
        return answer;
    }
};

int main() {
    auto s = Solution();
    auto ans = s.solveNQueens(9);
    return 0;
}