#include <unistd.h>
#include <stdio.h>

// 29 bytes + padding = 32 bytes
struct Chunk {
        int size;
        char free;
        struct Chunk *next_chunk;
        struct Chunk *prev_chunk;
        void *usable_size;
};

int is_first_allocation = 0; // 1 = false | 0 = true
static struct Chunk *first_allocated_chunk;

void* my_malloc(int bytes)
{
        if (is_first_allocation == 0) {
                first_allocated_chunk = sbrk(sizeof(struct Chunk) + bytes);
                first_allocated_chunk->size = bytes;
                first_allocated_chunk->free = 'n';
                first_allocated_chunk->next_chunk = first_allocated_chunk;
                first_allocated_chunk->prev_chunk = first_allocated_chunk;

                is_first_allocation = 1;

                return &first_allocated_chunk->usable_size;
        } else {
                struct Chunk *chunk = sbrk(sizeof(struct Chunk) + bytes);
                chunk->size = bytes;
                chunk->free = 'n';

                first_allocated_chunk->prev_chunk->next_chunk = chunk;
                chunk->next_chunk = first_allocated_chunk;
                first_allocated_chunk->prev_chunk = chunk;

                return &chunk->usable_size;
        }
}

int main()
{
        char *buffer = my_malloc(sizeof(char) * 16);
        buffer[0] = 'h';
        printf("%c\n", buffer[0]);

        return 0;
}
