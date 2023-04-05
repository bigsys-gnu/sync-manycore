/**
   Implements the Concurrent Skip list data structure with insert, delete,
   search and range operations
*/

#include "debug.hh"
#include <iostream>
#include <limits>
#include <cstdio>
#include <random>
#include <cassert>
#include <algorithm>
#include <map>
#include "skip_list.hh"
#include "mvrlu_api.hh"

constexpr const auto INT_MINI = std::numeric_limits<int>::min();
constexpr const auto INT_MAXI = std::numeric_limits<int>::max();

SkipList::SkipList()
{
  head_ = new node_t(INT_MINI, MAX_LEVEL);
  tail_ = new node_t(INT_MAXI, MAX_LEVEL);

  for (auto& n : head_->next)
    {
      n = tail_;
    }
}

/*
 * Must call this function within MV-RLU Session
 */
deref_ptr SkipList::find(int key, std::vector<deref_ptr> &predecessors)
{
  deref_ptr target;
  deref_ptr pred = head_;
  predecessors.resize(MAX_LEVEL + 1);
  for (int level = MAX_LEVEL; level >= 0; level--)
    {
      deref_ptr curr = pred->next[level];
      while (key > curr->get_key())
        {
          pred = curr;
          curr = deref_ptr(curr->next[level]);
        }
      target = curr;
      predecessors[level] = pred;
    }
  return target;
}

bool lock_preds(std::vector<deref_ptr> &preds)
{
  std::map<int, deref_ptr> set;

  for (auto ptr : preds)
    {
      set[ptr->get_key()] = ptr;
    }


  for (auto iter = set.rbegin(); iter != set.rend(); iter++)
    {
      // If we use MVRLU_TRY_LOCK, there might be a bug
      // So, we have to use mutex try lock to not to allow other threads entering critical section.
      std::unique_lock<std::mutex> guard{*iter->second->lock_ptr, std::defer_lock};
      if (!guard.try_lock() || !iter->second.try_lock())
        {
          return false;
        }
    }

  for (auto &ptr : preds)
    {
      ptr = set[ptr->get_key()];
    }

  return true;
}

/**
   Randomly generates a number and increments level if number less than or
   equal to 0.5 Once more than 0.5, returns the level or available max level.
   This decides until which level a new node_t is available.
*/
int get_random_level()
{
  static thread_local std::mt19937 gen(std::random_device{}());
  std::bernoulli_distribution dis(0.5);

  int level = 0;
  while (dis(gen))
    {
      level++;
    }
  return std::min(int(MAX_LEVEL), level);
}

/**
   Inserts into the Skip list at the appropriate place using locks.
   Return if already exists.
*/
bool SkipList::add(int key)
{
  // Get the level until which the new node must be available
  int top_level = get_random_level();

  // Initialization of references of the predecessors and successors
  std::vector<deref_ptr> preds(MAX_LEVEL + 1);

 restart:
  mvrlu_api::session session; // reader lock and unlock

  // Find the predecessors and successors of where the key must be inserted
  auto found = find(key, preds);

  if (found.get() != nullptr && found->get_key() == key)  // already exist
    {
      return false;
    }

  // create copy every preds and succs
  preds.resize(top_level + 1);
  if (!lock_preds(preds))
    {
      session.abort();
      goto restart;
    }

  // All conditions satisfied, create the node_t and insert it
  node_ptr new_node = new node_t(key, top_level);

  // Insert
  for(size_t level = 0; level < preds.size(); level++)
    {
      mvrlu_api::assign_pointer(&new_node->next[level], preds[level]->next[level]);
      mvrlu_api::assign_pointer(&preds[level]->next[level], new_node);
    }

  return true;
}

/**
   Performs search to find if a node exists.
   Return value if the key found, else return empty
*/
bool SkipList::search(int key)
{
  // Finds the predecessor and successors
  mvrlu_api::session session;

  deref_ptr pred = head_;
  deref_ptr target = pred->next[MAX_LEVEL];
  for (int level = MAX_LEVEL; level >= 0; level--)
    {
      while (key > target->get_key())
        {
          pred = target;
          target = deref_ptr(target->next[level]);
        }
      if (target->get_key() == key)
        {
          return true;
        }
    }
  return false;
}

/**
   Deletes from the Skip list at the appropriate place using locks.
   Return if key doesn’t exist in the list.
*/
bool SkipList::remove(int key)
{
  // Initialization of references of the predecessors and successors
  std::vector<deref_ptr> preds(MAX_LEVEL + 1);

 restart:
  mvrlu_api::session session; // reader lock and unlock
  // Find the predecessors and successors of where the key to be deleted
  auto victim = find(key, preds);

  if (victim.get() == nullptr || victim->get_key() != key) // no key found
    {
      return false;
    }

  // If found, select the node to delete. else return
  int top_level = victim->get_level();

  std::unique_lock<std::mutex> guard{*victim->lock_ptr, std::defer_lock};
  // create copy for every preds and victim
  if (!guard.try_lock() || !victim.try_lock_const())
    {
      session.abort();
      goto restart;
    }

  // create copy every preds and succs
  preds.resize(top_level + 1);
  if (!lock_preds(preds))
    {
      session.abort();
      goto restart;
    }

  for (size_t level = 0; level < preds.size(); level++)
    {
      mvrlu_api::assign_pointer(&preds[level]->next[level], victim->next[level]);
    }

  victim.free();
  guard.release();

  return true;
}

SkipList::~SkipList()
{
  std::size_t node_num = 0;
  for (auto iter = head_; iter != nullptr;)
    {
      auto tmp = iter;
      iter = iter->next[0];
      delete tmp;
      node_num++;
    }
  std::cout << "The Number Of Nodes: " << node_num << '\n';
}
