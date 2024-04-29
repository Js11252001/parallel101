/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复 答：会造成节点相互引用无法释放
    std::unique_ptr<Node> next;
    Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？答：使用初始化列表
    Node(int val) : value{val} {}

    //insert函数有问题
    //void insert(int val) {
    //    auto node = std::make_shared<Node>(val);
    //    node->next = next;
    //    node->prev = prev;
    //    if (prev)
    //        prev->next = node;
    //    if (next)
    //        next->prev = node;
    //}

    void erase() {
        Node* t = next.get();
        if (prev)
            prev->next = std::move(next);
        if (t)
            t->prev = prev;
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？shared_ptr相互引用导致无法释放资源.
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        //head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        head = std::make_unique<Node>(other.front()->value);
        int i = 1;
        Node* t = head.get();
        for (auto curr = other.front()->next.get(); curr; curr = curr->next.get()) {
            t->next = std::make_unique<Node>(curr->value);
            t->next->prev = t;
            t = t->next.get();
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？因为实现了移动赋值函数后，
    //List b= a可被编译器视为List b = List(a),先进行拷贝构造因为是临时对象再进行移动赋值

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        Node* t = head.get();
        node->next = std::move(head);
        if (t)
            t->prev = node.get();
        head = std::move(node);
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(List const & lst) {  // 有什么值得改进的？传递参数用const引用的方式
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

int main() {
    List a;

    a.push_front(7);
    a.push_front(5);
    a.push_front(8);
    a.push_front(2);
    a.push_front(9);
    a.push_front(4);
    a.push_front(1);

    print(a);   // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();

    print(a);   // [ 1 4 2 8 5 7 ]

    List b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
