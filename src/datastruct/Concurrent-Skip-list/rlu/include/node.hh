#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <array>
#include <utility>
#include "mvrlu_api.hh"

template <size_t L> class Node;

constexpr const size_t MAX_LEVEL = 17;
using node_t = Node<MAX_LEVEL>;
using node_ptr = node_t *;
using deref_ptr = mvrlu_api::derefered_ptr<node_t>;

template <size_t L>
class Node
{
  int key_{0};

public:

  MVRLU_NEW_DELETE(Node<L>)

private:
  class slot
  {
  private:
    Node *next_{nullptr};
    int next_key_{0};
  public:
    slot()
    {
    }

    slot(Node *next):
      next_{next}
    {
    }

    slot(Node *next, int key)
    {
      mvrlu_api::assign_pointer(&next_, next);
      next_key_ = key;
    }

    mvrlu_api::derefered_ptr<Node> get_next() const
    {
      return mvrlu_api::derefered_ptr<Node>(next_);
    }

    int get_next_key() const
    {
      return next_key_;
    }

    void set_next(const slot& o, int next_key)
    {
      mvrlu_api::assign_pointer(&next_, o.next_);
      next_key_ = next_key;
    }
  };

  // Stores the reference of the next node until the top level for the node
  std::array<slot, L + 1> next_;

  const int top_level_{L};

public:
  Node();
  Node(int key, int top_level);
  ~Node();
  int get_key() const;
  int get_level() const;
  int get_next_key(int level) const;
  void set_next(Node *next, int level);
  mvrlu_api::derefered_ptr<Node> get_next(int level) const;
};

template <size_t L>
using slot_t = typename Node<L>::slot;

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

template <size_t L>
int Node<L>::get_next_key(int level) const
{
  return next_[level].get_next_key();
}

template <size_t L>
void Node<L>::set_next(Node *next, int level)
{
  next_[level].set_next(slot{next, level}, next->get_key());
}


template <size_t L>
mvrlu_api::derefered_ptr<Node<L>> Node<L>::get_next(int level) const
{
  return next_[level].get_next();
}
