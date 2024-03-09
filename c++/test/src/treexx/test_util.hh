// Copyright (C) 2013-2024, Roman Panov (roman.a.panov@gmail.com).

#ifndef TREXX_TESTUTIL_HH
#define TREXX_TESTUTIL_HH

#include <treexx/compare_result.hh>

namespace treexx
{

struct Test_util
{
  template<class X, class Y>
  [[nodiscard]] static Compare_result compare(X const& x, Y const& y) noexcept
  {
    if(x < y)
    {
      return Compare_result::less;
    }
    return y < x ? Compare_result::greater : Compare_result::equal;
  }
};

} // namespace treexx

#endif // TREXX_TESTUTIL_HH
