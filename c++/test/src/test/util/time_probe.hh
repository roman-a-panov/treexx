/*
The MIT License (MIT) https://opensource.org/license/mit

Copyright (c) 2013-2024 Roman Panov roman.a.panov@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef TEST_UTIL_TIMEPROBE_HH
#define TEST_UTIL_TIMEPROBE_HH

#include <chrono>

namespace test::util
{

struct Time_probe
{
  Time_probe() :
    start_time_(Clock::now())
  {}

  [[nodiscard]] auto ns() const
  {
    auto const curr_time(Clock::now());
    return ::std::chrono::duration_cast<Nanoseconds>(
      curr_time - start_time_).count();
  }

private:
  using Clock = ::std::chrono::high_resolution_clock;
  using Nanoseconds = ::std::chrono::nanoseconds;
  using Time_point = Clock::time_point;

  Time_point const start_time_;
};

} // namespace test::util

#endif // TEST_UTIL_TIMEPROBE_HH
