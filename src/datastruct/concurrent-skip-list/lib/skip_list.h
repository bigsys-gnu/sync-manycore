#ifndef LAZYSKIPLIST_NODE_H
#define LAZYSKIPLIST_NODE_H

#include <limits>
#include <ctime>
#include <random>
#include <cmath>
#include <mutex>
#include <memory>
#include <vector>

template <typename T>
class LazySkipList {
public:
  static const int MAX_LEVEL = 31;

  class Node {
  public:
    T item;
    int key;
    int topLayer;
    bool marked;
    bool fullyLinked;

    std::shared_ptr<Node> nexts[MAX_LEVEL+1];
    std::recursive_mutex nodeMutex;

    Node(int k) : item(), key(k), topLayer(MAX_LEVEL), marked(false), fullyLinked(false),
                  nodeMutex() {
      for (int i = 0; i < topLayer; i++)
        nexts[i] = nullptr;
    }
    Node(T x, int height, int k) : item(x), topLayer(height), key(k), marked(false), fullyLinked(false),
                                   nodeMutex() {
      for (int i = 0; i < topLayer; i++)
        nexts[i] = nullptr;
    }
  };

  std::default_random_engine engine;
  std::uniform_int_distribution<unsigned int> distribution;
  std::shared_ptr<Node> head;
  std::shared_ptr<Node> tail;
  using u_lock = std::unique_lock<std::recursive_mutex>;

  inline unsigned randomLayer() {
    auto randNum = distribution.operator()(engine);
    unsigned two__30 = static_cast<unsigned>(pow(2, 30));
    if (randNum == 0)
      return 31; // p(level==31) = 2**-31
    else if (randNum >= two__30)
      return 0; // p(level=0) = 0.75
    else
      return 30 - static_cast<unsigned>(log2(randNum)); // p(level=i) = 2**-(i+2)
  }

  inline int find(int k, std::shared_ptr<Node> preds[], std::shared_ptr<Node> succs[]) {
    int lFound = -1;
    std::shared_ptr<Node> pred = head;
    for (int layer = MAX_LEVEL - 1; layer >= 0; layer--) {
      std::shared_ptr<Node> curr = pred->nexts[layer];
      while (k > curr->key) {
        pred = curr;
        curr = pred->nexts[layer];
      }
      if (lFound == -1 and k == curr->key) {
        lFound = layer;
      }
      preds[layer] = pred;
      succs[layer] = curr;
    }
    return lFound;
  }

  bool okToDelete(std::shared_ptr<Node> candidate, int lFound) {
    return (candidate->fullyLinked and
            candidate->topLayer == lFound and
            !candidate->marked);
  }

  LazySkipList() : engine(time(nullptr)), distribution() {
    head = std::make_shared<Node>(std::numeric_limits<int>::lowest());
    tail = std::make_shared<Node>(std::numeric_limits<int>::max());
    for (int i = 0; i < MAX_LEVEL; i++) {
      head->nexts[i] = tail;
    }
  };

public:
  bool add(T x, int key) {
    int topLayer = randomLayer();
    std::shared_ptr<Node> preds[MAX_LEVEL+1];
    std::shared_ptr<Node> succs[MAX_LEVEL+1];

    while (true) {
      auto lFound = find(key, preds, succs);
      if (lFound != -1) {
        std::shared_ptr<Node> nodeFound = succs[lFound];
        if (!nodeFound->marked) {
          while (!nodeFound->fullyLinked) ;
          //ofs.close();
          return false;
        }
        continue;
      }
      int highestLocked = -1;
      std::shared_ptr<Node> pred, succ, prevPred = nullptr;
      bool valid = true;
      int layer_count = topLayer + 1;
      std::vector<u_lock> lcks;
      for (int layer = 0; valid and layer <= topLayer; layer++) {
        pred = preds[layer];
        succ = succs[layer];
        if (pred != prevPred) {
          lcks.emplace_back(u_lock(pred->nodeMutex));
          highestLocked = layer;
          prevPred = pred;
        }
        valid = !pred->marked and !succ->marked and pred->nexts[layer] == succ;
      }
      if (!valid) {
        for (int layer = 0; layer <= highestLocked; layer++) {
          lcks[layer].unlock();
        }
        continue;
      }

      auto newNode = std::make_shared<Node>(x, topLayer, key);
      for (int layer = 0; layer <= topLayer; layer++) {
        newNode->nexts[layer] = succs[layer];
        preds[layer]->nexts[layer] = newNode;
      }
      newNode->fullyLinked = true;
      for (int layer = 0; layer <= highestLocked; layer++) {
        lcks[layer].unlock();
      }
      return true;
    }
  }

  bool remove(int key) {
    std::shared_ptr<Node> nodeToDelete = nullptr;
    bool isMarked = false;
    int topLayer = -1;
    std::shared_ptr<Node> preds[MAX_LEVEL+1];
    std::shared_ptr<Node> succs[MAX_LEVEL+1];
    while (true) {
      int lFound = find(key, preds, succs);
      if (isMarked or (lFound != -1 and okToDelete(succs[lFound], lFound))) {
        nodeToDelete = succs[lFound];
        u_lock nodeLock(nodeToDelete->nodeMutex, std::defer_lock);
        if (!isMarked) {
          topLayer = nodeToDelete->topLayer;
          nodeLock.lock();
          if (nodeToDelete->marked) {
            // ulck will unlock at return
            return false;
          }
          nodeToDelete->marked = true;
          isMarked = true;
        }

        int highestLocked = -1;
        std::shared_ptr<Node> pred, succ, prevPred = nullptr;
        bool valid = true;
        std::vector<u_lock> lcks;
        for (int layer = 0; valid and layer <= topLayer; layer++) {
          pred = preds[layer];
          succ = succs[layer];
          if (pred != prevPred) {
            lcks.emplace_back(u_lock(pred->nodeMutex));
            highestLocked = layer;
            prevPred = pred;
          }
          valid = !pred->marked and pred->nexts[layer] == succ;
        }
        if (!valid) {
          for (int i = 0; i < highestLocked; i++)
            lcks[i].unlock();
          continue;
        }

        for (int layer = topLayer; layer >= 0; layer--) {
          preds[layer]->nexts[layer] = nodeToDelete->nexts[layer];
        }
        // RAII: nodeLock and lcks will unlock when return
        return true;
      }
      else
        return false;
    }
  }

  bool empty() {
    return head->nexts[0] == tail;
  }

  // may not actually pop first item, but I'm ok with this
  // works same as remove(), gets head->nexts[0]->key as key,
  // though it could not be the first one when removed
  std::shared_ptr<Node> pop() {
    std::shared_ptr<Node> nodeToDelete = nullptr;
    bool isMarked = false;
    int topLayer = -1;
    std::shared_ptr<Node> preds[MAX_LEVEL+1];
    std::shared_ptr<Node> succs[MAX_LEVEL+1];
    auto key = head->nexts[0]->key;
    while (true) {
      int lFound = find(key, preds, succs);
      if (isMarked or (lFound != -1 and okToDelete(succs[lFound], lFound))) {
        nodeToDelete = succs[lFound];
        u_lock nodeLock(nodeToDelete->nodeMutex, std::defer_lock);
        if (!isMarked) {
          topLayer = nodeToDelete->topLayer;
          nodeLock.lock();
          if (nodeToDelete->marked) {
            // ulck will unlock at return
            return nullptr;
          }
          nodeToDelete->marked = true;
          isMarked = true;
        }

        int highestLocked = -1;
        std::shared_ptr<Node> pred, succ, prevPred = nullptr;
        bool valid = true;
        std::vector<u_lock> lcks;
        for (int layer = 0; valid and layer <= topLayer; layer++) {
          pred = preds[layer];
          succ = succs[layer];
          if (pred != prevPred) {
            lcks.emplace_back(u_lock(pred->nodeMutex));
            highestLocked = layer;
            prevPred = pred;
          }
          valid = !pred->marked and pred->nexts[layer] == succ;
        }
        if (!valid) {
          for (int i = 0; i < highestLocked; i++)
            lcks[i].unlock();
          continue;
        }

        for (int layer = topLayer; layer >= 0; layer--) {
          preds[layer]->nexts[layer] = nodeToDelete->nexts[layer];
        }
        // RAII: nodeLock and lcks will unlock when return
        return nodeToDelete;
      }
      else
        return nullptr;
    }
  }
};

#endif //LAZYSKIPLIST_NODE_H
