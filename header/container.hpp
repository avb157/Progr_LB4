#pragma once
#include <iterator>
#include <memory>

template<typename T, typename Allocator = std::allocator<T>>
class custom_container {
private:
    struct node {
        T data;
        node* next;
        
        template<typename... Args>
        node(Args&&... args) : data(std::forward<Args>(args)...), next(nullptr) {}
    };
    
    using node_allocator = typename std::allocator_traits<Allocator>::template rebind_alloc<node>;
    
    node* head;
    node* tail;
    std::size_t container_size;
    node_allocator alloc;
    
public:
    class iterator {
    private:
        node* current;
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
        
        iterator(node* n = nullptr) : current(n) {}
        
        reference operator*() const { return current->data; }
        pointer operator->() const { return &current->data; }
        
        iterator& operator++() {
            if (current) current = current->next;
            return *this;
        }
        
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        bool operator==(const iterator& other) const { return current == other.current; }
        bool operator!=(const iterator& other) const { return current != other.current; }
    };
    
    class const_iterator {
    private:
        const node* current;
        
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = const T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;
        
        const_iterator(const node* n = nullptr) : current(n) {}
        
        reference operator*() const { return current->data; }
        pointer operator->() const { return &current->data; }
        
        const_iterator& operator++() {
            if (current) current = current->next;
            return *this;
        }
        
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        bool operator==(const const_iterator& other) const { return current == other.current; }
        bool operator!=(const const_iterator& other) const { return current != other.current; }
    };
    
    custom_container() : head(nullptr), tail(nullptr), container_size(0), alloc() {}
    
    explicit custom_container(const Allocator& a) : head(nullptr), tail(nullptr), container_size(0), alloc(a) {}
    
    ~custom_container() {
        clear();
    }
    
    // Конструктор копирования
    custom_container(const custom_container& other) : head(nullptr), tail(nullptr), container_size(0), alloc(other.alloc) {
        for (const auto& item : other) {
            push_back(item);
        }
    }
    
    // Оператор присваивания
    custom_container& operator=(const custom_container& other) {
        if (this != &other) {
            clear();
            for (const auto& item : other) {
                push_back(item);
            }
        }
        return *this;
    }
    
    void push_back(const T& value) {
        node* new_node = std::allocator_traits<node_allocator>::allocate(alloc, 1);
        std::allocator_traits<node_allocator>::construct(alloc, new_node, value);
        
        if (!head) {
            head = tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
        ++container_size;
    }
    
    void push_back(T&& value) {
        node* new_node = std::allocator_traits<node_allocator>::allocate(alloc, 1);
        std::allocator_traits<node_allocator>::construct(alloc, new_node, std::move(value));
        
        if (!head) {
            head = tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
        ++container_size;
    }
    
    template<typename... Args>
    void emplace_back(Args&&... args) {
        node* new_node = std::allocator_traits<node_allocator>::allocate(alloc, 1);
        std::allocator_traits<node_allocator>::construct(alloc, new_node, std::forward<Args>(args)...);
        
        if (!head) {
            head = tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
        ++container_size;
    }
    
    void pop_front() {
        if (!head) return;
        
        node* old_head = head;
        head = head->next;
        std::allocator_traits<node_allocator>::destroy(alloc, old_head);
        std::allocator_traits<node_allocator>::deallocate(alloc, old_head, 1);
        --container_size;
        
        if (!head) tail = nullptr;
    }
    
    void clear() {
        while (!empty()) {
            pop_front();
        }
    }
    
    iterator begin() { return iterator(head); }
    iterator end() { return iterator(nullptr); }
    const_iterator begin() const { return const_iterator(head); }
    const_iterator end() const { return const_iterator(nullptr); }
    const_iterator cbegin() const { return const_iterator(head); }
    const_iterator cend() const { return const_iterator(nullptr); }
    
    std::size_t size() const { return container_size; }
    bool empty() const { return container_size == 0; }
    
    Allocator get_allocator() const { return alloc; }
};
