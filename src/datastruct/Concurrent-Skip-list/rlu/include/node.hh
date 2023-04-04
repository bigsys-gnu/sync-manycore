#pragma once

#include <vector>
#include <mutex>
#include <memory>
#include <array>
#include <utility>
#include "mvrlu_api.hh"

template <size_t L> class Node;

constexpr const int MAX_LEVEL = 17;
using node_t = Node<MAX_LEVEL>;
using node_ptr = node_t *;
using deref_ptr = mvrlu_api::derefered_ptr<node_t>;

template <size_t L>
class Node
{
  int key_{0};

public:

  MVRLU_NEW_DELETE(Node<L>)

  // Stores the reference of the next node until the top level for the node
  std::array<Node *, L + 1> next{nullptr};

  std::unique_ptr<std::mutex> lock_ptr = std::make_unique<std::mutex>();

private:

  const int top_level_{L};

public:
  Node();
  Node(int key, int top_level);
  ~Node();
  int get_key() const;
  int get_level() const;
};

template <size_t L>
Node<L>::Node()
{
}

template <size_t L>
Node<L>::Node(int key, int level):
  key_(key), top_level_(level)
{
}

/**
    Returns the key in the node
*/
template <size_t L>
int Node<L>::get_key() const
{
  return key_;
}

template <size_t L>
Node<L>::~Node()
{
}

template <size_t L>
int Node<L>::get_level() const
{
  return top_level_;
}
