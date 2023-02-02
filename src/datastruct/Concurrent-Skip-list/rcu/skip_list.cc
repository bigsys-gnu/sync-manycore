/**
    Implements the Concurrent Skip list data structure with insert, delete,
   search and range operations
*/

#include "skip_list.hh"
#include <iostream>
#include <limits>
#include <map>
#include <cstdio>
#include <random>

#include "rcu_api.hh"
#include "statistics.hh"

constexpr const auto INT_MINI = numeric_limits<int>::min();
constexpr const auto INT_MAXI = numeric_limits<int>::max();

/**
    Constructor
*/
SkipList::SkipList()
{
  // MAX_LEVEL = (int) round(log(max_elements) / log(1 / prob)) - 1;
  head_ = new node_t(INT_MINI, MAX_LEVEL);
  tail_ = new node_t(INT_MAXI, MAX_LEVEL);

  for (auto iter = head_->next.begin(); iter != head_->next.end(); iter++)
    {
      *iter = tail_;
    }
}

/**
    Finds the predecessors and successors at each level of where a given key
   exists or might exist. Updates the references in the vector using pass by
   reference. Returns -1 if not the key does not exist.
*/
int SkipList::find(int key, vector<node_ptr> &predecessors, vector<node_ptr> &successors)
{
  rcu_api::reader_scope reader_session;
  int found = -1;
  auto prev = head_;

  for(int level = MAX_LEVEL; level >= 0; level--)
    {
      auto curr = prev->next[level];

      while(key > curr->get_key())
        {
          prev = curr;
          curr = prev->next[level];
        }

      if(found == -1 && key == curr->get_key())
        {
          found = level;
        }

      predecessors[level] = prev;
      successors[level] = curr;
    }
  return found;
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
  vector<node_ptr> preds(MAX_LEVEL + 1, nullptr);
  vector<node_ptr> succs(MAX_LEVEL + 1, nullptr);

  // Keep trying to insert the element into the list. In case predecessors and
  // successors are changed, this loop helps to try the insert again
  while(true)
    {
      rcu_api::reader_scope rs;
      // Find the predecessors and successors of where the key must be inserted
      int found = find(key, preds, succs);

      // If found and marked, wait and continue insert
      // If found and unmarked, wait until it is fully_linked and return. No
      // insert needed If not found, go ahead with insert
      if(found != -1)
        {
          node_ptr node_found = succs[found];

          if(!node_found->marked)
            {
              while(!node_found->fully_linked)
                {
                }
              return false;
            }
          continue;
        }

      // Store all the Nodes which lock we acquire in a map
      // Map used so that we don't try to acquire lock to a node_t we have already
      // acquired This may happen when we have the same predecessor at different
      // levels
      vector<unique_lock<recursive_mutex>> locks;

      // Traverse the skip list and try to acquire the lock of predecessor at
      // every level
      node_ptr pred;
      node_ptr succ;
      node_ptr pre_pred = nullptr;

      // Used to check if the predecessor and successors are same from when we
      // tried to read them before
      bool valid = true;

      for(int level = 0; valid && (level <= top_level); level++)
        {
          pred = preds[level];
          succ = succs[level];

          if (pred != pre_pred)
            {
              locks.emplace_back(pred->get_lock());
              pre_pred = pred;
            }

          // If predecessor marked or if the predecessor and successors change,
          // then abort and try again
          valid = !pred->marked &&
            !succ->marked && pred->next[level] == succ;
        }

      // Conditons are not met, release locks, abort and try again.
      if(!valid)
        {
          rcu_stat::count_abort();
          continue;
        }

      // All conditions satisfied, create the node_t and insert it as we have all
      // the required locks
      node_ptr new_node = new node_t(key, top_level);

      // Update the predecessor and successors
      for(int level = 0; level <= top_level; level++)
        {
          new_node->next[level] = succs[level];
          preds[level]->next[level] = new_node;
        }

      // Mark the node as completely linked.
      new_node->fully_linked = true;
      return true;
    }
}

/**
    Performs search to find if a node exists.
    Return value if the key found, else return empty
*/
bool SkipList::search(int key)
{
  rcu_api::reader_scope reader_session;

  node_ptr prev = head_;
  node_ptr found = nullptr;
  for(int level = MAX_LEVEL; level >= 0; level--)
    {
      auto curr = prev->next[level];
      while(key > curr->get_key())
        {
          prev = curr;
          curr = prev->next[level];
        }

      if(key == curr->get_key())
        {
          found = curr;
          break;
        }
    }

  // If found, unmarked and fully linked, then return value. Else return empty.
  return (found != nullptr) && found->fully_linked && !found->marked;
}

/**
    Deletes from the Skip list at the appropriate place using locks.
    Return if key doesn’t exist in the list.
*/
node_ptr SkipList::remove_impl(int key)
{
  // Initialization of references of the predecessors and successors
  vector<node_ptr> preds(MAX_LEVEL + 1, nullptr);
  vector<node_ptr> succs(MAX_LEVEL + 1, nullptr);

  node_ptr victim = nullptr;
  bool is_marked = false;
  int top_level = -1;

  // Keep trying to delete the element from the list. In case predecessors and
  // successors are changed, this loop helps to try the delete again
  while(true)
    {
      rcu_api::reader_scope rs;
      // Find the predecessors and successors of where the key to be deleted
      int found = find(key, preds, succs);

      // If found, select the node to delete. else return
      if(found != -1)
        {
          victim = succs[found];
        }
      else
        return nullptr;

      // If node not found and the node to be deleted is fully linked and not
      // marked return
      if(is_marked || (found != -1 && (victim->fully_linked && victim->top_level == size_t(found) && !(victim->marked))))
        {
          unique_lock<recursive_mutex> victim_lock(victim->get_lock(), defer_lock);
          // If not marked, the we lock the node and mark the node to delete
          if(!is_marked)
            {
              top_level = victim->top_level;
              victim_lock.lock();
              if(victim->marked)
                {
                  return nullptr;
                }
              victim->marked = true;
              is_marked = true;
            }

          // Store all the Nodes which lock we acquire in a map
          // Map used so that we don't try to acquire lock to a node_t we have already
          // acquired This may happen when we have the same predecessor at different
          // levels
          vector<unique_lock<recursive_mutex>> locks;

          // Traverse the skip list and try to acquire the lock of predecessor at
          // every level
          node_ptr pred;
          node_ptr succ;
          node_ptr pre_pred = nullptr;

          // Used to check if the predecessors are not marked for delete and if
          // the predecessor next is the node we are trying to delete or if it is
          // changed.
          bool valid = true;

          for(int level = 0; valid && (level <= top_level); level++)
            {
              pred = preds[level];
              succ = succs[level];
              if (pred != pre_pred)
                {
                  locks.emplace_back(pred->get_lock());
                  pre_pred = pred;
                }

              // If predecessor marked or if the predecessor's next has changed,
              // then abort and try again
              valid = !(pred->marked) && pred->next[level] == succ;
            }

          // Conditons are not met, release locks, abort and try again.
          if(!valid)
            {
              rcu_stat::count_abort();
              continue;
            }

          // All conditions satisfied, delete the node_t and link them to the
          // successors appropriately
          for(int level = top_level; level >= 0; level--)
            {
              preds[level]->next[level] = victim->next[level];
            }

          // delete victim;
          return victim;
        }
      else
        {
          return nullptr;
        }
    }
}

bool SkipList::remove(int key)
{
  auto victim = remove_impl(key);
  if (victim != nullptr)
    {
      // rcu_api::synchronize();
      // delete victim;
      rcu_api::free(reinterpret_cast<void *>(victim));
      return true;
    }
  return false;
}

/**
    Searches for the start_key in the skip list by traversing once we reach a
   point closer to start_key reaches to level 0 to find all keys between
   start_key and end_key. If search exceeds end, then abort Updates and returns
   the key value pairs in a map.
*/
vector<int> SkipList::range(int start_key, int end_key)
{
  vector<int> range_output;

  if(start_key > end_key)
    {
      return range_output;
    }

  rcu_api::reader_scope reader_session;

  auto curr = head_;

  for(int level = MAX_LEVEL; level >= 0; level--)
    {
      while(curr->next[level] != nullptr && start_key > curr->next[level]->get_key())
        {
          if(curr->get_key() >= start_key && curr->get_key() <= end_key)
            {
              range_output.push_back(curr->get_key());
            }
          curr = curr->next[level];
        }
    }

  while(curr != nullptr && end_key >= curr->get_key())
    {
      if(curr->get_key() >= start_key && curr->get_key() <= end_key)
        {
          range_output.push_back(curr->get_key());
        }
      curr = curr->next[0];
    }

  return range_output;
}

/**
    Display the skip list in readable format
*/
void SkipList::display()
{
  for(size_t i = 0; i < MAX_LEVEL; i++)
    {
      auto temp = head_;
      int count = 0;
      if(!(temp->get_key() == INT_MINI && temp->next[i]->get_key() == INT_MAXI))
        {
          printf("Level %ld  ", i);
          while(temp != nullptr)
            {
              printf("%d -> ", temp->get_key());
              temp = temp->next[i];
              count++;
            }
          cout << endl;
        }
      if(count == 3)
        break;
    }
  printf("---------- Display done! ----------\n\n");
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
