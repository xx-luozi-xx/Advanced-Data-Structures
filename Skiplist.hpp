#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <tuple>

#include "LZ_algorithm.h"

#define MAX_H 31

#define OP_COUNT 10
#define OP_BOUND 4

#define _VIEW_STRUCT__1
#define _VIEW__1
#define _DEBUG__1

template<class K, class V>
class SkipList {
private:
    struct Node {
        std::pair<K, V> data_;
        std::vector<Node*> nexts_;
        std::vector<Node*> pre_;
    };

public:

    class Iterator {
        friend class SkipList;
    public:
        Iterator() = default;
        Iterator(Node*);

        Iterator& operator++();
        Iterator operator++(int);
        Iterator& operator--();
        Iterator operator--(int);

        std::pair<K, V>* operator->()const;
        std::pair<K, V>& operator* ()const;

        bool operator==(const Iterator&)const;
        bool operator!=(const Iterator&)const;

    private:
        Node* node_ = nullptr;
    };
public:

    SkipList(const std::vector<std::pair<K, V>>&);
    SkipList(const SkipList<K, V>&) = delete;
    SkipList& operator=(const SkipList<K, V>&) = delete;
    ~SkipList();

    bool empty() const;
    size_t size() const;
    Iterator find(const K&);

    Iterator insert(const K&, const V&); 
    Iterator erase(const K&);
    Iterator erase(Iterator&);

    void pop_front();
    void pop_back();

    Iterator begin() const;
    Iterator end() const;

    void show_struct()const;

protected:
    Node* head_;
    Node* end_;
    size_t size_;

protected:
    int max_h_() const;
    
    std::vector<size_t> std_op_cnt_;
    void try_restruct_(size_t);
    void restruct_();
};

template<class K, class V>
SkipList<K, V>::Iterator::Iterator(Node* node) :node_(node) {};

template<class K, class V>
typename std::pair<K, V>* SkipList<K, V>::Iterator::operator->()const {
    return  &(node_->data_);
}
template<class K, class V>
typename std::pair<K, V>& SkipList<K, V>::Iterator::operator* ()const {
    return  (node_->data_);
}
template<class K, class V>
typename SkipList<K, V>::Iterator& SkipList<K, V>::Iterator::operator++() {
    node_ = node_->nexts_[0];
    return *this;
}
template<class K, class V>
typename SkipList<K, V>::Iterator SkipList<K, V>::Iterator::operator++(int) {
    SkipList<K, V>::Iterator ret = *this;
    node_ = node_->nexts_[0];
    return ret;
}
template<class K, class V>
typename SkipList<K, V>::Iterator& SkipList<K, V>::Iterator::operator--() {
    node_ = node_->pre_[0];
    return *this;
}
template<class K, class V>
typename SkipList<K, V>::Iterator SkipList<K, V>::Iterator::operator--(int) {
    SkipList<K, V>::Iterator ret = *this;
    node_ = node_->pre_[0];
    return ret;
}
template<class K, class V>
bool SkipList<K, V>::Iterator::operator==(const Iterator& rhs)const {
    return node_ == rhs.node_;
}
template<class K, class V>
bool SkipList<K, V>::Iterator::operator!=(const Iterator& rhs)const {
    return node_ != rhs.node_;
}
template<class K, class V>
SkipList<K, V>::SkipList(const std::vector<std::pair<K, V>>& input) :size_(input.size()) {
    srand(time(0)^rand());
    head_ = new Node();
    end_ = new Node();

    head_->nexts_ = std::vector<Node*>(MAX_H, end_);
    head_->pre_ = std::vector<Node*>(MAX_H, nullptr);
    end_->nexts_ = std::vector<Node*>(MAX_H, nullptr);
    end_->pre_ = std::vector<Node*>(MAX_H, head_);

#ifdef _DEBUG__
    printf("head&end\n");
#endif
    std::vector<std::pair<K, V>> init(input);
    sort(&init[0], &init[0]+init.size());

    std::vector<Node*> pre(MAX_H, head_);
    std::vector<Node*> nxt(MAX_H, end_);
#ifdef _DEBUG__    
    printf("begin init\n");
#endif
    //*init
    for (unsigned i = 0; i < init.size(); ++i) {
#ifdef _DEBUG__
        printf("init round %d\n", i + 1);
#endif
        Node* newNode = new Node();
        newNode->data_ = init[i];

        unsigned I = i + 1;
        unsigned lbit = I&(-I);
        int h = 0;
        while (lbit) {
            if(h==MAX_H){
                break;
            }
            newNode->nexts_.push_back(nxt[h]);
            newNode->pre_.push_back(pre[h]);
            nxt[h]->pre_[h] = newNode;
            pre[h]->nexts_[h] = newNode;

            pre[h] = newNode;

            h++;
            lbit >>= 1;
        }
#ifdef _DEBUG__
        printf("end init round %d\n", i + 1);
#endif
#ifdef _VIEW__
        printf("init#%d h=%d\n", i + 1, h);
#endif
    }
}
template<class K, class V>
void SkipList<K, V>::try_restruct_(size_t op_cnt){
    static long long sum = -1;
    if(std_op_cnt_.size() < OP_COUNT){
        std_op_cnt_.push_back(op_cnt);
        sum = -1;
#ifdef _VIEW_STRUCT__
        printf("%llu 0\n", op_cnt);
#endif
    }else{
        if(sum < 0){
            sum = 0;
            for(int i = 0; i < std_op_cnt_.size(); ++i){
                sum += std_op_cnt_[i];
            }
        }
        //sum is useful
        if(op_cnt * OP_COUNT > sum * OP_BOUND){
            restruct_();
#ifdef _VIEW_STRUCT__
        printf("%llu 2\n", op_cnt/*, sum/OP_COUNT*/);
#endif
        }
#ifdef _VIEW_STRUCT__
        else{
            printf("%llu 1\n", op_cnt/*, sum/OP_COUNT*/);
        }
#endif
    }
    return;
}
template<class K, class V>
void SkipList<K, V>::restruct_(){
    std_op_cnt_.clear();
    Node* next_node = head_->nexts_[0];
    Node* pre_node = end_->pre_[0];
    head_->nexts_ = std::vector<Node*>(MAX_H, end_);
    end_->pre_ = std::vector<Node*>(MAX_H, head_);
    head_->nexts_[0] = next_node;
    end_->pre_[0] = pre_node;

    std::vector<Node*> pre(MAX_H, head_);
    std::vector<Node*> nxt(MAX_H, end_);
    unsigned I = 1;
    for(Node* ind = head_->nexts_[0]; ind != end_ ; ind = ind->nexts_[0], ++I){
        Node*& newNode = ind;
        next_node = newNode->nexts_[0];
        pre_node = newNode->pre_[0];
        newNode->nexts_.clear();
        newNode->pre_.clear();
        newNode->nexts_.push_back(next_node);
        newNode->pre_.push_back(pre_node);
    
        unsigned lbit = I&(-I); lbit >>= 1;
        int h = 1;
        while (lbit) {
            if(h == MAX_H){
                break;
            }
            newNode->nexts_.push_back(nxt[h]);
            newNode->pre_.push_back(pre[h]);
            nxt[h]->pre_[h] = newNode;
            pre[h]->nexts_[h] = newNode;

            pre[h] = newNode;

            h++;
            lbit >>= 1;
        }
    }   
}
template<class K, class V>
void SkipList<K, V>::show_struct()const {
    size_t i = 0;
    for (Node* itr = head_; itr != nullptr; itr = itr->nexts_[0]) {
        printf("node#%llu h:%d\n", i++, itr->nexts_.size());
    }
}
template<class K, class V>
SkipList<K, V>::~SkipList() {
    while (head_->nexts_[0] != end_) {
        Node* del = head_->nexts_[0];
        head_->nexts_[0] = head_->nexts_[0]->nexts_[0];
        delete del;
    }
    delete head_;
    delete end_;
}

template<class K, class V>
bool SkipList<K, V>::empty()const {
    return !size_;
}

template<class K, class V>
size_t SkipList<K, V>::size()const {
    return size_;
}
template<class K, class V>
typename SkipList<K, V>::Iterator SkipList<K, V>::insert(const K& key, const V& val) {
    Node* now = head_;
    int flr = max_h_();
    size_t op_cnt = 0;
    std::vector<Node*> pre(MAX_H, head_);
#ifdef _VIEW__
    printf("find:begin flr:%d\n", flr);
#endif
    while (now != end_ && flr >= 0) {
        ++op_cnt;
        if ((now == head_ || now->data_.first < key) && (now->nexts_[flr] == end_ || now->nexts_[flr]->data_.first > key)) {//头小尾大，下沉
            pre[flr] = now;
            flr--;
#ifdef _VIEW__
            printf("find:to flr:%d\n", flr);
#endif
        }
        else if (now != head_ && now->data_.first > key) {//区间头大，直接没有，插入点前一个为头
            now = head_;//* 这种情况实际上没有
            break;
        }
        else if (now != head_ && now->data_.first == key) {//区间头等于
            break;
        }
        else if (now->nexts_[flr] != end_ && now->nexts_[flr]->data_.first <= key) {//区间尾小，平移
            now = now->nexts_[flr];

#ifdef _VIEW__
            printf("find:next node\n");
#endif
        }
    }
    if(now==head_||now->data_.first != key){//插在后面
#ifdef _VIEW__
        printf("insert:add key %d\n", key);
#endif
        Node* newNode = new Node();
        newNode->data_ = std::pair<K,V>(key, val);
        
        //随机数求lbit ,然后从0层开始构建
        unsigned r = 0;
        while(!r){
            r<<=15; r|=rand(); 
            r<<=15; r|=rand(); 
            r<<=15; r|=rand();
        } 

        unsigned lbit = r&(-r);
#ifdef __VIEW_
        printf("RAND: %x lbit: %u\n", r, lbit);
#endif
        int h = 0;
        while(lbit){
            if(h == MAX_H){
                break;
            }
            Node* next = pre[h]->nexts_[h];

            newNode->nexts_.push_back(next);
            newNode->pre_.push_back(pre[h]);
            next->pre_[h] = newNode;
            pre[h]->nexts_[h] = newNode;

            lbit >>= 1;
            ++h;
        }
#ifdef _VIEW__
        printf("insert:new key %d, flr %d\n", key, h);
#endif
        now = newNode;
        ++size_;
    }else{//直接更改
#ifdef _VIEW__
        printf("insert:updata key %d\n", key);
#endif
        now->data_.second = val;
    }
    try_restruct_(op_cnt);
    return Iterator(now);
}
template<class K, class V>
typename SkipList<K, V>::Iterator SkipList<K, V>::find(const K& key){
    Node* now = head_;
    int flr = max_h_();
    size_t op_cnt = 0;
#ifdef _VIEW__
    printf("find:begin flr:%d\n", flr);
#endif
    while (now != end_ && flr >= 0) {
        ++op_cnt;
        if ((now == head_ || now->data_.first < key) && (now->nexts_[flr] == end_ || now->nexts_[flr]->data_.first > key)) {//头小尾大，下沉
            flr--;
#ifdef _VIEW__
            printf("find:to flr:%d\n", flr);
#endif
        }
        else if (now != head_ && now->data_.first > key) {//区间头大，直接没有
            try_restruct_(op_cnt);
            return end();
        }
        else if (now != head_ && now->data_.first == key) {//区间头等于
            try_restruct_(op_cnt);
            return SkipList<K, V>::Iterator(now);
        }
        else if (now->nexts_[flr] != end_ && now->nexts_[flr]->data_.first <= key) {//区间尾小，平移
            now = now->nexts_[flr];
#ifdef _VIEW__
            printf("find:next node\n");
#endif
        }
    }
    try_restruct_(op_cnt);
    return end();
}
template<class K, class V>
typename SkipList<K, V>::Iterator SkipList<K, V>::erase(Iterator& itr) {
    if (itr == end() || itr == Iterator(head_) || itr.node_ == nullptr) {
        return itr;
    }
#ifdef _VIEW__
    printf("erase: erase key ");
    std::cout << itr->first << std::endl;
#endif
    Node* delet = itr.node_;
    itr.node_ = nullptr;

    for (int i = 0; i < delet->nexts_.size(); ++i) {
        delet->pre_[i]->nexts_[i] = delet->nexts_[i];
        delet->nexts_[i]->pre_[i] = delet->pre_[i];
#ifdef _VIEW__
        printf("erase: erase flr %d\n", i);
#endif
    }

    Iterator ret(delet->nexts_[0]);
    delete delet;
    --size_;
    return ret;

}
template<class K, class V>
typename SkipList<K, V>::Iterator SkipList<K, V>::erase(const K& key) {
    Iterator itr = find(key);
    return erase(itr);
}
template<class K, class V>
void SkipList<K, V>::pop_front() {
    Iterator itr = begin();
    erase(itr);
}
template<class K, class V>
void SkipList<K, V>::pop_back() {
    Iterator itr = end();
    erase(--itr);
}
template<class K, class V>
typename SkipList<K, V>::Iterator SkipList<K, V>::begin()const {
    return Iterator(head_->nexts_[0]);
}
template<class K, class V>
typename SkipList<K, V>::Iterator SkipList<K, V>::end()const {
    return Iterator(end_);
}
template<class K, class V>
int SkipList<K, V>::max_h_()const {
    for (int i = MAX_H - 1; i >= 0; --i) {
        if (head_->nexts_[i] != end_) {
            return i;
        }
    }
    return 0;
}
