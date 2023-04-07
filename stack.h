#pragma once

#include <cstddef>

template <typename T> class Stack {
  private:
    struct Node {
        T val;
        Node* next;
    };

    Node* head;
    size_t size;

  public:
    Stack()
    {
        head = nullptr;
        size = 0;
    }

    ~Stack()
    {
        while (!is_empty())
            pop();
    }

    size_t get_size() { return size; }

    bool is_empty() { return size == 0; }

    void push(T val)
    {
        Node* new_node = new Node;
        new_node->val  = val;
        new_node->next = head;
        head           = new_node;
        size++;
    }

    T pop()
    {
        if (is_empty())
            throw "Stack is empty";
        Node* temp = head;
        head       = head->next;
        T val      = temp->val;
        delete temp;
        size--;
        return val;
    }

    T peek()
    {
        if (is_empty())
            throw "Stack is empty";
        return head->val;
    }
};