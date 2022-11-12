#pragma once

#include <map>
#include "node.h"

constexpr const size_t MAX_LEVEL = 255;
using node_t = Node<MAX_LEVEL>;

class SkipList
{
private:
  // Head and Tail of the Skiplist
  node_t *head_{nullptr};
  node_t *tail_{nullptr};

public:
  SkipList();
  ~SkipList();
  int get_random_level();

  // Supported operations
  int find(int key, vector<node_t *> &predecessors, vector<node_t *> &successors);
  bool add(int key, string value);
  string search(int key);
  bool remove(int key);
  map<int, string> range(int start_key, int end_key);
  void display();
};