// Copyright (C) 2013-2024, Roman Panov (roman.a.panov@gmail.com).

#ifndef TREXX_COMPARERESULT_HH
#define TREXX_COMPARERESULT_HH

#include <cstdint>

namespace treexx
{

enum class Compare_result : ::std::uint8_t
{
  equal = 0u,
  greater,
  less
};

} // namespace treexx

#endif // TREXX_COMPARERESULT_HH
