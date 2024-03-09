// Copyright (C) 2013-2024, Roman Panov (roman.a.panov@gmail.com).

#ifndef TREXX_AVL_BALANCE_HH
#define TREXX_AVL_BALANCE_HH

#include <cstdint>

namespace treexx::avl
{

enum class Balance : ::std::uint8_t
{
  poised = 0u,
  overleft,
  overright
};

} // namespace treexx::avl

#endif // TREXX_AVL_BALANCE_HH
