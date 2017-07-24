#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include <cmath>

using namespace std;

template <typename Key, size_t N = 32>
class ADS_set {
    
public:
    class Iterator;
    using value_type = Key;
    using key_type = Key;
    using reference = key_type&;
    using const_reference = const key_type&;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = Iterator;
    using const_iterator = Iterator;
    using key_compare = std::less<key_type>;
    
    class Node {
        
    public:
        value_type* keys;
        Node** children;
        Node* parent;
        Node* next;
        bool leaf;
        unsigned keys_counter;
        unsigned children_counter;
    public:
        Node();
        ~Node();
        int add(const_reference);
        void set_parent(Node*);
        void set_next(Node*);
        void set_leaf(bool);
        
        /// dump
        void keys_printer(ostream&);
        inline friend std::ostream& operator<<(std::ostream& os, const Node& node) {
            node.keys_printer(os);
            if(node.children_counter > 0) {
                for (size_t i = 0; i < node.children_counter; ++i) {
                    os << *node.children[i];
                }
            }
            return os;
        }
    };
    
private:
    Node* root;
    int depth;
    unsigned element_counter;
private:
    void root_split();
    void internal_split(Node*);
    void external_split(Node*);

    pair<Iterator,bool> insert_private_external(const_reference);
    int insert_private_internal(Node *current, const_reference key);
    
    bool has_max_num_of_keys(Node *);
    bool is_root(Node* root);
    
    void shift_left(size_t start, size_t end, Node* current);
    void shift_right(size_t start, size_t end, Node* current);
    
    bool equal(const key_type& key, const key_type& to);
    
    size_t index_from_parent(Node* current);
    
    bool steal_from_right(Node *current, size_t index, std::pair<Node*, size_t>);
    bool steal_from_left(Node *current, size_t index, std::pair<Node*, size_t>);
    bool merge_with_left(Node *current, size_t index, const_reference key, std::pair<Node*, size_t>);
    bool merge_with_right(Node *current, size_t index, const_reference key, std::pair<Node*, size_t>);
    void merge_root();
    
    void delete_element(Node *current, size_t index);
    
    Node* find_leaf(Node*, const_reference &) const;
    
    pair<int,bool> binary_search_in_node(Node *, int, int, const_reference) const;
    
    Node* find_leaf_with_twin(Node* current, const_reference &key,pair<Node*,int>& twin);
    
public:
    void printTree();
    ADS_set();
    ADS_set(std::initializer_list<key_type> ilist);
    template<typename InputIt> ADS_set(InputIt first, InputIt last);
    ADS_set(const ADS_set& other);
    ~ADS_set();
    
    ADS_set& operator=(const ADS_set& other);
    ADS_set& operator=(std::initializer_list<key_type> ilist);
    
    size_type size() const;
    bool empty() const;
    
    size_type count(const key_type& key) const;
    iterator find(const key_type& key) const;
    
    void clear();
    void swap(ADS_set& other);
    
    void insert(std::initializer_list<key_type> ilist);
    std::pair<iterator,bool> insert(const key_type& key);
    template<typename InputIt> void insert(InputIt first, InputIt last);
    
    size_type erase(const key_type& key);
    
    const_iterator begin() const;
    const_iterator end() const;
    
    void dump(std::ostream& o = std::cerr) const;
    
    friend bool operator==(const ADS_set& lhs, const ADS_set& rhs) {
        if (lhs.element_counter != rhs.element_counter) {
            return false;
        }
        for (const auto& k : lhs) {
            if (!rhs.count(k)) {
                return false;
            }
        }
        return true;
    }
    friend bool operator!=(const ADS_set& lhs, const ADS_set& rhs) {
        return !(lhs == rhs);
    }
    
};

template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
private:
    Node* current;
    ADS_set<Key,N> *tree;
    size_t index;
    
public:
    using value_type = Key;
    using difference_type = std::ptrdiff_t;
    using reference = const value_type&;
    using pointer = const value_type*;
    using iterator_category = std::forward_iterator_tag;
    
    explicit Iterator(Node* _current, size_type _index) : current(_current), index(_index) {}
    reference operator*() const {
        return current->keys[index];
    }
    pointer operator->() const {
        
        return &(current->keys[index]);
    }
    Iterator& operator++() {
        if (index + 1 < current->keys_counter || current->next == nullptr) {
            ++index;
        } else {
            current = current->next;
            index = 0;
        }
        
        return *this;
        
    }
    Iterator operator++(int) {
        Iterator it = *this;
        
        if (index + 1 < current->keys_counter || current->next == nullptr) {
            ++index;
        } else {
            current = current->next;
            index = 0;
        }
        
        return it;
    }
    
    friend bool operator==(const Iterator& lhs, const Iterator& rhs) {
        if (lhs.current == rhs.current) {
            if (lhs.index == rhs.index) {
                return true;
            }
        }
        return false;
    }
    friend bool operator!=(const Iterator& lhs, const Iterator& rhs) {
        return !(lhs==rhs);
    }
};

template <typename Key, size_t N> void swap(ADS_set<Key,N>& lhs, ADS_set<Key,N>& rhs) { lhs.swap(rhs); }

// #pragma mark - implemantation

//#pragma Public ADS_set methods

template <typename Key, size_t N>
ADS_set<Key,N>::ADS_set() {
    root = new Node();
    element_counter = 0;
    depth = 0;
    root->leaf = 1;
}
template <typename Key, size_t N>
ADS_set<Key,N>::ADS_set(std::initializer_list<key_type> ilist): ADS_set{} {
    insert(ilist);
}
template <typename Key, size_t N>
template<typename InputIt> ADS_set<Key,N>::ADS_set(InputIt first, InputIt last): ADS_set() {
    
    insert(first,last);
}
template <typename Key, size_t N>
ADS_set<Key,N>::ADS_set(const ADS_set& other) {
    root = new Node();
    depth = 0;
    insert(other.begin(), other.end());
    element_counter = other.element_counter;
}
template <typename Key, size_t N>
ADS_set<Key,N>::~ADS_set(){
    if (root) {
        delete root;
    }
}

template <typename Key, size_t N>
ADS_set<Key,N>& ADS_set<Key,N>::operator=(const ADS_set& other) {
    if (this == &other) {return *this;}
    clear();
    
    insert(other.begin(), other.end());
    element_counter = other.element_counter;
    return *this;
}
template <typename Key, size_t N>
ADS_set<Key,N>& ADS_set<Key,N>::operator=(std::initializer_list<key_type> ilist) {
    clear();
    insert(ilist);
    return *this;
    
}

template <typename Key, size_t N>
typename ADS_set<Key,N>::size_type ADS_set<Key,N>::size() const {
    return element_counter;
}
template <typename Key, size_t N>
bool ADS_set<Key,N>::empty() const {
    return element_counter == 0;
}

template <typename Key, size_t N>
size_t ADS_set<Key,N>::count(const_reference key) const {
    
    Node* current = find_leaf(root, key);
    
    auto pair = binary_search_in_node(current, 0, current->keys_counter-1, key);
    
    if (pair.second) {
        return true;
    }
    return false;
}

template <typename Key, size_t N>
typename ADS_set<Key,N>::iterator ADS_set<Key,N>::find(const key_type& key) const {
    
    Node *current = find_leaf(root, key);
    
    auto pair = binary_search_in_node(current, 0, current->keys_counter-1, key);
    
    if (pair.second) {
        return Iterator(current, pair.first);
    }
    return end();
}

template <typename Key, size_t N>
void ADS_set<Key,N>::clear() {
    delete root;
    root = new Node();
    element_counter = 0;
    depth = 0;
}
template <typename Key, size_t N>
void ADS_set<Key,N>::swap(ADS_set<Key, N> &other) {
    using std::swap;
    swap(root,other.root);
    swap(element_counter,other.element_counter);
    swap(depth,other.depth);
    
}

template <typename Key, size_t N>
void ADS_set<Key,N>::insert(std::initializer_list<key_type> ilist) {
    for(auto key: ilist) {
        insert_private_external(key);
    }
}
template <typename Key, size_t N>
std::pair<typename ADS_set<Key,N>::iterator,bool> ADS_set<Key,N>::insert(const key_type& key) {
    
    return insert_private_external(key);
}
template <typename Key, size_t N>
template<typename InputIt> void ADS_set<Key,N>::insert(InputIt first, InputIt last) {
    if (first == last) {
        return;
    }
    for_each(first, last, [&] (const_reference key){
        
        insert_private_external(key);
        
    });
}

template <typename Key, size_t N>
size_t ADS_set<Key,N>::erase(const key_type& key) {

    pair<Node*,int> twin;
    
    if (!element_counter) { return 0;}
    Node *current = find_leaf_with_twin(root, key, twin);
    auto pair = binary_search_in_node(current, 0, current->keys_counter, key);
    
    if (!pair.second) {return 0;}
    
    if (current->keys_counter-1>=N || root->leaf == true) {
        delete_element(current, pair.first);
        if (twin.first) {
            twin.first->keys[twin.second] = current->keys[0];
        }
        return 1;
        
        // happy option
    } else {
        
        size_t index = index_from_parent(current);
        delete_element(current,pair.first);
        
        if (index == 0) {
    
            if (!steal_from_right(current, index, make_pair(twin.first, twin.second))) {
                merge_with_right(current, index, key, make_pair(twin.first, twin.second));
                return 1;
            }
            
        } else if (index == current->parent->children_counter-1) {
            
            if (!steal_from_left(current, index, make_pair(twin.first, twin.second))) {
                merge_with_left(current, index, key, make_pair(twin.first, twin.second));
                return 1;
            }
            
        } else {
            
            if (!steal_from_left(current, index, make_pair(twin.first, twin.second))) {
                if (!steal_from_right(current, index, make_pair(twin.first, twin.second))) {
                    merge_with_left(current, index, key, make_pair(twin.first, twin.second));
                    return 1;
                }
            }
        }
        return true;
    }
    return 0;
}


template <typename Key, size_t N>
typename ADS_set<Key,N>::const_iterator ADS_set<Key,N>::begin() const {
    Node *current = root;
    while (!current->leaf) {
        current = current->children[0];
    }
    return Iterator(current,0);
}


template <typename Key, size_t N>
typename ADS_set<Key,N>::const_iterator ADS_set<Key,N>::end() const {
    Node *current = root;
    while (!current->leaf) {
        current = current->children[current->children_counter-1];
    }
    return Iterator(current, current->keys_counter);
}

template <typename Key, size_t N>
void ADS_set<Key,N>::dump(std::ostream& o) const {
    Node *current = root;
    while (current->leaf != 1) {
        current = current->children[0];
    }
    
    while(current->next != nullptr) {
        current->keys_printer(o);
        current = current->next;
    }
    current->keys_printer(o);
}

// #pragma mark - Private ADS_set methods

template <typename Key, size_t N>
void ADS_set<Key,N>::root_split() {
    depth += 1;
    
    value_type middle = root->keys[N];
    bool root_was_leaf = root->leaf;
    
    Node* left = root;
    Node *new_root = new Node();
    Node *right = new Node();
    
    root = new_root;
    
    right->set_parent(root);
    left->set_parent(root);
    
    root->children[root->children_counter++] = left;
    root->children[root->children_counter++] = right;
    
    root->add(middle);
    
    // move keys to the right
    for (size_t i = N+(!root_was_leaf); i < left->keys_counter; ++i) {
        right->add(left->keys[i]);
    }
    
    if (!root_was_leaf) {
        for (size_t i = N+1; i < left->children_counter; ++i) {
            left->children[i]->set_parent(right);
            right->children[right->children_counter++] = left->children[i];
            left->children[i] = nullptr;
        }
        left->children_counter -= N+1;
    }
    
    left->keys_counter -= N+1;
    
    root->set_leaf(false);
    root->set_next(nullptr);
    // if root was leaf, then making children leaf and setting for left child's "next" pointer to the right child
    if (root_was_leaf) {
        left->set_leaf(true);
        left->set_next(right);
        right->set_next(nullptr);
    } else {
        // else right is no leaf
        right->set_leaf(false);
    }
    
}
template <typename Key, size_t N>
void ADS_set<Key,N>::internal_split(Node* left) {
    value_type middle = left->keys[N];
    size_t counter = 0;
    Node *parent = left->parent;
    Node *right = new Node();
    
    right->set_parent(parent);
    right->set_leaf(false);
    parent->add(middle);
    
    for (size_t i = 0; i < parent->children_counter; ++i) {
        if (left == parent->children[i]) {
            counter = i;
            break;
        }
    }
    // shifting the parents children to the right
    if (counter < parent->children_counter-1) {
        for (size_t i = parent->children_counter-1; i > counter; --i) {
            parent->children[i+1] = parent->children[i];
        }
        parent->children[counter+1] = right;
        parent->children_counter+=1;
    } else {
        parent->children[parent->children_counter++] = right;
    }
    
    // move keys to the right
    for (size_t i = N+1; i < left->keys_counter; ++i) {
        right->add(left->keys[i]);
    }
    left->keys_counter -= N+1;
    // move pointers to the right
    for (size_t i = N+1; i < left->children_counter; ++i) {
        left->children[i]->set_parent(right);
        right->children[right->children_counter++] = left->children[i];
        left->children[i] = nullptr;
    }
    left->children_counter = N+1;
    
    if (has_max_num_of_keys(parent)) {
        if (is_root(parent)) {
            root_split();
        } else {
            internal_split(parent);
        }
    }
}
template <typename Key, size_t N>
void ADS_set<Key,N>::external_split(Node* left) {
    value_type middle = left->keys[N];
    Node *parent = left->parent;
    Node *right = new Node();
    right->set_parent(parent);
    
    // counter for who has raised split
    int counter = 0;
    
    // assignment of next
    if (left->next!=nullptr) {
        right->set_next(left->next);
    }
    left->set_next(right);
    parent->add(middle);
    // find needed index for shifting nodes
    for (int i = 0; i < parent->children_counter; ++i) {
        if (left == parent->children[i]) {
            counter = i;
            break;
        }
    }
    
    // shift parent childrens
    if (counter < parent->children_counter-1) {
        for (int i = parent->children_counter-1; i >= counter; --i) {
            parent->children[i+1] = parent->children[i];
        }
        
        //setting next pointer
        parent->children[counter]->set_next(right);
        right->set_next(parent->children[counter+2]);
        
        parent->children[counter+1] = right;
        parent->children_counter+=1;
        
    } else {
        parent->children[parent->children_counter-1]->set_next(right);
        parent->children[parent->children_counter++] = right;
    }
    
    // move the keys to the right
    for (size_t i = N; i < left->keys_counter; ++i) {
        right->add(left->keys[i]);
    }
    left->keys_counter -= N+1;
    
    //check split for parent (internal split)
    if (left->parent->keys_counter == 2*N + 1) {
        if (left->parent->parent == nullptr) {
            root_split();
        }
        else {
            internal_split(left->parent);
        }
    }
}


template <typename Key, size_t N>
bool ADS_set<Key,N>::has_max_num_of_keys(Node* current) {
    return current->keys_counter == 2*N+1;
}
template <typename Key, size_t N>
bool ADS_set<Key,N>::is_root(Node* current) {
    return current == root;
}

template <typename Key, size_t N>
void  ADS_set<Key,N>::shift_left(size_t start, size_t end, Node* current) {
    for (size_t i = start; i < end; ++i) {
        current->keys[i] = current->keys[i+1];
    }
}
template <typename Key, size_t N>
void  ADS_set<Key,N>::shift_right(size_t start, size_t end, Node* current) {
    for (size_t i = end; i > start; --i) {
        current->keys[i] = current->keys[i-1];
    }
}

template <typename Key, size_t N>
bool  ADS_set<Key,N>::equal(const key_type& key, const key_type& to) {
    if (!key_compare()(key,to) && !key_compare()(to,key)) {
        return true;
    }
    return false;
}

template <typename Key, size_t N>
size_t ADS_set<Key,N>::index_from_parent(Node* current) {
    if (current == root) {
        throw runtime_error("root! index_from_parent");
    } else {
        Node *parent = current->parent;
        
        for (size_t i = 0; i < parent->children_counter; ++i) {
            if (parent->children[i] == current) {
                return i;
            }
        }
    }
    throw runtime_error("no current in childrens from parent! index_from_parent");
}

template <typename Key, size_t N>
bool ADS_set<Key,N>::steal_from_right(Node *current, size_t index, std::pair<Node*, size_t> twin) {
    Node *right = current->parent->children[index+1];
    
    // check if we just merge or we make marge and then split
    size_t common_size = current->keys_counter + right->keys_counter;
    if (common_size < 2*N) { return false; }
    
    if (current->leaf) {
        
        // move first right key
        current->keys[current->keys_counter++] = right->keys[0];
        
        // move all keys to the left
        shift_left(0, right->keys_counter-1, right);
        right->keys_counter -= 1;
        
        // change first key in parent from first in right
        current->parent->keys[index_from_parent(right)-1] = right->keys[0];
        
        if (twin.first) {
            twin.first->keys[twin.second] = current->keys[0];
        }
        
        return true;
        
    } else { // internal node
        Node* parent = current->parent;
        
        // put parent's key in the left
        current->keys[current->keys_counter++] = parent->keys[index_from_parent(current)];
        
        // moving childrens from the right to the left
        current->children[current->children_counter++] = right->children[0];
        
        right->children[0]->parent = current;
        
        // moving pointers to the left
        for (size_t i = 0; i < right->children_counter; ++i) {
            right->children[i] = right->children[i+1];
        }
        right->children_counter -= 1;
        
        // moving key from the right to the parent and making shift in the left
        parent->keys[index_from_parent(right)-1] = right->keys[0];
        shift_left(0, right->keys_counter-1, right);
        right->keys_counter -= 1;
        
        return true;
        
    }
    
    return false;
}
template <typename Key, size_t N>
bool ADS_set<Key,N>::steal_from_left(Node *current, size_t index, std::pair<Node*, size_t> twin) {
    Node *left = current->parent->children[index-1];
    
    // check if we just merge or we make marge and then split
    size_t common_size = current->keys_counter + left->keys_counter;
    if (common_size < 2*N) {return false;}
    
    // make place for new element
    shift_right(0, current->keys_counter, current);
    current->keys_counter += 1;
    
    if (current->leaf) {
        
        // move elements from left to the right
        current->keys[0] = left->keys[left->keys_counter-1];
        left->keys_counter -= 1;
        
        // change first key in parent from first in right
        current->parent->keys[index_from_parent(current)-1] = current->keys[0];
        
        if (twin.first) {
            twin.first->keys[twin.second] = current->keys[0];
        }
        
        return true;
        
    } else {
        Node *parent = current->parent;
        
        current->keys[0] = parent->keys[index-1];
        
        parent->keys[index-1] = left->keys[left->keys_counter-1];
        left->keys_counter -= 1;
        
        // move all pointers to the right on 1 step
        for (int i = current->children_counter-1; i >= 0; --i) {
            current->children[i+1] = current->children[i];
        }
        current->children_counter += 1;
        
        // move the children from left to the right
        current->children[0] = left->children[left->children_counter-1];
        left->children[left->children_counter-1]->parent = current;
        
        left->children_counter -= 1;
        
        return true;
    }
    
    return false;
}
template <typename Key, size_t N>
bool ADS_set<Key,N>::merge_with_left(Node *current, size_t index, const_reference key, std::pair<Node*, size_t> twin) {
    Node *left = current->parent->children[index-1];
    Node *parent = current->parent;
    
    if (parent == root && parent->keys_counter == 1) {
        merge_root();
        return true;
    }
    
    if (current->leaf) {
        
        // moving keys from the left to the right
        for (size_t i = 0; i < current->keys_counter; ++i) {
            left->keys[left->keys_counter++] = current->keys[i];
        }
        current->keys_counter = 0;
        
        // setting new next for the left, becouse right merging with him
        left->next = current->next;
        
        if (twin.first) {
            key_type& twin_key = twin.first->keys[twin.second];
            
            if (!key_compare()(key,twin_key) && !key_compare()(twin_key,key)) {
                
                twin.first->keys[twin.second] = current->keys[0];
            }
        }
        
        // shifting parent's keys to the left
        shift_left(index-1, parent->keys_counter, parent);
        parent->keys_counter -= 1;
        
        delete parent->children[index];
        
        // shifting parent's childrens to the left
        for (size_t i = index; i < parent->children_counter; ++i) {
            parent->children[i] = parent->children[i+1];
        }
        parent->children_counter -= 1;
        
    } else { // internal node
        
        // get down parent's key
        left->keys[left->keys_counter++] = parent->keys[index-1];
        
        // shifting parent's keys
        shift_left(index-1, parent->keys_counter-1, parent);
        parent->keys_counter -= 1;
        
        // moving keys from the right to the left
        for (size_t i = 0; i < current->keys_counter; ++i) {
            left->keys[left->keys_counter++] = current->keys[i];
        }
        current->keys_counter = 0;
        
        // moving kids from the right to the left
        for (size_t i = 0; i < current->children_counter; ++i) {
            left->children[left->children_counter++] = current->children[i];
            current->children[i]->parent = left;
        }
        
        // moving pointers
        for (size_t i = index; i < parent->children_counter; ++i) {
            parent->children[i] = parent->children[i+1];
        }
        parent->children_counter -= 1;
        
        for (size_t i = 0; i < 2*N + 1; ++i) {
            current->children[i] = nullptr;
        }
        current->children_counter = 0;
        
        delete current;
    }
    
    if (parent->keys_counter < N && parent != root) {
        
        size_t index_parent = index_from_parent(parent);
        size_t max_parent_index = parent->parent->children_counter-1;
        
        if (index_parent == 0) {
            if (!steal_from_right(parent, index_parent, twin)) {
                merge_with_right(parent, index_parent, key, twin);
            }
        } else if (index_parent == max_parent_index){
            if (!steal_from_left(parent, index_parent, twin)) {
                merge_with_left(parent, index_parent, key, twin);
            }
        } else {
            if (!steal_from_left(parent, index_parent, twin)) {
                if (!steal_from_right(parent, index_parent, twin)) {
                    merge_with_left(parent, index_parent, key, twin);
                }
            }
        }
        return true;
    }
    
    return false;
}
template <typename Key, size_t N>
bool ADS_set<Key,N>::merge_with_right(Node *current, size_t index, const_reference key, std::pair<Node*, size_t> twin) {
    Node *right = current->parent->children[index+1];
    Node *parent = current->parent;
    
    
    if (parent == root && parent->keys_counter == 1) {
        merge_root();
        return true;
    }
    
    if (current->leaf) {
        
        // move elements from the right to the left
        for (size_t i = 0; i < right->keys_counter; ++i) {
            current->keys[current->keys_counter++] = right->keys[i];
        }
        right->keys_counter = 0;
        
        // set new next
        if (current->leaf) {
            current->next = right->next;
        }
        
        // shift keys in parents
        shift_left(index, parent->keys_counter-1, parent);
        parent->keys_counter -= 1;
        
        // shift pointers in parent to the left
        for (size_t i = index_from_parent(right); i < parent->children_counter; ++i) {
            parent->children[i] = parent->children[i+1];
        }
        parent->children_counter -= 1;
        
        if (twin.first) {
            twin.first->keys[twin.second] = current->keys[0];
        }
        twin.first = nullptr;
        
        delete right;
        
    } else {
        
        // making empty space in the right for left's and parent's keys
        size_t common = current->keys_counter + 1;
        for (int i = right->keys_counter-1; i >= 0; --i) {
            right->keys[i+common] = right->keys[i];
        }
        right->keys_counter += common;
        
        // making empty space in the right for left's childrens
        for (int i = right->children_counter-1; i >= 0; --i) {
            
            right->children[i+common] = right->children[i];
        }
        right->children_counter += common;
        
        // moving keys from the left to the rigth
        for (size_t i = 0; i < current->keys_counter; ++i) {
            right->keys[i] = current->keys[i];
        }
        
        // getting there first key of parent
        right->keys[current->keys_counter] = parent->keys[0];
        
        // shifting parent's key to the left
        shift_left(0, parent->keys_counter-1, parent);
        parent->keys_counter -= 1;
        current->keys_counter = 0;
        
        // shifting parent's childrens to the left
        for (size_t i = 0; i < parent->children_counter; ++i) {
            parent->children[i] = parent->children[i+1];
        }
        parent->children_counter -= 1;
        
        // now moving pointers from the left to the right and nullptr pointers in the left
        for (size_t i = 0; i < current->children_counter; ++i) {
            right->children[i] = current->children[i];
            current->children[i]->parent = right;
            
            current->children[i] = nullptr;
        }
        
        delete current;
    }
    
    
    if (parent->keys_counter < N && parent != root) {
        
        size_t index_parent = index_from_parent(parent);
        size_t max_parent_index = parent->parent->children_counter-1;
        
        if (index_parent == 0) {
            if (!steal_from_right(parent, index_parent, twin)) {
                merge_with_right(parent, index_parent, key, twin);
            }
        } else if (index_parent == max_parent_index){
            if (!steal_from_left(parent, index_parent, twin)) {
                merge_with_left(parent, index_parent, key, twin);
            }
        } else {
            if (!steal_from_left(parent, index_parent, twin)) {
                if (!steal_from_right(parent, index_parent, twin)) {
                    merge_with_left(parent, index_parent, key, twin);
                }
            }
        }
        return true;
    }
    
    return false;
    
}
template <typename Key, size_t N>
void ADS_set<Key,N>::merge_root() {
    
    Node *parent = root;
    Node *left = parent->children[0];
    Node *right = parent->children[1];
    
    if (right->leaf) {
        parent->keys_counter -=1 ;
    }
    
    // makeing empty space for left's keys
    for (int i = parent->keys_counter; i >= 0; --i) {
        parent->keys[i+left->keys_counter] = parent->keys[i];
    }
    parent->keys_counter += left->keys_counter;
    
    // moving keys from the left to the up
    for (size_t i = 0; i < left->keys_counter; ++i) {
        parent->keys[i] = left->keys[i];
    }
    // moving keys from the right to the up
    for (size_t i = 0; i < right->keys_counter; ++i) {
        parent->keys[parent->keys_counter++] = right->keys[i];
    }
    
    // nullptr for old childrens
    for (size_t i = 0; i < 2*N+1; ++i) {
        parent->children[i] = nullptr;
    }
    parent->children_counter = 0;
    
    if (!left->leaf) {
        
        // moving childrens from the left
        for (size_t i = 0; i < left->children_counter; ++i) {
            
            parent->children[parent->children_counter++] = left->children[i];
            left->children[i]->parent = parent;
        }
        
        // moving childrens from the right
        for (size_t i = 0; i < right->children_counter; ++i) {
            
            parent->children[parent->children_counter++] = right->children[i];
            right->children[i]->parent = parent;
        }
        
        for (size_t i = 0; i < 2*N+1; ++i) {
            left->children[i] = nullptr;
            right->children[i] = nullptr;
        }
    }
    
    delete left;
    delete right;
    
    if (root->children_counter == 0) {
        root->leaf = true;
    }
    
    depth-=1;
}

template <typename Key, size_t N>
void ADS_set<Key,N>::delete_element(Node *current, size_t index) {
    shift_left(index, current->keys_counter-1, current);
    current->keys_counter-=1;
    element_counter -= 1;
    
//    for (size_t i = 0; i < current->keys_counter; ++i) {
//        if (equal(key, current->keys[i])) {
//            shift_left(i, current->keys_counter-1, current);
//            current->keys_counter-=1;
//            element_counter -= 1;
//        }
//    }
}

template <typename Key, size_t N>
void ADS_set<Key,N>::printTree() {
    size_type current_depth = 0;
    size_type nol = pow((2 * N + 1), depth); //num of leafs
    
    Node** current = new Node*[nol];
    
    current[0] = root;
    size_type current_count = 1;
    
    while (current_depth <= depth) {
        //printing out current level of leafs
        cout << "depth level: " << current_depth << "\n";
        for (size_t i = 0; i < current_count; ++i) {
            current[i]->keys_printer(cout);
        }
        cout << "\n";
        
        //defining new level of leafs
        Node** temp = new Node*[nol];
        int t = 0;
        for (size_t i = 0; i < current_count; ++i) {
            for (size_t l = 0; l < current[i]->children_counter; ++l) {
                temp[t] = current[i]->children[l];
                ++t;
            }
        }
        
        //copying leafs back to current
        for (size_t i = 0; i < t; ++i) {
            current[i] = temp[i];
        }
        
        delete[] temp;
        
        current_count = t;
        ++current_depth;
    }
}

//#pragma mark - Node methods

template <typename Key, size_t N>
ADS_set<Key,N>::Node::Node() {
    keys = new value_type[(2*N)+1];
    children = new Node*[(2*N)+2];
    parent = nullptr;
    next = nullptr;
    leaf = 1;
    keys_counter = 0;
    children_counter = 0;
}

template <typename Key, size_t N>
ADS_set<Key,N>::Node::~Node() {
    delete[] keys;
    for (size_t i = 0; i < children_counter; ++i) {
        delete children[i];
    }
    delete[] children;
}

template <typename Key, size_t N>
int ADS_set<Key,N>::Node::add(const_reference key) {
    if (keys_counter == 0) {
        keys[keys_counter++] = key;
        return keys_counter-1;
    }
    
    if (key_compare()(key, keys[0])) {
        for (size_t i = keys_counter; i > 0; --i) {
            keys[i] = keys[i-1];
        }
        keys[0] = key;
        ++keys_counter;
        return 0;
    } else if (key_compare()(keys[keys_counter-1], key)) {
        keys[keys_counter++] = key;
        return keys_counter-1;
    } else {
        //   binary search
        
        unsigned start = 0;
        unsigned end = keys_counter - 1;
        unsigned middle = start + ((end - start) / 2);
        
        while(!((key_compare()(key, keys[middle + 1])) && (key_compare()(keys[middle], key)))) {
            if(key_compare()(key, keys[middle])) {
                end = middle;
            } else {
                start = middle;
            }
            middle = start + ((end - start) / 2);
        }
        
        for (size_t i = keys_counter; i > middle; --i) {
            keys[i] = keys[i-1];
        }
        keys[middle + 1] = key;
        ++keys_counter;
        return middle+1;
    }
}

template <typename Key, size_t N>
void ADS_set<Key,N>::Node::set_parent(Node* _parent) {
    parent = _parent;
}
template <typename Key, size_t N>
void ADS_set<Key,N>::Node::set_next(Node* _next) {
    next = _next;
}
template <typename Key, size_t N>
void ADS_set<Key,N>::Node::set_leaf(bool _leaf) {
    leaf = _leaf;
}

template <typename Key, size_t N>
void ADS_set<Key,N>::Node::keys_printer(ostream& o) {
    if (keys_counter!=0) {
        o << "[";
        for (size_t i = 0; i < keys_counter-1; ++i) {
            o << keys[i] << ",";
        }
        o << keys[keys_counter-1] << "]" << std::endl;
    }
}

template <typename Key, size_t N>
pair<typename ADS_set<Key,N>::Iterator,bool> ADS_set<Key,N>::insert_private_external(const_reference key) {
    Node *current = find_leaf(root, key);

    pair<int,bool> pair = binary_search_in_node(current,0,((int)current->keys_counter)-1, key);

    if (!pair.second) {
        int x = insert_private_internal(current, key);
        return make_pair(Iterator(current, x), !pair.second);
    }
    return make_pair(Iterator(current, pair.first), !pair.second);
}

template <typename Key, size_t N>
int ADS_set<Key,N>::insert_private_internal(Node *current, const_reference key) {
    int counter = current->add(key);
    ++element_counter;

    if (has_max_num_of_keys(current)) {
        if (is_root(current)) {
            root_split();
        } else {
            external_split(current);
        }
    }
    return counter;
}

template <typename Key, size_t N>
pair<int,bool> ADS_set<Key,N>::binary_search_in_node(Node *current, int start, int end, const_reference key) const{
    
    if (start <= end) {
        int middle = (start + end) / 2;  // compute mid point.
        if (middle > current->keys_counter-1) {
            return make_pair(-1, false);
        }

        if (key_compare()(key,current->keys[middle])) {
            return binary_search_in_node(current, start, middle-1, key);
        } else if (key_compare()(current->keys[middle],key)) {
            return binary_search_in_node(current, middle+1, end, key);
        } else {
            return make_pair(middle, true);
        }
    }
    return make_pair(-1, false);
}

template <typename Key, size_t N>
typename ADS_set<Key,N>::Node* ADS_set<Key,N>::find_leaf_with_twin(Node* current, const_reference &key, pair<Node*,int>& twin) {
    
    if (!current->leaf) {
        for (size_t i = 0; i < current->keys_counter; ++i) {
            if (key_compare{}(key,current->keys[i])) {
                if (equal(key, current->keys[i])) {
                    twin.first = current;
                    twin.second = (int)i-1;
                }
                return find_leaf_with_twin(current->children[i], key, twin);
            }
        }
        if (equal(key, current->keys[current->children_counter-2])) {
            twin.first = current;
            twin.second = current->children_counter-2;
        }
        return find_leaf_with_twin(current->children[current->children_counter-1], key, twin);
    }
    return current;
}

template <typename Key, size_t N>
typename ADS_set<Key,N>::Node* ADS_set<Key,N>::find_leaf(Node* current, const_reference &key) const {
    if (!current->leaf) {
        // look for right path
        for (size_t i = 0; i < current->keys_counter; ++i) {
            if (key_compare{}(key,current->keys[i])) {
                return find_leaf(current->children[i], key);
            }
        }
        return find_leaf(current->children[current->children_counter-1], key);
    }
    return current;
    
}

#endif // ADS_SET_H
//
