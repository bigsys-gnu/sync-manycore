#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <array>
#include <utility>
#include "mvrlu_api.hh"

using namespace std;

template <size_t L>
class Node
{
  int key_{0};

public:

  MVRLU_NEW_DELETE(Node<L>)

  // Stores the reference of the next node until the top level for the node
  array<Node *, L + 1> next {nullptr};

  const int top_level{L};

  Node();
  Node(int key, int top_level);
  ~Node();
  int get_key() const;
};

template <size_t L>
Node<L>::Node()
{
}

template <size_t L>
Node<L>::Node(int key, int level):
  key_(key), top_level(level)
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

constexpr const size_t MAX_LEVEL = 255;
using node_t = Node<MAX_LEVEL>;
using node_ptr = node_t *;
