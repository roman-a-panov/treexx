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

#ifndef TEST_UTIL_REGCLASS_HH
#define TEST_UTIL_REGCLASS_HH

#include <memory>

namespace test::util
{

template<class T>
struct Reg_class
{
  using Test = T;

  static void const* reg() noexcept
  {
    return ::std::addressof(registrar_);
  }

private:
  struct Registrar
  {
    Registrar()
    {
      Test::register_test_cases();
    }

  private:
    static void run()
    {
      Test test;
      test.run();
    }
  };

  static Registrar registrar_;
};

template<class T>
typename Reg_class<T>::Registrar Reg_class<T>::registrar_;

} // namespace test::util

#endif // TEST_UTIL_REGCLASS_HH
