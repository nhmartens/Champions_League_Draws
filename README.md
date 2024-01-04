# Champions League Draw of the Round of 16

This repository uses an algorithm suggested in https://github.com/eminga/cldraw in JS, Python and C++. This is my first C++ project and the main goal was to improve my C++ skills, hence the code is far from optimized.
Based on UEFA's rules for the draw of the round of 16, draw probabilities are not uniform. Group winners are matched only with runner-ups from other groups and from other countries.
To compute the probabilities of the pairings the algorithm iterates over all possible draw sequences and computes the probabilities using the law of total probability.
On my hardware the original code in JS took on average 20.5ms for the draw of the season 2023/2024. The Python version took 165ms on average and the C++ version took 8ms.

| C++ | JavaScript | Python |
|---:|---:|---:|
| 8ms | 21ms | 160ms|
