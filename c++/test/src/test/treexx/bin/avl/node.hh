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

#ifndef TEST_TREEXX_BIN_AVL_NODE_HH
#define TEST_TREEXX_BIN_AVL_NODE_HH

#include <algorithm>
#include <limits>
#include <type_traits>

#include <test/treexx/util/pointer.hh>
#include <treexx/bin/side.hh>
#include <treexx/bin/avl/balance.hh>

namespace test::treexx::bin::avl
{

template<class Value, class Index = void, class Offset = void>
class Node
{
  template<class T>
  using Numeric_limits_ = ::std::numeric_limits<T>;

  template<class T>
  using Decay_ = typename ::std::decay<T>::type;

  template<class T, class U>
  using Is_same_ = typename ::std::is_same<T, U>::type;

  template<class T>
  using Is_object_ = typename ::std::is_object<T>::type;

  template<class T, class... Args>
  using Is_constructible_ = typename ::std::is_constructible<T, Args...>::type;

  template<class T>
  using Pointer_ = ::test::treexx::util::Pointer<T>;

  template<bool, class = void>
  struct Enable_if_
  {};

  template<class T>
  struct Enable_if_<true, T>
  {
    using Type = T;
  };

  static_assert(Is_same_<Index, Decay_<Index>>::value);
  static_assert(Is_same_<Offset, Decay_<Offset>>::value);

  static bool constexpr has_index_ = Is_object_<Index>::value;
  static bool constexpr has_offset_ = Is_object_<Offset>::value;

public:
  using Side = ::treexx::bin::Side;
  using Balance = ::treexx::bin::avl::Balance;

  template<
    class... Val_args,
    bool e = Is_constructible_<Value, Val_args...>::value,
    class = typename Enable_if_<e>::Type>
  explicit Node(Val_args&&... val_args) :
    data_(static_cast<Val_args&&>(val_args)...)
  {}

  template<
    class... Val_args,
    bool e = Is_constructible_<Value, Val_args...>::value,
    class = typename Enable_if_<e>::Type>
  explicit Node(
    Pointer_<Node> const& parent,
    Side const side, Val_args&&... val_args) :
    data_(parent, side, static_cast<Val_args&&>(val_args)...)
  {}

  [[nodiscard]] Value& value() noexcept
  {
    return data_.value;
  }

  [[nodiscard]] Value const& value() const noexcept
  {
    return data_.value;
  }

  [[nodiscard]] Balance xyz_balance() const noexcept
  {
    return data_.balance;
  }

  [[nodiscard]] Side xyz_side() const noexcept
  {
    return data_.side;
  }

  [[nodiscard]] Pointer_<Node> const& xyz_parent() noexcept
  {
    return data_.parent;
  }

  [[nodiscard]] Pointer_<Node const> xyz_parent() const noexcept
  {
    return data_.parent;
  }

  template<Side side>
  [[nodiscard]] Pointer_<Node> const& xyz_child() noexcept
  {
    return xyz_child_<side, Pointer_<Node> const&>(data_);
  }

  template<Side side>
  [[nodiscard]] Pointer_<Node const> xyz_child() const noexcept
  {
    return xyz_child_<side, Pointer_<Node const>>(data_);
  }

  template<bool e = has_index_, class I = Index>
  [[nodiscard]] auto xyz_index(
    ) const noexcept -> typename Enable_if_<e, I const&>::Type
  {
    return data_.index;
  }

  template<bool e = has_offset_, class O = Offset>
  [[nodiscard]] auto xyz_offset(
    ) const noexcept -> typename Enable_if_<e, O const&>::Type
  {
    return data_.offset;
  }

  void set_xyz_balance(Balance const b) noexcept
  {
    data_.balance = b;
  }

  void set_xyz_side(Side const s) noexcept
  {
    data_.side = s;
  }

  void set_xyz_parent(Pointer_<Node> const& p) noexcept
  {
    data_.parent = p;
  }

  template<Side side>
  auto set_xyz_child(
    Pointer_<Node> const& c) noexcept ->
      typename Enable_if_<Side::left == side>::Type
  {
    data_.left_child = c;
  }

  template<Side side>
  auto set_xyz_child(
    Pointer_<Node> const& c) noexcept ->
      typename Enable_if_<Side::right == side>::Type
  {
    data_.right_child = c;
  }

  template<bool e = has_index_>
  auto increment_xyz_index() noexcept -> typename Enable_if_<e>::Type
  {
    ++data_.index;
  }

  template<bool e = has_index_>
  auto decrement_xyz_index() noexcept -> typename Enable_if_<e>::Type
  {
    --data_.index;
  }

  template<bool e = has_index_, class I = Index>
  auto add_to_xyz_index(
    I const& i) noexcept -> typename Enable_if_<e>::Type
  {
    data_.index += i;
  }

  template<bool e = has_index_, class I = Index>
  auto subtract_from_xyz_index(
    I const& i) noexcept -> typename Enable_if_<e>::Type
  {
    data_.index -= i;
  }

  template<unsigned i, bool e = has_index_>
  auto set_xyz_index() noexcept -> typename Enable_if_<e>::Type
  {
    data_.index = static_cast<Index>(i);
  }

  template<bool e = has_index_, class I = Index>
  auto set_xyz_index(I const& i) noexcept -> typename Enable_if_<e>::Type
  {
    data_.index = i;
  }

  template<bool e = has_offset_, class O = Offset>
  auto add_to_xyz_offset(
    O const& o) noexcept -> typename Enable_if_<e>::Type
  {
    data_.offset += o;
  }

  template<bool e = has_offset_, class O = Offset>
  auto subtract_from_xyz_offset(
    O const& o) noexcept -> typename Enable_if_<e>::Type
  {
    data_.offset -= o;
  }

  template<bool e = has_offset_, class O = Offset>
  auto set_xyz_offset(O const& o) noexcept -> typename Enable_if_<e>::Type
  {
    data_.offset = o;
  }

  void swap_xyz_aux(Node& x) noexcept
  {
    if constexpr(has_index_)
    {
      ::std::swap(data_.index, x.data_.index);
    }
    if constexpr(has_offset_)
    {
      ::std::swap(data_.offset, x.data_.offset);
    }

    ::std::swap(data_.balance, x.data_.balance);
    ::std::swap(data_.side, x.data_.side);
  }

private:
  template<class = Index, bool = has_index_>
  struct Index_base_
  {};

  template<class I>
  struct Index_base_<I, true>
  {
    Index_base_() noexcept :
      index(Numeric_limits_<I>::max() - static_cast<I>(3u))
    {}

    I index;
  };

  template<class = Offset, bool = has_offset_>
  struct Offset_base_
  {};

  template<class O>
  struct Offset_base_<O, true>
  {
    Offset_base_() noexcept :
      offset(Numeric_limits_<O>::max() - static_cast<O>(5u))
    {}

    O offset;
  };

  struct Data_ : Index_base_<>, Offset_base_<>
  {
    template<
      class... Val_args,
      bool e = Is_constructible_<Value, Val_args...>::value,
      class = typename Enable_if_<e>::Type>
    explicit Data_(Val_args&&... val_args) :
      value(static_cast<Val_args&&>(val_args)...),
      balance(Balance::overleft),
      side(Side::right)
    {}

    template<
      class... Val_args,
      bool e = Is_constructible_<Value, Val_args...>::value,
      class = typename Enable_if_<e>::Type>
    explicit Data_(
      Pointer_<Node> const& p,
      Side const s, Val_args&&... val_args) :
      parent(p),
      value(static_cast<Val_args&&>(val_args)...),
      balance(Balance::overleft),
      side(s)
    {}

    Pointer_<Node> parent;
    Pointer_<Node> left_child;
    Pointer_<Node> right_child;
    Value value;
    Balance balance;
    Side side;
  };

  template<Side side, class Ret, class Data>
  [[nodiscard]] static auto xyz_child_(
    Data& data) noexcept ->
      typename Enable_if_<Side::left == side, Ret>::Type
  {
    return data.left_child;
  }

  template<Side side, class Ret, class Data>
  [[nodiscard]] static auto xyz_child_(
    Data& data) noexcept ->
      typename Enable_if_<Side::right == side, Ret>::Type
  {
    return data.right_child;
  }

  Data_ data_;
};

} // test::treexx::bin::avl

#endif // TEST_TREEXX_BIN_AVL_NODE_HH
