#include "lib/skip_list.h"
#include <iostream>
#include <thread>

using namespace std;

template <typename T>
std::ostream& operator<<(std::ostream& out, const LazySkipList<T>& lsl) {
  out << "list: " << endl;
  auto p = lsl.head->nexts[0];
  while (p != lsl.tail) {
    out << p->item << "\t";
    p = p->nexts[0];
  }
  out << endl;
  return out;
}

int main() {
  auto l = LazySkipList<int>();
  cout << l.empty() << endl;

  std::thread t1(&LazySkipList<int>::add, l, 1, 1);
  std::thread t2(&LazySkipList<int>::add, l, 2, 2);
  std::thread t3(&LazySkipList<int>::add, l, 3, 3);

  t1.join(); t2.join(); t3.join();
  cout << l.empty() << endl;
  cout << l << endl;

  std::thread t4(&LazySkipList<int>::remove, l, 3);
  std::thread t5(&LazySkipList<int>::remove, l, 3);
  std::thread t6(&LazySkipList<int>::remove, l, 2);

  t4.join(); t5.join(); t6.join();
  cout << l << endl;

  return 0;
}
