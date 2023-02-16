#pragma once

#include <map>
#include "node.hh"

class SkipList
{
private:
  // Head and Tail of the Skiplist
  node_ptr head_{nullptr};
  node_ptr tail_{nullptr};

  int find(int key, std::vector<deref_ptr> &predecessors,
           std::vector<deref_ptr> &successors);

public:
  SkipList();
  ~SkipList();

  // Supported operations
  bool add(int key);
  bool search(int key);
  bool remove(int key);
};
