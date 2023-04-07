#pragma once
#include <cstddef>
// dynamic list implementation using array

template <typename T> class List {
  private:
    T* arr;
    size_t size;
    size_t capacity;

  public:
    List()
    {
        size     = 0;
        capacity = 1;
        arr      = new T[capacity];
    }
    // copy constructor
    List(const List& other)
    {
        size     = other.size;
        capacity = other.capacity;
        arr      = new T[capacity];
        for (size_t i = 0; i < size; i++) {
            arr[i] = other.arr[i];
        }
    }
    // move constructor
    List(List&& other)
    {
        size      = other.size;
        capacity  = other.capacity;
        arr       = other.arr;
        other.arr = nullptr;
    }
    ~List() { delete[] arr; }

    // copy assignment
    List& operator=(const List& other)
    {
        if (this == &other)
            return *this;
        delete[] arr;
        size     = other.size;
        capacity = other.capacity;
        arr      = new T[capacity];
        for (size_t i = 0; i < size; i++) {
            arr[i] = other.arr[i];
        }
        return *this;
    }

    void push_back(T value)
    {
        if (size == capacity) {
            capacity *= 2;
            T* new_arr = new T[capacity];
            for (size_t i = 0; i < size; i++) {
                new_arr[i] = arr[i];
            }
            delete[] arr;
            arr = new_arr;
        }
        arr[size] = value;
        size++;
    }

    T& operator[](size_t index)
    {
        if (index >= size)
            throw "Index out of bounds";
        return arr[index];
    }
    size_t get_size() { return size; }
    T last() { return arr[size - 1]; }
};
