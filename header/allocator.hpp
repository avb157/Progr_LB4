#pragma once
#include <memory>
#include <cstddef>
#include <vector>

template<typename T, std::size_t BlockSize = 10>
class custom_allocator {
private:
    struct memory_block {
        char* data;
        std::size_t used;
        std::size_t size;
        std::vector<bool> allocated_flags;
        
        memory_block(std::size_t n) : data(new char[sizeof(T) * n]), used(0), size(n), allocated_flags(n, false) {}
        
        ~memory_block() {
            // Деструктор блока - освобождаем всю память
            delete[] data;
        }
        
        bool can_allocate(std::size_t n) const { 
            return used + n <= size; 
        }
        
        T* allocate(std::size_t n) {
            if (!can_allocate(n)) return nullptr;
            
            // Находим последовательные свободные ячейки
            for (std::size_t i = 0; i <= size - n; ++i) {
                bool can_use = true;
                for (std::size_t j = 0; j < n; ++j) {
                    if (allocated_flags[i + j]) {
                        can_use = false;
                        break;
                    }
                }
                
                if (can_use) {
                    for (std::size_t j = 0; j < n; ++j) {
                        allocated_flags[i + j] = true;
                    }
                    used += n;
                    return reinterpret_cast<T*>(data + i * sizeof(T));
                }
            }
            return nullptr;
        }
        
        void deallocate(T* ptr, std::size_t n) {
            std::size_t offset = (reinterpret_cast<char*>(ptr) - data) / sizeof(T);
            
            for (std::size_t i = 0; i < n; ++i) {
                if (offset + i < allocated_flags.size()) {
                    allocated_flags[offset + i] = false;
                }
            }
            used -= n;
        }
        
        bool belongs_to_block(T* ptr) const {
            char* ptr_char = reinterpret_cast<char*>(ptr);
            return ptr_char >= data && ptr_char < data + size * sizeof(T);
        }
    };
    
    std::vector<memory_block*> blocks;
    
    memory_block* find_block(T* ptr) {
        for (auto block : blocks) {
            if (block->belongs_to_block(ptr)) {
                return block;
            }
        }
        return nullptr;
    }
    
    void expand_memory(std::size_t n) {
        std::size_t new_size = (n > BlockSize) ? n : BlockSize;
        blocks.push_back(new memory_block(new_size));
    }
    
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    
    template<typename U>
    struct rebind {
        using other = custom_allocator<U, BlockSize>;
    };
    
    custom_allocator() {
        expand_memory(BlockSize);
    }
    
    custom_allocator(const custom_allocator& other) {
        // При копировании создаем новые блоки
        expand_memory(BlockSize);
    }
    
    template<typename U>
    custom_allocator(const custom_allocator<U, BlockSize>& other) {
        expand_memory(BlockSize);
    }
    
    ~custom_allocator() {
        for (auto block : blocks) {
            delete block;
        }
    }
    
    pointer allocate(size_type n) {
        if (n == 0) return nullptr;
        
        // Пытаемся выделить в существующих блоках
        for (auto block : blocks) {
            if (block->can_allocate(n)) {
                pointer result = block->allocate(n);
                if (result) {
                    return result;
                }
            }
        }
        
        // Если не хватило места - расширяем
        expand_memory(n);
        pointer result = blocks.back()->allocate(n);
        return result;
    }
    
    void deallocate(pointer p, size_type n) {
        if (p == nullptr || n == 0) return;
        
        memory_block* block = find_block(p);
        if (block) {
            block->deallocate(p, n);
        }
    }
    
    void deallocate_all() {
        for (auto block : blocks) {
            delete block;
        }
        blocks.clear();
        expand_memory(BlockSize);
    }
    
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        new(p) U(std::forward<Args>(args)...);
    }
    
    template<typename U>
    void destroy(U* p) {
        p->~U();
    }
    
    size_type max_size() const {
        size_type total = 0;
        for (auto block : blocks) {
            total += block->size;
        }
        return total;
    }
    
    // Методы для совместимости
    custom_allocator& operator=(const custom_allocator&) = delete;
    
    bool operator==(const custom_allocator& other) const {
        return this == &other;
    }
    
    bool operator!=(const custom_allocator& other) const {
        return !(*this == other);
    }
};
