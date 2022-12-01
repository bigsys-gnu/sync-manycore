#pragma once

#include <map>
#include "node.hh"

using deref_ptr = mvrlu_api::derefered_ptr<node_t>;

class SkipList
{
private:
  // Head and Tail of the Skiplist
  node_ptr head_{nullptr};
  node_ptr tail_{nullptr};

  int find(int key, vector<deref_ptr> &predecessors, vector<deref_ptr> &successors);

public:
  SkipList();
  ~SkipList();

  // Supported operations
  bool add(int key);
  bool search(int key);
  bool remove(int key);
  void display();
};
