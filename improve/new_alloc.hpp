#include <cstdlib>
#include <vector>
#include <cstddef>
#include <algorithm>
#include <cstdint>

#define SIZE 114688

/**
 * Get specified SIZE * n bytes from the memory.
 * @param n
 * @return the address of the block
 */
int* getNewBlock(int n) {
    if (n <= 0) return nullptr;
    size_t size = SIZE * n;
    void* ptr = malloc(size);
    return static_cast<int*>(ptr);
}

/**
 * Free specified SIZE * n bytes from the memory.
 * @param block the pointer to the block
 * @param n
 */
void freeBlock(const int* block, int n) {
    if (block == nullptr) return;
    free(const_cast<int*>(block));
}

// 辅助函数：对齐地址到指定边界
static inline void* align_ptr(void* ptr, size_t alignment) {
    uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
    uintptr_t aligned = (p + alignment - 1) & ~(alignment - 1);
    return reinterpret_cast<void*>(aligned);
}

class Allocator {
private:
    struct Block {
        char* base;               // 起始地址（字节）
        int block_cnt;            // 输入的 n，表示 SIZE * block_cnt 字节
        std::vector<bool> used;   // 每个字节一个标志，true表示已分配
        size_t used_cnt;          // 已分配的字节数

        Block(char* block, int n)
            : base(block), block_cnt(n), used(n * SIZE, false), used_cnt(0) {}

        // 禁止拷贝，允许移动
        Block(const Block&) = delete;
        Block& operator=(const Block&) = delete;
        Block(Block&&) = default;
        Block& operator=(Block&&) = default;
    };

    std::vector<Block> all_blocks;

public:
    Allocator() = default;

    ~Allocator() {
        for (auto& block : all_blocks) {
            freeBlock(reinterpret_cast<int*>(block.base), block.block_cnt);
        }
        all_blocks.clear();
    }

    /**
     * Allocate a memory block of size bytes.
     * @param size number of bytes to allocate
     * @return pointer to allocated memory, or nullptr if failed
     */
    void* allocate(size_t size) {
        if (size == 0) return nullptr;
        // 保证返回的指针按 8 字节对齐（足够常见类型）
        const size_t alignment = 8;
        size = (size + alignment - 1) & ~(alignment - 1);

        // 先尝试在现有 block 中分配
        void* ptr = try_allocate_wholly(size);
        if (ptr) return ptr;
        // 否则分配新 block
        return try_allocate_new(size);
    }

    /**
     * Deallocate the memory previously allocated by allocate.
     * @param pointer pointer to the memory block
     * @param size the size passed to allocate (must match)
     */
    void deallocate(void* pointer, size_t size) {
        if (pointer == nullptr || size == 0) return;
        Block* block = find_block(pointer);
        if (!block) return;
        uintptr_t start = reinterpret_cast<uintptr_t>(block->base);
        uintptr_t p = reinterpret_cast<uintptr_t>(pointer);
        size_t offset = p - start;
        for (size_t i = 0; i < size; ++i) {
            block->used[offset + i] = false;
        }
        block->used_cnt -= size;
        free_empty_blocks();
    }

private:
    // 在指定 block 中查找连续 size 字节的空闲空间
    std::pair<char*, size_t> find_continuous_space(Block& block, size_t size) {
        size_t cnt = 0;
        size_t pos = 0;
        for (size_t i = 0; i < block.used.size(); ++i) {
            if (block.used[i]) {
                cnt = 0;
                pos = i + 1;
            } else {
                cnt++;
                if (cnt == size) {
                    return {block.base + pos, pos};
                }
            }
        }
        return {nullptr, 0};
    }

    void* try_allocate_in_block(Block& block, size_t size) {
        auto [ptr, offset] = find_continuous_space(block, size);
        if (ptr) {
            for (size_t i = 0; i < size; ++i) {
                block.used[offset + i] = true;
            }
            block.used_cnt += size;
            return ptr;
        }
        return nullptr;
    }

    void* try_allocate_wholly(size_t size) {
        for (auto& block : all_blocks) {
            if (block.used.size() - block.used_cnt >= size) {
                void* ptr = try_allocate_in_block(block, size);
                if (ptr) return ptr;
            }
        }
        return nullptr;
    }

    void* try_allocate_new(size_t size) {
        int n = (size + SIZE - 1) / SIZE;          // 需要多少个 SIZE 字节块
        int* new_block = getNewBlock(n);
        if (!new_block) return nullptr;
        char* base = reinterpret_cast<char*>(new_block);
        all_blocks.emplace_back(base, n);
        return try_allocate_in_block(all_blocks.back(), size);
    }

    Block* find_block(void* ptr) {
        for (auto& block : all_blocks) {
            char* start = block.base;
            char* end = start + block.used.size();
            if (ptr >= start && ptr < end) {
                return &block;
            }
        }
        return nullptr;
    }

    void free_empty_blocks() {
        auto it = all_blocks.begin();
        while (it != all_blocks.end()) {
            if (it->used_cnt == 0) {
                freeBlock(reinterpret_cast<int*>(it->base), it->block_cnt);
                it = all_blocks.erase(it);
            } else {
                ++it;
            }
        }
    }
};