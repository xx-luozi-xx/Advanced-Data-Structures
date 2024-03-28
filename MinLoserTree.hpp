#pragma once
#include <vector>

#define _MIN_LOSER_TREE_VIEW__1

#ifdef _MIN_LOSER_TREE_VIEW__
#include <iostream>
#endif

template<class T>
class MinLoserTree {
public:
    MinLoserTree(const T* data, size_t size);
    MinLoserTree(const std::vector<T>&);
    MinLoserTree(const MinLoserTree<T>&) = delete;
    MinLoserTree<T>& operator=(const MinLoserTree<T>&) = delete;
    ~MinLoserTree();

    void remake(const std::vector<T>&);
    size_t size()const;
    const T& top()const;

    void change_top(const T& val);
    const T& operator[](size_t idx)const;

    void swap(MinLoserTree<T>&);

protected:
    T** tree_;//树区
    T* data_;//原始数据
    size_t size_;

    T** data_begin_;//start to point to data

private:
    T* make_tree_(size_t); // return the pointer that point to the winner

#ifdef _MIN_LOSER_TREE_VIEW__
public:
    void look_tree();

#endif
};
template<class T>
MinLoserTree<T>::MinLoserTree(const T* init, size_t arr_size) :tree_(nullptr), data_(nullptr), size_(arr_size) {
    data_ = new T[size_]();
    for (int i = 0; i < size_; ++i) {
        data_[i] = init[i];
    }

    size_t cpcty = 1;
    while (cpcty < size_) {
        cpcty <<= 1;
    }

    tree_ = new T * [cpcty << 1] {};
    data_begin_ = tree_ + cpcty;
    for (int i = 0; i < size_; ++i) {
        data_begin_[i] = data_ + i;
    }

    *tree_ = make_tree_(1llu);//tree_[0] is the winner
}

template<class T>
MinLoserTree<T>::MinLoserTree(const std::vector<T>& init) :tree_(nullptr), data_(nullptr), size_(init.size()) {
    data_ = new T[size_]();
    for (int i = 0; i < size_; ++i) {
        data_[i] = init[i];
    }

    size_t cpcty = 1;
    while (cpcty < size_) {
        cpcty <<= 1;
    }

    tree_ = new T * [cpcty << 1] {};
    data_begin_ = tree_ + cpcty;
    for (int i = 0; i < size_; ++i) {
        data_begin_[i] = data_ + i;
    }

    *tree_ = make_tree_(1llu);//tree_[0] is the winner
}

template<class T>
T* MinLoserTree<T>::make_tree_(size_t root) {

    if (tree_ + root >= data_begin_) {//* 外部节点
        return tree_[root];
    }

    T* r_win = make_tree_(root << 1);
    T* l_win = make_tree_((root << 1) + 1);

    if (l_win == nullptr || *r_win < *l_win) {//* r win
        tree_[root] = l_win;
        return r_win;
    }
    else {//* l win
        tree_[root] = r_win;
        return l_win;
    }
}

template<class T>
MinLoserTree<T>::~MinLoserTree() {
    delete[] data_;
    delete[] tree_;
}
template<class T>
void MinLoserTree<T>::remake(const std::vector<T>& init) {
    MinLoserTree<T> tmp(init);
    swap(tmp);
}
template<class T>
void MinLoserTree<T>::swap(MinLoserTree<T>& rhs) {
    std::swap(tree_, rhs.tree_);
    std::swap(data_, rhs.data_);
    std::swap(size_, rhs.size_);
    std::swap(data_begin_, rhs.data_begin_);
}
template<class T>
size_t MinLoserTree<T>::size()const {
    return size_;
}
template<class T>
const T& MinLoserTree<T>::top()const {
    return **tree_;
}
template<class T>
const T& MinLoserTree<T>::operator[](size_t idx)const {
    return *data_begin_[idx];
}
template<class T>
void MinLoserTree<T>::change_top(const T& val) {
    size_t tree_idx = data_begin_ - tree_ + *tree_ - data_;
    data_[*tree_ - data_] = val;
    T* winner = *tree_;
    while (tree_idx > 1) {
        tree_idx >>= 1;
        if (tree_[tree_idx] == nullptr || *tree_[tree_idx] >= *winner) {//loser still lose
            continue;
        }
        else {
            std::swap(tree_[tree_idx], winner);
        }
    }
    *tree_ = winner;
}
#ifdef _MIN_LOSER_TREE_VIEW__
template<class T>
void MinLoserTree<T>::look_tree() {
    size_t idx = 0;
    for (auto itr = tree_; itr != data_begin_ + size_; ++itr, ++idx) {
        if (*itr) {
            std::cout << '[' << idx << ']' << **itr << std::endl;
        }
        else {
            std::cout << '[' << idx << ']' << 'n' << std::endl;
        }
    }
}
#include <stdlib.h>
int main() {
    std::vector<int> init;
    for (int i = 0; i < 100; ++i) {
        init.push_back(rand());
    }
    MinLoserTree<int> loserTree(init);
    for (int i = 0; i < 100; ++i) {
        printf("%d\n", loserTree.top());
        loserTree.change_top(0x3f3f3f3f);
    }
}

#endif