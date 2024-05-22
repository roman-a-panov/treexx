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

#ifndef TREEXX_BIN_AVL_TREEALGO_HH
#define TREEXX_BIN_AVL_TREEALGO_HH

#include <memory>
#include <type_traits>

#include <treexx/assert.hh>
#include <treexx/compare_result.hh>
#include <treexx/bin/side.hh>
#include <treexx/bin/tree_algo.hh>
#include <treexx/bin/avl/balance.hh>

namespace treexx::bin::avl
{

struct Tree_algo : ::treexx::bin::Tree_algo
{
private:
  using Compare_result_ = ::treexx::Compare_result;

  template<class... T>
  using Void_ = ::std::void_t<T...>;

  template<class T>
  using Is_object_ = typename ::std::is_object<T>::type;

  template<class T>
  using Remove_cv_ = typename ::std::remove_cv<T>::type;

  template<class T>
  using Remove_reference_ = typename ::std::remove_reference<T>::type;

  template<class T>
  using Remove_cv_ref_ = Remove_cv_<Remove_reference_<Remove_cv_<T>>>;

  template<bool, class = void>
  struct Enable_if_
  {};

  template<class T>
  struct Enable_if_<true, T>
  {
    using Type = T;
  };

  template<class, class = Void_<>>
  struct Index_trait_
  {
    static bool constexpr value = false;
  };

  template<class Tree>
  struct Index_trait_<Tree, Void_<typename Remove_cv_ref_<Tree>::Index>>
  {
    using Type = typename Remove_cv_ref_<Tree>::Index;
    static bool constexpr value = Is_object_<Type>::value;
  };

  template<class, class = Void_<>>
  struct Offset_trait_
  {
    static bool constexpr value = false;
  };

  template<class Tree>
  struct Offset_trait_<Tree, Void_<typename Remove_cv_ref_<Tree>::Offset>>
  {
    using Type = typename Remove_cv_ref_<Tree>::Offset;
    static bool constexpr value = Is_object_<Type>::value;
  };

public:
  using Side = ::treexx::bin::Side;

  template<class Tree>
  using Index = typename Index_trait_<Tree>::Type;

  template<class Tree>
  using Offset = typename Offset_trait_<Tree>::Type;

  template<
    bool with_node = true,
    bool with_index = false,
    bool with_offset = false,
    class Tree,
    class Compare>
  [[nodiscard]] static auto binary_search(
    Tree&& tree,
    Compare&& compare) ->
      typename Enable_if_<
        (!with_index || Index_trait_<Tree>::value) &&
        (!with_offset || Offset_trait_<Tree>::value),
        Node_pointer<Tree>>::Type
  {
    return binary_search_<
      Binary_search_type_::any_match,
      with_node, with_index, with_offset, false>(
        static_cast<Tree&&>(tree), static_cast<Compare&&>(compare));
  }

  template<
    bool with_node = true,
    bool with_index = false,
    bool with_offset = false,
    bool unique = false,
    class Tree,
    class Compare>
  [[nodiscard]] static auto lower_bound(
    Tree&& tree,
    Compare&& compare) ->
      typename Enable_if_<
        (!with_index || Index_trait_<Tree>::value) &&
        (!with_offset || Offset_trait_<Tree>::value),
        Node_pointer<Tree>>::Type
  {
    return binary_search_<
      Binary_search_type_::lower_bound,
      with_node, with_index, with_offset, unique>(
        static_cast<Tree&&>(tree), static_cast<Compare&&>(compare));
  }

  template<
    bool with_node = true,
    bool with_index = false,
    bool with_offset = false,
    class Tree,
    class Compare>
  [[nodiscard]] static auto upper_bound(
    Tree&& tree,
    Compare&& compare) ->
      typename Enable_if_<
        (!with_index || Index_trait_<Tree>::value) &&
        (!with_offset || Offset_trait_<Tree>::value),
        Node_pointer<Tree>>::Type
  {
    return binary_search_<
      Binary_search_type_::upper_bound,
      with_node, with_index, with_offset, false>(
        static_cast<Tree&&>(tree), static_cast<Compare&&>(compare));
  }

  template<class Tree>
  [[nodiscard]] static Node_pointer<Tree> at_index(
    Tree&& tree,
    typename Index_trait_<Tree>::Type const& idx)
  {
    return
      binary_search_<Binary_search_type_::any_match, false, true, false, false>(
        static_cast<Tree&&>(tree),
        [&idx](auto const& node_idx) noexcept -> Compare_result_
        {
          if(node_idx < idx)
          {
            return Compare_result::less;
          }
          if(idx < node_idx)
          {
            return Compare_result::greater;
          }
          return Compare_result::equal;
        });
  }

  template<class Tree>
  [[nodiscard]] static auto node_index(
    Tree&& tree,
    Node<Tree>& node) noexcept -> typename Index_trait_<Tree>::Type
  {
    return node_index_or_offset<false>(static_cast<Tree&&>(tree), node);
  }

  template<class Tree>
  [[nodiscard]] static auto node_offset(
    Tree&& tree,
    Node<Tree>& node) noexcept -> typename Offset_trait_<Tree>::Type
  {
    return node_index_or_offset<true>(static_cast<Tree&&>(tree), node);
  }

  template<class Tree>
  static auto push_back(
    Tree&& tree,
    Node_pointer<Tree> const& node_ptr) noexcept ->
      typename Enable_if_<!Offset_trait_<Tree>::value>::Type
  {
    push_<Side::right>(static_cast<Tree&&>(tree), node_ptr);
  }

  template<class Tree>
  static void push_back(
    Tree&& tree,
    Node_pointer<Tree> const& node_ptr,
    typename Offset_trait_<Tree>::Type const& offset) noexcept
  {
    push_<Side::right>(static_cast<Tree&&>(tree), node_ptr, offset);
  }

  template<class Tree>
  static auto push_front(
    Tree&& tree,
    Node_pointer<Tree> const& node_ptr) noexcept ->
      typename Enable_if_<!Offset_trait_<Tree>::value>::Type
  {
    push_<Side::left>(static_cast<Tree&&>(tree), node_ptr);
  }

  template<class Tree>
  static auto insert(
    Tree&& tree,
    Node_pointer<Tree> const& spot_ptr,
    Node_pointer<Tree> const& node_ptr) noexcept ->
      typename Enable_if_<!Offset_trait_<Tree>::value>::Type
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    bool constexpr has_index = Index_trait_<Tree>::value;

    if(spot_ptr)
    {
      Node* node = static_cast<Tree&&>(tree).address(node_ptr);
      Node* const spot = static_cast<Tree&&>(tree).address(spot_ptr);
      TREEXX_ASSERT(node_ptr);
      TREEXX_ASSERT(node);
      TREEXX_ASSERT(spot);

      Node_pointer const left_child_ptr(
        static_cast<Tree&&>(tree).template child<Side::left>(*spot));
      Node_pointer parent_ptr;
      Side side;
      bool is_leftmost = false;

      if(left_child_ptr)
      {
        parent_ptr = left_child_ptr;
        side = Side::right;
        for(;;)
        {
          Node* const parent = static_cast<Tree&&>(tree).address(parent_ptr);
          TREEXX_ASSERT(parent);
          Node_pointer const next_parent_ptr(
            static_cast<Tree&&>(tree).template child<Side::right>(*parent));
          if(next_parent_ptr)
          {
            parent_ptr = next_parent_ptr;
          }
          else
          {
            break;
          }
        }
      }
      else
      {
        parent_ptr = spot_ptr;
        side = Side::left;
        is_leftmost =
          spot_ptr == static_cast<Tree&&>(tree).template extreme<Side::left>();
      }

      static_cast<Tree&&>(tree).set_parent(*node, parent_ptr);
      static_cast<Tree&&>(tree).set_balance(*node, Balance::poised);
      static_cast<Tree&&>(tree).set_side(*node, side);
      static_cast<Tree&&>(tree).template set_child<Side::left>(*node, nullptr);
      static_cast<Tree&&>(tree).template set_child<Side::right>(*node, nullptr);
      if (is_leftmost)
      {
        static_cast<Tree&&>(tree).template set_extreme<Side::left>(node_ptr);
        if constexpr(has_index)
        {
          static_cast<Tree&&>(tree).template set_index<0u>(*node);
        }
      }
      else
      {
        if constexpr(has_index)
        {
          static_cast<Tree&&>(tree).template set_index<1u>(*node);
        }
      }

      if constexpr(has_index)
      {
        node = spot;
        static_cast<Tree&&>(tree).increment_index(*node);
        for(;;)
        {
          Node_pointer const next_node_ptr(
            static_cast<Tree&&>(tree).parent(*node));
          if(next_node_ptr)
          {
            Node* const next_node =
              static_cast<Tree&&>(tree).address(next_node_ptr);
            TREEXX_ASSERT(next_node);
            Side const from_side(static_cast<Tree&&>(tree).side(*node));
            node = next_node;
            if(Side::left == from_side)
            {
              static_cast<Tree&&>(tree).increment_index(*node);
            }
          }
          else
          {
            break;
          }
        }
      }

      attach_and_fix_up_(
        static_cast<Tree&&>(tree),
        parent_ptr, node_ptr, side);
    }
    else
    {
      push_<Side::right>(static_cast<Tree&&>(tree), node_ptr);
    }
  }

  template<class Tree, class Compare, class Create_node>
  static auto try_insert(
    Tree&& tree,
    Compare&& compare,
    Create_node&& create_node) ->
      typename Enable_if_<!Offset_trait_<Tree>::value, Node_pointer<Tree>>::Type
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    Node* parent = nullptr;
    Node_pointer parent_ptr(static_cast<Tree&&>(tree).root());
    Node_pointer const& parent_ptr_const = parent_ptr;
    Side side = Side::left;
    bool has_root = parent_ptr_const ? true : false;
    bool is_rightmost = true;
    bool is_leftmost = true;
    bool constexpr has_index = Index_trait_<Tree>::value;

    if(has_root)
    {
      Node_pointer child_ptr;
      for(;;)
      {
        parent =
          static_cast<Tree&&>(tree).address(parent_ptr_const);
        TREEXX_ASSERT(parent);
        Compare_result_ const cmp = compare(*parent);

        switch(cmp)
        {
        case Compare_result::equal:
          return parent_ptr;
        case Compare_result::greater:
          child_ptr =
            static_cast<Tree&&>(tree).template child<Side::left>(*parent);
          is_rightmost = false;
          side = Side::left;
          break;
        case Compare_result::less:
          child_ptr =
            static_cast<Tree&&>(tree).template child<Side::right>(*parent);
          is_leftmost = false;
          side = Side::right;
          break;
        }

        if(child_ptr)
        {
          parent_ptr = child_ptr;
        }
        else
        {
          break;
        }
      }
    }

    Node_pointer const node_ptr =
      static_cast<Create_node&&>(create_node)(parent_ptr, side);
    Node* const node = static_cast<Tree&&>(tree).address(node_ptr);
    TREEXX_ASSERT(node_ptr);
    TREEXX_ASSERT(node);

    static_cast<Tree&&>(tree).template set_child<Side::left>(*node, nullptr);
    static_cast<Tree&&>(tree).template set_child<Side::right>(*node, nullptr);
    static_cast<Tree&&>(tree).set_balance(*node, Balance::poised);

    if(is_leftmost)
    {
      static_cast<Tree&&>(tree).template set_extreme<Side::left>(node_ptr);
      if constexpr(has_index)
      {
        static_cast<Tree&&>(tree).template set_index<0u>(*node);
      }
    }
    else if constexpr(has_index)
    {
      static_cast<Tree&&>(tree).template set_index<1u>(*node);
    }

    if(is_rightmost)
    {
      static_cast<Tree&&>(tree).template set_extreme<Side::right>(node_ptr);
    }

    if(has_root)
    {
      if constexpr(has_index)
      {
        for(Side from_side = side;;)
        {
          if(Side::left == from_side)
          {
            static_cast<Tree&&>(tree).increment_index(*parent);
          }

          Node_pointer const next_parent_ptr =
            static_cast<Tree&&>(tree).parent(*parent);
          if(next_parent_ptr)
          {
            from_side = static_cast<Tree&&>(tree).side(*parent);
            parent = static_cast<Tree&&>(tree).address(next_parent_ptr);
          }
          else
          {
            break;
          }
        }
      }

      attach_and_fix_up_(static_cast<Tree&&>(tree), parent_ptr, node_ptr, side);
    }
    else
    {
      static_cast<Tree&&>(tree).set_root(node_ptr);
    }

    return node_ptr;
  }

  template<class Tree>
  static auto insert_at_index(
    Tree&& tree,
    Node_pointer<Tree> const& node_ptr,
    typename Index_trait_<Tree>::Type const& idx) noexcept ->
      typename Enable_if_<!Offset_trait_<Tree>::value>::Type
  {
    using Node_pointer = Tree_algo::Node_pointer<Tree>;
    using Node = Tree_algo::Node<Tree>;
    using Index = Tree_algo::Index<Tree>;

    Node_pointer parent_ptr = static_cast<Tree&&>(tree).root();
    Index base_idx(make_index_<Tree, 0u>());
    Side side = Side::left;
    bool const has_root = parent_ptr ? true : false;
    bool is_rightmost = true;
    bool is_leftmost = true;
    TREEXX_ASSERT(node_ptr);

    if(has_root)
    {
      Node* parent;
      Node_pointer child_ptr;
      for(;;)
      {
        parent = static_cast<Tree&&>(tree).address(parent_ptr);
        TREEXX_ASSERT(parent);
        Index const parent_idx(
          base_idx + static_cast<Tree&&>(tree).index(*parent));
        if(parent_idx < idx)
        {
          child_ptr =
            static_cast<Tree&&>(tree).template child<Side::right>(*parent);
          base_idx = parent_idx;
          is_leftmost = false;
          side = Side::right;
        }
        else
        {
          child_ptr =
            static_cast<Tree&&>(tree).template child<Side::left>(*parent);
          static_cast<Tree&&>(tree).increment_index(*parent);
          is_rightmost = false;
          side = Side::left;
        }

        if(child_ptr)
        {
          parent_ptr = child_ptr;
        }
        else
        {
          break;
        }
      }
    }

    Node* const node = static_cast<Tree&&>(tree).address(node_ptr);
    TREEXX_ASSERT(node);
    static_cast<Tree&&>(tree).set_parent(*node, parent_ptr);
    static_cast<Tree&&>(tree).template set_child<Side::left>(*node, nullptr);
    static_cast<Tree&&>(tree).template set_child<Side::right>(*node, nullptr);
    static_cast<Tree&&>(tree).set_balance(*node, Balance::poised);
    static_cast<Tree&&>(tree).set_side(*node, side);

    if(is_leftmost)
    {
      static_cast<Tree&&>(tree).template set_extreme<Side::left>(node_ptr);
      static_cast<Tree&&>(tree).template set_index<0u>(*node);
    }
    else
    {
      static_cast<Tree&&>(tree).template set_index<1u>(*node);
    }

    if(is_rightmost)
    {
      static_cast<Tree&&>(tree).template set_extreme<Side::right>(node_ptr);
    }

    if(has_root)
    {
      attach_and_fix_up_(static_cast<Tree&&>(tree), parent_ptr, node_ptr, side);
    }
    else
    {
      static_cast<Tree&&>(tree).set_root(node_ptr);
    }
  }

  template<class Tree>
  static void insert_at_offset(
    Tree&& tree,
    Node_pointer<Tree> const& node_ptr,
    typename Offset_trait_<Tree>::Type const& offset) noexcept
  {
    insert_at_offset_(static_cast<Tree&&>(tree), node_ptr, offset);
  }

  template<class Tree>
  static void insert_at_offset(
    Tree&& tree,
    Node_pointer<Tree> const& node_ptr,
    typename Offset_trait_<Tree>::Type const& offset,
    typename Offset_trait_<Tree>::Type const& shift) noexcept
  {
    insert_at_offset_(static_cast<Tree&&>(tree), node_ptr, offset, shift);
  }

  template<class Tree>
  static Node_pointer<Tree> pop_back(Tree&& tree) noexcept
  {
    return pop_<Side::right>(static_cast<Tree&&>(tree));
  }

  template<class Tree>
  static Node_pointer<Tree> pop_front(Tree&& tree) noexcept
  {
    return pop_<Side::left>(static_cast<Tree&&>(tree));
  }

  template<class Tree>
  static void erase(Tree&& tree, Node_pointer<Tree> const& node_ptr) noexcept
  {
    erase_<false>(static_cast<Tree&&>(tree), node_ptr);
  }

  template<Side side, class Tree>
  static void shift_suffix(
    Tree&& tree,
    Node<Tree>& node,
    typename Offset_trait_<Tree>::Type const& shift) noexcept
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    static_assert(Side::left == side || Side::right == side);

    for(Node* node_addr = ::std::addressof(node);;)
    {
      if constexpr(Side::left == side)
      {
        TREEXX_ASSERT(shift < static_cast<Tree&&>(tree).offset(*node_addr));
        static_cast<Tree&&>(tree).subtract_from_offset(*node_addr, shift);
      }
      else
      {
        static_cast<Tree&&>(tree).add_to_offset(*node_addr, shift);
      }

      for(;;)
      {
        Node_pointer const parent_ptr(
          static_cast<Tree&&>(tree).parent(*node_addr));
        if(parent_ptr)
        {
          Node* const parent_addr(
            static_cast<Tree&&>(tree).address(parent_ptr));
          Side const node_side(static_cast<Tree&&>(tree).side(*node_addr));
          TREEXX_ASSERT(parent_addr);
          node_addr = parent_addr;
          if(Side::left == node_side)
          {
            break;
          }
        }
        else
        {
          return;
        }
      }
    }
  }

  template<class Tree>
  static void shift_suffix(
    Tree&& tree,
    Node<Tree>& node,
    typename Offset_trait_<Tree>::Type const& shift,
    Side const side) noexcept
  {
    if(Side::left == side)
    {
      shift_suffix<Side::left>(static_cast<Tree&&>(tree), node, shift);
    }
    else
    {
      TREEXX_ASSERT(Side::right == side);
      shift_suffix<Side::right>(static_cast<Tree&&>(tree), node, shift);
    }
  }

private:
  template<class Tree, bool>
  struct Optional_index_
  {};

  template<class Tree>
  struct Optional_index_<Tree, true>
  {
    using Index = typename Index_trait_<Tree>::Type;

    Optional_index_() noexcept :
      base_index(make_index_<Tree, 0u>())
    {}

    Index base_index;
    Index node_index;
  };

  template<class Tree, bool>
  struct Optional_offset_
  {};

  template<class Tree>
  struct Optional_offset_<Tree, true>
  {
    using Offset = typename Offset_trait_<Tree>::Type;

    Optional_offset_() noexcept :
      base_offset(make_offset_<Tree , 0u>())
    {}

    Offset base_offset;
    Offset node_offset;
  };

  template<class Tree, bool, bool>
  struct Erase_base_
  {};

  template<class Tree>
  struct Erase_base_<Tree, true, false>
  {
    Erase_base_() noexcept :
      node_to_shift_count(0u)
    {}

    Node<Tree>* node_to_shift;
    typename Offset_trait_<Tree>::Type offset_or_shift;
    short unsigned node_to_shift_count;
  };

  template<class Tree>
  struct Erase_base_<Tree, true, true> : Erase_base_<Tree, true, false>
  {
    Erase_base_() noexcept :
      is_shift(false)
    {}

    bool is_shift;
  };

  template<
    class Tree,
    bool is_offset,
    bool = Index_trait_<Tree>::value,
    bool = Offset_trait_<Tree>::value>
  struct Index_or_offset_type_
  {};

  template<class Tree, bool has_offset>
  struct Index_or_offset_type_<Tree, false, true, has_offset>
  {
    using Type = Index<Tree>;
  };

  template<class Tree, bool has_index>
  struct Index_or_offset_type_<Tree, true, has_index, true>
  {
    using Type = Offset<Tree>;
  };

  template<class Tree, bool is_offset>
  using Index_or_offset_ =
    typename Index_or_offset_type_<Tree, is_offset>::Type;

  enum class Binary_search_type_ : char unsigned
  {
    any_match = 0u,
    lower_bound,
    upper_bound
  };

  template<
    Binary_search_type_ type,
    bool with_node,
    bool with_index,
    bool with_offset,
    bool unique,
    class Tree, class Compare>
  [[nodiscard]] static auto binary_search_(
    Tree&& tree,
    Compare&& compare) -> Node_pointer<Tree>
  {
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    constexpr bool has_index = Index_trait_<Tree>::value;
    constexpr bool has_offset = Offset_trait_<Tree>::value;
    static_assert(
      Binary_search_type_::any_match == type ||
      Binary_search_type_::lower_bound == type ||
      Binary_search_type_::upper_bound == type);
    static_assert(Binary_search_type_::lower_bound == type || !unique);
    static_assert(with_node || with_index || with_offset);
    static_assert(!with_index || has_index);
    static_assert(!with_offset || has_offset);

    struct Data :
      Optional_index_<Tree, with_index>,
      Optional_offset_<Tree, with_offset>
    {
      Data() :
        ptr(nullptr)
      {}

      Node_pointer ptr;
    };

    Node_pointer node_ptr(static_cast<Tree&&>(tree).root());
    Node_pointer const& node_ptr_const = node_ptr;
    Data data;

    if(node_ptr)
    {
      for(;;)
      {
        auto* const node(static_cast<Tree&&>(tree).address(node_ptr_const));
        TREEXX_ASSERT(node);
        Compare_result_ cmp;
        if constexpr(with_index)
        {
          data.node_index = data.base_index;
          data.node_index += static_cast<Tree&&>(tree).index(*node);
        }
        if constexpr(with_offset)
        {
          data.node_offset = data.base_offset;
          data.node_offset += static_cast<Tree&&>(tree).offset(*node);
        }

        if constexpr(with_node)
        {
          if constexpr(with_index)
          {
            auto const& index = data.node_index;
            if constexpr(with_offset)
            {
              auto const& offset = data.node_offset;
              cmp = static_cast<Compare&&>(compare)(*node, index, offset);
            }
            else
            {
              cmp = static_cast<Compare&&>(compare)(*node, index);
            }
          }
          else
          {
            if constexpr(with_offset)
            {
              auto const& offset = data.node_offset;
              cmp = static_cast<Compare&&>(compare)(*node, offset);
            }
            else
            {
              cmp = static_cast<Compare&&>(compare)(*node);
            }
          }
        }
        else
        {
          if constexpr(with_index)
          {
            auto const& index = data.node_index;
            if constexpr(with_offset)
            {
              auto const& offset = data.node_offset;
              cmp = static_cast<Compare&&>(compare)(index, offset);
            }
            else
            {
              cmp = static_cast<Compare&&>(compare)(index);
            }
          }
          else
          {
            if constexpr(with_offset)
            {
              auto const& offset = data.node_offset;
              cmp = static_cast<Compare&&>(compare)(offset);
            }
          }
        }

        switch(cmp)
        {
          case Compare_result_::greater:
            if constexpr(
              Binary_search_type_::lower_bound == type ||
              Binary_search_type_::upper_bound == type)
            {
              data.ptr = node_ptr;
            }
            node_ptr =
              static_cast<Tree&&>(tree).template child<Side::left>(*node);
            if(node_ptr)
            {
              continue;
            }
            break;

          case Compare_result_::equal:
            if constexpr(Binary_search_type_::lower_bound == type)
            {
              if constexpr(unique)
              {
                return node_ptr;
              }
              else
              {
                data.ptr = node_ptr;
                node_ptr =
                  static_cast<Tree&&>(tree).template child<Side::left>(*node);
                if(node_ptr)
                {
                  continue;
                }
                break;
              }
            }
            else if constexpr(Binary_search_type_::any_match == type)
            {
              return node_ptr;
            }
            else
            {
              [[fallthrough]];
            }

          case Compare_result_::less:
            node_ptr =
              static_cast<Tree&&>(tree).template child<Side::right>(*node);
            if(node_ptr)
            {
              if constexpr(with_index)
              {
                data.base_index = data.node_index;
              }
              if constexpr(with_offset)
              {
                data.base_offset = data.node_offset;
              }
              continue;
            }
            break;

          default:
            TREEXX_ASSERT(false);
            break;
        }

        break;
      }
    }

    return data.ptr;
  }

  template<Side side, class Tree, class... Offset>
  static void push_(
    Tree&& tree,
    Node_pointer<Tree> const& node_ptr,
    Offset const&... offset) noexcept
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    Side constexpr opp_side = Side::left == side ? Side::right : Side::left;
    bool constexpr has_offset = Offset_trait_<Tree>::value;
    bool constexpr has_index = Index_trait_<Tree>::value;
    static_assert(Side::left == side || Side::right == side);
    static_assert(has_offset || 1u > sizeof...(Offset));

    Node* const node = static_cast<Tree&&>(tree).address(node_ptr);
    TREEXX_ASSERT(node_ptr);
    TREEXX_ASSERT(node);

    Node_pointer const parent_ptr =
      static_cast<Tree&&>(tree).template extreme<side>();
    Side node_side;
    bool const has_parent = parent_ptr ? true : false;
    if constexpr(Side::left == side)
    {
      node_side = Side::left;
    }
    else if constexpr(Side::right == side)
    {
      node_side = has_parent ? side : Side::left;
    }

    static_cast<Tree&&>(tree).set_parent(*node, parent_ptr);
    static_cast<Tree&&>(tree).set_balance(*node, Balance::poised);
    static_cast<Tree&&>(tree).set_side(*node, node_side);
    static_cast<Tree&&>(tree).template set_child<Side::left>(*node, nullptr);
    static_cast<Tree&&>(tree).template set_child<Side::right>(*node, nullptr);
    static_cast<Tree&&>(tree).template set_extreme<side>(node_ptr);
    if constexpr(has_offset && Side::right == side)
    {
      static_cast<Tree&&>(tree).set_offset(*node, offset...);
    }

    if(has_parent)
    {
      if constexpr(has_offset && Side::left == side)
      {
        Node* const parent = static_cast<Tree&&>(tree).address(parent_ptr);
        TREEXX_ASSERT(parent);
        static_cast<Tree&&>(tree).set_offset(
          *node, static_cast<Tree&&>(tree).offset(*parent));
        for(Node* addr = parent;;)
        {
          if constexpr(has_index)
          {
            static_cast<Tree&&>(tree).increment_index(*addr);
          }

          static_cast<Tree&&>(tree).add_to_offset(*addr, offset...);
          Node_pointer const ptr = static_cast<Tree&&>(tree).parent(*addr);
          if(ptr)
          {
            addr = static_cast<Tree&&>(tree).address(ptr);
            TREEXX_ASSERT(addr);
          }
          else
          {
            break;
          }
        }
      }

      if constexpr(has_index)
      {
        if constexpr(Side::left == side)
        {
          static_cast<Tree&&>(tree).template set_index<0u>(*node);
          if constexpr(!has_offset)
          {
            Node_pointer ptr(parent_ptr);
            do
            {
              Node* const addr = static_cast<Tree&&>(tree).address(ptr);
              TREEXX_ASSERT(addr);
              static_cast<Tree&&>(tree).increment_index(*addr);
              ptr = static_cast<Tree&&>(tree).parent(*addr);
            }
            while(ptr);
          }
        }
        else if constexpr(Side::right == side)
        {
          static_cast<Tree&&>(tree).template set_index<1u>(*node);
        }
      }

      TREEXX_ASSERT(
        !static_cast<Tree&&>(tree).template child<side>(
          *static_cast<Tree&&>(tree).address(parent_ptr)));
      attach_and_fix_up_(static_cast<Tree&&>(tree), parent_ptr, node_ptr, side);
    }
    else
    {
      if constexpr(has_index)
      {
        static_cast<Tree&&>(tree).template set_index<0u>(*node);
      }
      if constexpr(has_offset && Side::left == side)
      {
        static_cast<Tree&&>(tree).set_offset(*node, offset...);
      }

      static_cast<Tree&&>(tree).set_root(node_ptr);
      static_cast<Tree&&>(tree).template set_extreme<opp_side>(node_ptr);
    }
  }

  template<Side side, class Tree>
  static Node_pointer<Tree> pop_(Tree&& tree) noexcept
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    static_assert(Side::left == side || Side::right == side);
    Side constexpr opp_side = Side::left == side ? Side::right : Side::left;
    bool constexpr has_offset = Offset_trait_<Tree>::value;
    bool constexpr has_index = Index_trait_<Tree>::value;

    Node_pointer const node_ptr =
      static_cast<Tree&&>(tree).template extreme<side>();
    Node* const node = static_cast<Tree&&>(tree).address(node_ptr);
    TREEXX_ASSERT(node_ptr);
    TREEXX_ASSERT(node);
    Node_pointer const parent_ptr = static_cast<Tree&&>(tree).parent(*node);
    Node_pointer const child_ptr =
      static_cast<Tree&&>(tree).template child<opp_side>(*node);
    bool const has_parent = parent_ptr ? true : false;

    if(child_ptr)
    {
      Node* const child = static_cast<Tree&&>(tree).address(child_ptr);
      TREEXX_ASSERT(child);
      static_cast<Tree&&>(tree).template set_extreme<side>(child_ptr);
      static_cast<Tree&&>(tree).set_parent(*child, parent_ptr);
      if constexpr(Side::left == side)
      {
        if constexpr(has_index)
        {
          static_cast<Tree&&>(tree).template set_index<0u>(*child);
        }
        if constexpr(has_offset)
        {
          static_cast<Tree&&>(tree).add_to_offset(
            *child, static_cast<Tree&&>(tree).offset(*node));
        }
      }

      if constexpr(Side::left == side)
      {
        static_cast<Tree&&>(tree).set_side(*child, Side::left);
      }
      else
      {
        if(has_parent)
        {
          static_cast<Tree&&>(tree).set_side(*child, Side::right);
        }
      }
    }
    else
    {
      static_cast<Tree&&>(tree).template set_extreme<side>(parent_ptr);
    }

    if(has_parent)
    {
      Node* const parent = static_cast<Tree&&>(tree).address(parent_ptr);
      TREEXX_ASSERT(parent);
      static_cast<Tree&&>(tree).template set_child<side>(*parent, child_ptr);
      if constexpr(Side::left == side && has_index)
      {
        for(Node* addr = parent;;)
        {
          static_cast<Tree&&>(tree).decrement_index(*addr);
          Node_pointer const ptr = static_cast<Tree&&>(tree).parent(*addr);
          if(ptr)
          {
            addr = static_cast<Tree&&>(tree).address(ptr);
            TREEXX_ASSERT(addr);
          }
          else
          {
            break;
          }
        }
      }

      fix_up_detachment_(static_cast<Tree&&>(tree), parent_ptr, side);
    }
    else
    {
      static_cast<Tree&&>(tree).set_root(child_ptr);
      static_cast<Tree&&>(tree).template set_extreme<opp_side>(child_ptr);
    }

    return node_ptr;
  }

  template<class Tree, class... Shift>
  static void insert_at_offset_(
    Tree&& tree,
    Node_pointer<Tree> const& node_ptr,
    typename Offset_trait_<Tree>::Type const& offset,
    Shift const&... shift) noexcept
  {
    using Node_pointer = Tree_algo::Node_pointer<Tree>;
    using Node = Tree_algo::Node<Tree>;
    using Offset = Tree_algo::Offset<Tree>;

    bool constexpr has_index = Index_trait_<Tree>::value;
    TREEXX_ASSERT(node_ptr);

    Node_pointer parent_ptr(static_cast<Tree&&>(tree).root());
    Offset base_offset(make_offset_<Tree, 0u>());
    Side side = Side::left;
    bool const has_root = parent_ptr ? true : false;
    bool is_rightmost = true;
    bool is_leftmost = true;

    if(has_root)
    {
      Node* parent;
      Node_pointer child_ptr;
      for(;;)
      {
        parent = static_cast<Tree&&>(tree).address(parent_ptr);
        TREEXX_ASSERT(parent);
        Offset const parent_offset(
          base_offset + static_cast<Tree&&>(tree).offset(*parent));
        if(parent_offset < offset)
        {
          child_ptr =
            static_cast<Tree&&>(tree).template child<Side::right>(*parent);
          base_offset = parent_offset;
          is_leftmost = false;
          side = Side::right;
        }
        else
        {
          child_ptr =
            static_cast<Tree&&>(tree).template child<Side::left>(*parent);
          if constexpr(has_index)
          {
            static_cast<Tree&&>(tree).increment_index(*parent);
          }
          if constexpr(0u < sizeof...(Shift))
          {
            static_cast<Tree&&>(tree).add_to_offset(*parent, shift...);
          }
          else
          {
            TREEXX_ASSERT(offset < parent_offset);
          }

          is_rightmost = false;
          side = Side::left;
        }

        if(child_ptr)
        {
          parent_ptr = child_ptr;
        }
        else
        {
          break;
        }
      }
    }

    Node* const node = static_cast<Tree&&>(tree).address(node_ptr);
    Offset const node_offset(offset - base_offset);
    TREEXX_ASSERT(node);
    static_cast<Tree&&>(tree).set_parent(*node, parent_ptr);
    static_cast<Tree&&>(tree).template set_child<Side::left>(*node, nullptr);
    static_cast<Tree&&>(tree).template set_child<Side::right>(*node, nullptr);
    static_cast<Tree&&>(tree).set_offset(*node, node_offset);
    static_cast<Tree&&>(tree).set_balance(*node, Balance::poised);
    static_cast<Tree&&>(tree).set_side(*node, side);

    if(is_leftmost)
    {
      static_cast<Tree&&>(tree).template set_extreme<Side::left>(node_ptr);
      if constexpr(has_index)
      {
        static_cast<Tree&&>(tree).template set_index<0u>(*node);
      }
    }
    else
    {
      if constexpr(has_index)
      {
        static_cast<Tree&&>(tree).template set_index<1u>(*node);
      }
    }

    if(is_rightmost)
    {
      static_cast<Tree&&>(tree).template set_extreme<Side::right>(node_ptr);
    }

    if(has_root)
    {
      attach_and_fix_up_(static_cast<Tree&&>(tree), parent_ptr, node_ptr, side);
    }
    else
    {
      static_cast<Tree&&>(tree).set_root(node_ptr);
    }
  }

  template<bool with_shift, class Tree>
  static void erase_(
    Tree&& tree,
    Node_pointer<Tree> const& node_ptr) noexcept
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;
    using Index_trait = Index_trait_<Tree>;
    using Offset_trait = Offset_trait_<Tree>;

    bool constexpr has_index = Index_trait::value;
    bool constexpr has_offset = Offset_trait::value;
    static_assert(!with_shift || has_offset);

    struct Data : Erase_base_<Tree, has_offset, with_shift>
    {
      explicit Data(Side const s) noexcept :
        side(s),
        fixup_side(s),
        attach(true)
      {}

      Side side;
      Side fixup_side;
      bool attach;
    };

    Node* const node = static_cast<Tree&&>(tree).address(node_ptr);
    TREEXX_ASSERT(node_ptr);
    TREEXX_ASSERT(node);

    if(static_cast<Tree&&>(tree).template extreme<Side::left>() == node_ptr)
    {
      static_cast<Tree&&>(tree).template set_extreme<Side::left>(
        adjacent_node<Side::right>(static_cast<Tree&&>(tree), *node));
    }
    if(static_cast<Tree&&>(tree).template extreme<Side::right>() == node_ptr)
    {
      static_cast<Tree&&>(tree).template set_extreme<Side::right>(
        adjacent_node<Side::left>(static_cast<Tree&&>(tree), *node));
    }

    Node* parent = nullptr;
    Node* attach_node = nullptr;
    Node_pointer attach_node_ptr(nullptr);
    Node_pointer parent_ptr = static_cast<Tree&&>(tree).parent(*node);
    Node_pointer fixup_node_ptr(parent_ptr);
    Node_pointer const left_child_ptr =
      static_cast<Tree&&>(tree).template child<Side::left>(*node);
    Node_pointer const right_child_ptr =
      static_cast<Tree&&>(tree).template child<Side::right>(*node);
    Data data(static_cast<Tree&&>(tree).side(*node));

    if constexpr(with_shift)
    {
      data.offset_or_shift = static_cast<Tree&&>(tree).offset(*node);
    }

    if(left_child_ptr)
    {
      Node* const left_child =
      static_cast<Tree&&>(tree).address(left_child_ptr);
      TREEXX_ASSERT(left_child);

      if(right_child_ptr)
      {
        Node* const right_child =
          static_cast<Tree&&>(tree).address(right_child_ptr);
        TREEXX_ASSERT(right_child);
        Node_pointer left_grandchild_ptr =
          static_cast<Tree&&>(tree).template child<Side::left>(*right_child);
        Node* transplant_node;
        Node_pointer transplant_node_ptr;
        Balance const balance(static_cast<Tree&&>(tree).balance(*node));
        bool has_left_grandchild = left_grandchild_ptr ? true : false;

        if(has_left_grandchild)
        {
          Node* prev_node = right_child;
          Node_pointer prev_node_ptr = right_child_ptr;
          if constexpr(has_offset)
          {
            data.node_to_shift = prev_node;
          }

          for(;;)
          {
            if constexpr(has_index)
            {
              static_cast<Tree&&>(tree).decrement_index(*prev_node);
            }
            if constexpr(has_offset)
            {
              ++data.node_to_shift_count;
            }

            Node* const left_grandchild =
              static_cast<Tree&&>(tree).address(left_grandchild_ptr);
            TREEXX_ASSERT(left_grandchild);
            Node_pointer const next_left_grandchild_ptr =
              static_cast<Tree&&>(
                tree).template child<Side::left>(*left_grandchild);
            if(next_left_grandchild_ptr)
            {
              prev_node = left_grandchild;
              prev_node_ptr = left_grandchild_ptr;
              left_grandchild_ptr = next_left_grandchild_ptr;
              continue;
            }

            Node_pointer const right_grandchild_ptr =
              static_cast<Tree&&>(
                tree).template child<Side::right>(*left_grandchild);
            static_cast<Tree&&>(tree).
              template set_child<Side::left>(*prev_node, right_grandchild_ptr);
            if(right_grandchild_ptr)
            {
              Node* const right_grandchild =
                static_cast<Tree&&>(tree).address(right_grandchild_ptr);
              TREEXX_ASSERT(right_grandchild);
              static_cast<Tree&&>(
                tree).set_parent(*right_grandchild, prev_node_ptr);
              static_cast<Tree&&>(
                tree).set_side(*right_grandchild, Side::left);
            }

            transplant_node = left_grandchild;
            transplant_node_ptr = left_grandchild_ptr;
            fixup_node_ptr = prev_node_ptr;
            data.fixup_side = Side::left;
            static_cast<Tree&&>(
              tree).template set_child<Side::right>(
                *transplant_node, right_child_ptr);
            static_cast<Tree&&>(
              tree).set_parent(*right_child, transplant_node_ptr);
            break;
          }
        }
        else
        {
          transplant_node = right_child;
          transplant_node_ptr = right_child_ptr;
          fixup_node_ptr = right_child_ptr;
          data.fixup_side = Side::right;
        }

        if constexpr(has_index)
        {
          typename Index_trait::Type index(
            static_cast<Tree&&>(tree).index(*node));
          static_cast<Tree&&>(tree).set_index(
            *transplant_node, static_cast<decltype(index)&&>(index));
        }

        if constexpr(has_offset)
        {
          if constexpr(with_shift)
          {
            auto const offset = data.offset_or_shift;
            data.offset_or_shift =
              static_cast<Tree&&>(tree).offset(*transplant_node);
            data.is_shift = true;
            static_cast<Tree&&>(tree).set_offset(*transplant_node, offset);
          }
          else
          {
            data.offset_or_shift = static_cast<Tree&&>(tree).offset(*node);
            auto const offset = data.offset_or_shift;
            data.offset_or_shift =
              static_cast<Tree&&>(tree).offset(*transplant_node);
            static_cast<Tree&&>(tree).add_to_offset(*transplant_node, offset);
          }

          if(0u < data.node_to_shift_count)
          {
            for(;;)
            {
              Node*& node_to_shift = data.node_to_shift;
              TREEXX_ASSERT(node_to_shift);
              static_cast<Tree&&>(tree).
                subtract_from_offset(*node_to_shift, data.offset_or_shift);
              if(0u < --data.node_to_shift_count)
              {
                Node_pointer const next_node_to_shift_ptr =
                  static_cast<Tree&&>(tree).
                    template child<Side::left>(*node_to_shift);
                TREEXX_ASSERT(next_node_to_shift_ptr);
                node_to_shift =
                  static_cast<Tree&&>(tree).address(next_node_to_shift_ptr);
                TREEXX_ASSERT(node_to_shift);
              }
              else
              {
                break;
              }
            }
          }
        }

        static_cast<Tree&&>(tree).
          template set_child<Side::left>(*transplant_node, left_child_ptr);
        static_cast<Tree&&>(tree).set_balance(*transplant_node, balance);
        static_cast<Tree&&>(tree).set_side(*transplant_node, data.side);
        static_cast<Tree&&>(tree).set_parent(*left_child, transplant_node_ptr);

        if(parent_ptr)
        {
          parent = static_cast<Tree&&>(tree).address(parent_ptr);
          TREEXX_ASSERT(parent);
          static_cast<Tree&&>(tree).set_parent(*transplant_node, parent_ptr);
          set_child_(
            static_cast<Tree&&>(tree),
            *parent,
            transplant_node_ptr,
            data.side);
        }
        else
        {
          static_cast<Tree&&>(tree).set_parent(*transplant_node, nullptr);
          static_cast<Tree&&>(tree).set_root(transplant_node_ptr);
        }

        data.attach = false;
      }
      else
      {
        attach_node = left_child;
        attach_node_ptr = left_child_ptr;
      }
    }
    else if(right_child_ptr)
    {
      Node* const right_child =
        static_cast<Tree&&>(tree).address(right_child_ptr);
      TREEXX_ASSERT(right_child);
      attach_node = right_child;
      attach_node_ptr = right_child_ptr;
      if constexpr(has_index)
      {
        typename Index_trait::Type index(
          static_cast<Tree&&>(tree).index(*node));
        static_cast<Tree&&>(tree).set_index(
          *right_child, static_cast<decltype(index)&&>(index));
      }
      if constexpr(has_offset)
      {
        if constexpr(with_shift)
        {
          auto const offset = data.offset_or_shift;
          data.offset_or_shift =
            static_cast<Tree&&>(tree).offset(*right_child);
          data.is_shift = true;
          static_cast<Tree&&>(tree).set_offset(*right_child, offset);
        }
        else
        {
          auto& shift = data.offset_or_shift;
          shift = static_cast<Tree&&>(tree).offset(*node);
          static_cast<Tree&&>(tree).add_to_offset(*right_child, shift);
        }
      }
    }

    if(data.attach)
    {
      if(parent_ptr)
      {
        parent = static_cast<Tree&&>(tree).address(parent_ptr);
        TREEXX_ASSERT(parent);

        if(attach_node)
        {
          static_cast<Tree&&>(tree).set_parent(*attach_node, parent_ptr);
          static_cast<Tree&&>(tree).set_side(*attach_node, data.side);
        }

        set_child_(
          static_cast<Tree&&>(tree),
          *parent, attach_node_ptr, data.side);
      }
      else
      {
        if(attach_node)
        {
          static_cast<Tree&&>(tree).set_parent(*attach_node, nullptr);
        }

        static_cast<Tree&&>(tree).set_root(attach_node_ptr);
        return;
      }
    }

    if constexpr(has_index || with_shift)
    {
      for(;;)
      {
        if(!parent)
        {
          if(parent_ptr)
          {
            parent = static_cast<Tree&&>(tree).address(parent_ptr);
            TREEXX_ASSERT(parent);
          }
          else
          {
            break;
          }
        }

        for(;;)
        {
          if(Side::left == data.side)
          {
            if constexpr(has_index)
            {
              static_cast<Tree&&>(tree).decrement_index(*parent);
            }
            if constexpr(with_shift)
            {
              auto& offset = data.offset_or_shift;
              if(!data.is_shift)
              {
                offset = static_cast<typename Offset_trait::Type>(
                  static_cast<typename Offset_trait::Type>(
                    static_cast<Tree&&>(tree).offset(*parent)) -
                  offset);
                data.is_shift = true;
              }

              static_cast<Tree&&>(tree).subtract_from_offset(*parent, offset);
            }
          }
          else
          {
            TREEXX_ASSERT(Side::right == data.side);
            if constexpr(with_shift)
            {
              if(!data.is_shift)
              {
                data.offset_or_shift +=
                  static_cast<typename Offset_trait::Type>(
                    static_cast<Tree&&>(tree).offset(*parent));
              }
            }
          }

          parent_ptr = static_cast<Tree&&>(tree).parent(*parent);
          if(parent_ptr)
          {
            data.side = static_cast<Tree&&>(tree).side(*parent);
            parent = static_cast<Tree&&>(tree).address(parent_ptr);
          }
          else
          {
            break;
          }
        }

        break;
      }
    }

    fix_up_detachment_(
      static_cast<Tree&&>(tree),
      fixup_node_ptr, data.fixup_side);
  }

  template<class Tree>
  static void attach_and_fix_up_(
    Tree&& tree,
    Node_pointer<Tree> const& node_ptr,
    Node_pointer<Tree> const& child_ptr,
    Side const side) noexcept
  {
    using Node = Tree_algo::Node<Tree>;

    TREEXX_ASSERT(node_ptr);
    TREEXX_ASSERT(child_ptr);
    TREEXX_ASSERT(Side::left == side || Side::right == side);
    Node* const node = static_cast<Tree&&>(tree).address(node_ptr);
    TREEXX_ASSERT(node);
    set_child_(static_cast<Tree&&>(tree), *node, child_ptr, side);
    Balance const balance(static_cast<Tree&&>(tree).balance(*node));
    switch(balance)
    {
    case Balance::poised:
      switch(side)
      {
      case Side::left:
        static_cast<Tree&&>(tree).set_balance(*node, Balance::overleft);
        break;
      case Side::right:
        static_cast<Tree&&>(tree).set_balance(*node, Balance::overright);
        break;
      }
      break;

    case Balance::overleft:
    case Balance::overright:
      TREEXX_ASSERT(
        (Balance::overleft == balance && Side::right == side) ||
        (Balance::overright == balance && Side::left == side));
      static_cast<Tree&&>(tree).set_balance(*node, Balance::poised);
      return; // No fixup is needed.

    default:
      TREEXX_ASSERT(false);
      return;
    }

    fix_up_attachment_(static_cast<Tree&&>(tree), node_ptr);
  }

  template<class Tree>
  static void fix_up_attachment_(
    Tree&& tree,
    Node_pointer<Tree> node_ptr) noexcept
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    Node_pointer const& node_ptr_const = node_ptr;
    Node* node = static_cast<Tree&&>(tree).address(node_ptr_const);
    TREEXX_ASSERT(node_ptr_const);
    TREEXX_ASSERT(node);

    for(;;)
    {
      Node_pointer const parent_ptr = static_cast<Tree&&>(tree).parent(*node);
      if(!parent_ptr)
      {
        break;
      }

      Node_pointer const& parent_ptr_const = parent_ptr;
      Node* const parent = static_cast<Tree&&>(tree).address(parent_ptr_const);
      TREEXX_ASSERT(parent);
      Balance const parent_balance(static_cast<Tree&&>(tree).balance(*parent));
      Side const side(static_cast<Tree&&>(tree).side(*node));
      bool fix_up_left = false, fix_up_right = false;

      switch(side)
      {
      case Side::left:
        switch(parent_balance)
        {
        case Balance::poised:
          static_cast<Tree&&>(tree).set_balance(*parent, Balance::overleft);
          break;
        case Balance::overleft:
          fix_up_left = true;
          break;
        case Balance::overright:
          static_cast<Tree&&>(tree).set_balance(*parent, Balance::poised);
          return; // Well balanced from now on.
        default:
          TREEXX_ASSERT(false);
          break;
        }
        break;

      case Side::right:
        switch(parent_balance)
        {
        case Balance::poised:
          static_cast<Tree&&>(tree).set_balance(*parent, Balance::overright);
          break;
        case Balance::overleft:
          static_cast<Tree&&>(tree).set_balance(*parent, Balance::poised);
          return; // Well balanced from now on.
        case Balance::overright:
          fix_up_right = true;
          break;
        default:
          TREEXX_ASSERT(false);
          break;
        }
        break;

      default:
        TREEXX_ASSERT(false);
        break;
      }

      if(fix_up_left)
      {
        Balance const balance(static_cast<Tree&&>(tree).balance(*node));
        switch(balance)
        {
        case Balance::overleft:
          static_cast<Tree&&>(tree).set_balance(*parent, Balance::poised);
          static_cast<Tree&&>(tree).set_balance(*node, Balance::poised);
          rotate_<Side::right>(static_cast<Tree&&>(tree), parent_ptr);
          break;

        case Balance::overright:
          {
            Node_pointer const child_ptr =
              static_cast<Tree&&>(tree).template child<Side::right>(*node);
            if(child_ptr)
            {
              Node* const child = static_cast<Tree&&>(tree).address(child_ptr);
              TREEXX_ASSERT(child);
              Balance const child_balance(
                static_cast<Tree&&>(tree).balance(*child));
              Balance new_parent_balance = Balance::poised;
              Balance new_node_balance = Balance::poised;
              switch(child_balance)
              {
              case Balance::poised:
                break;
              case Balance::overleft:
                new_parent_balance = Balance::overright;
                break;
              case Balance::overright:
                new_node_balance = Balance::overleft;
                break;
              default:
                TREEXX_ASSERT(false);
                break;
              }

              static_cast<Tree&&>(tree).set_balance(
                *parent, new_parent_balance);
              static_cast<Tree&&>(tree).set_balance(*node, new_node_balance);
              static_cast<Tree&&>(tree).set_balance(*child, Balance::poised);
              rotate_<Side::left>(static_cast<Tree&&>(tree), node_ptr);
              rotate_<Side::right>(static_cast<Tree&&>(tree), parent_ptr);
            }
            else
            {
              TREEXX_ASSERT(false);
            }
          }
          break;

        default:
          TREEXX_ASSERT(false);
          break;
        }

        break;
      }

      if(fix_up_right)
      {
        Balance const balance(static_cast<Tree&&>(tree).balance(*node));
        switch(balance)
        {
        case Balance::overright:
          static_cast<Tree&&>(tree).set_balance(*parent, Balance::poised);
          static_cast<Tree&&>(tree).set_balance(*node, Balance::poised);
          rotate_<Side::left>(static_cast<Tree&&>(tree), parent_ptr);
          break;

        case Balance::overleft:
          {
            Node_pointer const child_ptr = static_cast<Tree&&>(tree).
              template child<Side::left>(*node);
            if(child_ptr)
            {
              Node* const child = static_cast<Tree&&>(tree).address(child_ptr);
              TREEXX_ASSERT(child);
              Balance const child_balance(
                static_cast<Tree&&>(tree).balance(*child));
              Balance new_parent_balance = Balance::poised;
              Balance new_node_balance = Balance::poised;
              switch(child_balance)
              {
              case Balance::poised:
                break;
              case Balance::overleft:
                new_node_balance = Balance::overright;
                break;
              case Balance::overright:
                new_parent_balance = Balance::overleft;
                break;
              default:
                TREEXX_ASSERT(false);
                break;
              }

              static_cast<Tree&&>(tree).set_balance(*parent, new_parent_balance);
              static_cast<Tree&&>(tree).set_balance(*node, new_node_balance);
              static_cast<Tree&&>(tree).set_balance(*child, Balance::poised);
              rotate_<Side::right>(static_cast<Tree&&>(tree), node_ptr);
              rotate_<Side::left>(static_cast<Tree&&>(tree), parent_ptr);
            }
            else
            {
              TREEXX_ASSERT(false);
            }
          }
            break;

        default:
          TREEXX_ASSERT(false);
          break;
        }

        break;
      }

      node_ptr = parent_ptr;
      node = parent;
    }
  }

  template<class Tree>
  static void fix_up_detachment_(
    Tree&& tree,
    Node_pointer<Tree> node_ptr,
    Side side) noexcept
  {
    using Node = Tree_algo::Node<Tree>;

    for(;;)
    {
      TREEXX_ASSERT(Side::left == side || Side::right == side);
      Node* const node = Side::left == side
        ? fix_up_detachment_<Side::left>(static_cast<Tree&&>(tree), node_ptr)
        : fix_up_detachment_<Side::right>(static_cast<Tree&&>(tree), node_ptr);
      if(node)
      {
        node_ptr = static_cast<Tree&&>(tree).parent(*node);
        if(node_ptr)
        {
          side = static_cast<Tree&&>(tree).side(*node);
          continue;
        }
      }

      break;
    }
  }

  template<Side side, class Tree>
  [[nodiscard]] static auto fix_up_detachment_(
    Tree&& tree,
    Node_pointer<Tree> const& node_ptr) noexcept -> Node<Tree>*
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    static_assert(Side::left == side || Side::right == side);
    Node* const node = static_cast<Tree&&>(tree).address(node_ptr);
    TREEXX_ASSERT(node_ptr);
    TREEXX_ASSERT(node);

    Balance const balance(static_cast<Tree&&>(tree).balance(*node));
    Balance new_balance = Balance::poised;
    bool rotate = false;
    switch(balance)
    {
    case Balance::overleft:
      if constexpr(Side::right == side)
      {
        rotate = true;
      }
      break;

    case Balance::overright:
      if constexpr(Side::left == side)
      {
        rotate = true;
      }
      break;

    default:
      TREEXX_ASSERT(Balance::poised == balance);
      if constexpr(Side::left == side)
      {
        new_balance = Balance::overright;
      }
      else if constexpr(Side::right == side)
      {
        new_balance = Balance::overleft;
      }
      break;
    }

    if(!rotate)
    {
      static_cast<Tree&&>(tree).set_balance(*node, new_balance);
      return Balance::poised == new_balance ? node : nullptr;
    }

    Side constexpr opp_side = Side::left == side ? Side::right : Side::left;
    Node_pointer const child_ptr =
      static_cast<Tree&&>(tree).template child<opp_side>(*node);
    Node* const child = static_cast<Tree&&>(tree).address(child_ptr);
    Node* ret = nullptr;
    TREEXX_ASSERT(child_ptr);
    TREEXX_ASSERT(child);

    Balance const child_balance(static_cast<Tree&&>(tree).balance(*child));
    Balance child_new_balance = Balance::poised;
    bool rotate_twice = false;
    switch(child_balance)
    {
    case Balance::overleft:
      if constexpr(Side::left == side)
      {
        rotate_twice = true;
      }
      break;

    case Balance::overright:
      if constexpr(Side::right == side)
      {
        rotate_twice = true;
      }
      break;

    default:
      TREEXX_ASSERT(Balance::poised == child_balance);
      if constexpr(Side::left == side)
      {
        new_balance = Balance::overright;
        child_new_balance = Balance::overleft;
      }
      else if constexpr(Side::right == side)
      {
        new_balance = Balance::overleft;
        child_new_balance = Balance::overright;
      }
      break;
    }

    if(rotate_twice)
    {
      Node_pointer const grandchild_ptr =
        static_cast<Tree&&>(tree).template child<side>(*child);
      Node* const grandchild =
        static_cast<Tree&&>(tree).address(grandchild_ptr);
      TREEXX_ASSERT(grandchild_ptr);
      TREEXX_ASSERT(grandchild);

      Balance const grandchild_balance(
        static_cast<Tree&&>(tree).balance(*grandchild));
      switch(grandchild_balance)
      {
      case Balance::overleft:
        if constexpr(Side::left == side)
        {
          child_new_balance = Balance::overright;
        }
        else if constexpr(Side::right == side)
        {
          new_balance = Balance::overright;
        }
        break;

      case Balance::overright:
        if constexpr(Side::left == side)
        {
          new_balance = Balance::overleft;
        }
        else if constexpr(Side::right == side)
        {
          child_new_balance = Balance::overleft;
        }
        break;

      default:
        TREEXX_ASSERT(Balance::poised == grandchild_balance);
        break;
      }

      rotate_<opp_side>(static_cast<Tree&&>(tree), child_ptr);
      if(Balance::poised != grandchild_balance)
      {
        static_cast<Tree&&>(tree).set_balance(*grandchild, Balance::poised);
      }
      ret = grandchild;
    }
    else if(Balance::poised == child_new_balance)
    {
      ret = child;
    }

    rotate_<side>(static_cast<Tree&&>(tree), node_ptr);
    static_cast<Tree&&>(tree).set_balance(*node, new_balance);
    static_cast<Tree&&>(tree).set_balance(*child, child_new_balance);
    return ret;
  }

  template<Side side, class Tree>
  static void rotate_(Tree&& tree, Node_pointer<Tree> const& node_ptr) noexcept
  {
    using Node = Tree_algo::Node<Tree>;
    using Node_pointer = Tree_algo::Node_pointer<Tree>;

    static_assert(Side::left == side || Side::right == side);
    Side constexpr opp_side = Side::left == side ? Side::right : Side::left;
    Node* const node = static_cast<Tree&&>(tree).address(node_ptr);
    TREEXX_ASSERT(node_ptr);
    TREEXX_ASSERT(node);
    Node_pointer const parent_ptr(static_cast<Tree&&>(tree).parent(*node));
    Node_pointer const child_ptr(
      static_cast<Tree&&>(tree).template child<opp_side>(*node));
    Node* const child = static_cast<Tree&&>(tree).address(child_ptr);
    TREEXX_ASSERT(child);
    Node_pointer const grandchild_ptr(
      static_cast<Tree&&>(tree).template child<side>(*child));
    Side const subtree_side = static_cast<Tree&&>(tree).side(*node);
    static_cast<Tree&&>(tree).template set_child<side>(*child, node_ptr);
    static_cast<Tree&&>(tree).set_parent(*node, child_ptr);
    static_cast<Tree&&>(tree).
      template set_child<opp_side>(*node, grandchild_ptr);
    static_cast<Tree&&>(tree).set_side(*node, side);
    if(grandchild_ptr)
    {
      Node* const grandchild =
        static_cast<Tree&&>(tree).address(grandchild_ptr);
      TREEXX_ASSERT(grandchild);
      static_cast<Tree&&>(tree).set_parent(*grandchild, node_ptr);
      static_cast<Tree&&>(tree).set_side(*grandchild, opp_side);
    }

    static_cast<Tree&&>(tree).set_parent(*child, parent_ptr);
    static_cast<Tree&&>(tree).set_side(*child, subtree_side);
    if(parent_ptr)
    {
      Node* const parent = static_cast<Tree&&>(tree).address(parent_ptr);
      TREEXX_ASSERT(parent);
      switch(subtree_side)
      {
      case Side::left:
        static_cast<Tree&&>(tree).
          template set_child<Side::left>(*parent, child_ptr);
        break;
      case Side::right:
        static_cast<Tree&&>(tree).
          template set_child<Side::right>(*parent, child_ptr);
        break;
      default:
        TREEXX_ASSERT(false);
        break;
      }
    }
    else
    {
      static_cast<Tree&&>(tree).set_root(child_ptr);
    }

    if constexpr(Index_trait_<Tree>::value)
    {
      if constexpr(Side::left == side)
      {
        static_cast<Tree&&>(tree).add_to_index(
          *child, static_cast<Tree&&>(tree).index(*node));
      }
      else
      {
        static_cast<Tree&&>(tree).subtract_from_index(
          *node, static_cast<Tree&&>(tree).index(*child));
      }
    }

    if constexpr(Offset_trait_<Tree>::value)
    {
      if constexpr(Side::left == side)
      {
        static_cast<Tree&&>(tree).add_to_offset(
          *child, static_cast<Tree&&>(tree).offset(*node));
      }
      else
      {
        static_cast<Tree&&>(tree).subtract_from_offset(
          *node, static_cast<Tree&&>(tree).offset(*child));
      }
    }
  }

  template<class Tree>
  static void set_child_(
    Tree&& tree,
    Node<Tree>& node,
    Node_pointer<Tree> const& child_ptr,
    Side const side) noexcept
  {
    if(Side::left == side)
    {
      static_cast<Tree&&>(tree).template set_child<Side::left>(node, child_ptr);
    }
    else
    {
      TREEXX_ASSERT(Side::right == side);
      static_cast<Tree&&>(tree).template set_child<Side::right>(node, child_ptr);
    }
  }

  template<
    bool is_offset,
    class Tree, class = typename Enable_if_<!is_offset>::Type>
  [[nodiscard]] static decltype(auto) index_or_offset(
    Tree&& tree,
    Node<Tree>& node) noexcept
  {
    return static_cast<Tree&&>(tree).index(node);
  }

  template<
    bool is_offset, int = 0,
    class Tree, class = typename Enable_if_<is_offset>::Type>
  [[nodiscard]] static decltype(auto) index_or_offset(
    Tree&& tree,
    Node<Tree>& node) noexcept
  {
    return static_cast<Tree&&>(tree).offset(node);
  }

  template<bool is_offset, class Tree>
  [[nodiscard]] static auto node_index_or_offset(
    Tree&& tree,
    Node<Tree>& node) noexcept -> Index_or_offset_<Tree, is_offset>
  {
    using Node = Tree_algo::Node<Tree>;
    using Index_or_offset = Index_or_offset_<Tree, is_offset>;

    Node* node_addr = ::std::addressof(node);
    Index_or_offset result(
      index_or_offset<is_offset>(static_cast<Tree&&>(tree), node));
    for(;;)
    {
      auto const parent_ptr = static_cast<Tree&&>(tree).parent(*node_addr);
      if(parent_ptr)
      {
        Node* const parent_addr = static_cast<Tree&&>(tree).address(parent_ptr);
        TREEXX_ASSERT(parent_addr);
        if(Side::right == static_cast<Tree&&>(tree).side(*node_addr))
        {
          if constexpr(is_offset)
          {
            result += static_cast<Tree&&>(tree).offset(*parent_addr);
          }
          else
          {
            result += static_cast<Tree&&>(tree).index(*parent_addr);
          }

        }

        node_addr = parent_addr;
      }
      else
      {
        break;
      }
    }

    return result;
  }

  template<class Tree, unsigned i>
  [[nodiscard]] static auto constexpr make_index_(
    ) noexcept -> typename Index_trait_<Tree>::Type
  {
    return Remove_cv_ref_<Tree>::template make_index<i>();
  }

  template<class Tree, unsigned o>
  [[nodiscard]] static auto constexpr make_offset_(
    ) noexcept -> typename Offset_trait_<Tree>::Type
  {
    return Remove_cv_ref_<Tree>::template make_offset<o>();
  }
};

} // namespace treexx::bin::avl

#endif // TREEXX_BIN_AVL_TREEALGO_HH
