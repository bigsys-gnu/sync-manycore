#pragma once

#include <map>
#include <memory>
#include "node.hh"

class SkipList
{
private:
  // Head and Tail of the Skiplist
  node_ptr head_{nullptr};
  node_ptr tail_{nullptr};

public:
  SkipList();
  ~SkipList();
  int get_random_level();

  // Supported operations
  int find(int key, vector<node_ptr> &predecessors, vector<node_ptr> &successors);
  bool add(int key, string value);
  string search(int key);
  bool remove(int key);
  map<int, string> range(int start_key, int end_key);
  void display();
};
