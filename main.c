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

int is_first_allocation = 1; // 1 = true | 0 = false
static struct Chunk *first_allocated_chunk;

int round_up_bytes(int bytes)
{
        if (bytes < 16) {
                return 16;
        }

        int r = 16;
        int result;
        for (int i = 1; result < bytes; i++) {
                result = r*i;
        }
        int diff = result - bytes;

        return bytes + diff;
}

void* x_malloc(int bytes)
{
        if (bytes % 16 != 0) {
                bytes = round_up_bytes(bytes);
        }

        if (is_first_allocation == 1) {
                first_allocated_chunk = sbrk(sizeof(struct Chunk) + bytes);
                first_allocated_chunk->size = bytes;
                first_allocated_chunk->free = 'n';
                first_allocated_chunk->next_chunk = first_allocated_chunk;
                first_allocated_chunk->prev_chunk = first_allocated_chunk;

                is_first_allocation = 0;

                return &first_allocated_chunk->usable_size;
        } else {
                struct Chunk *chunk = sbrk(sizeof(struct Chunk) + bytes);
                chunk->size = bytes;
                chunk->free = 'n';

                chunk->prev_chunk = first_allocated_chunk->prev_chunk;
                first_allocated_chunk->prev_chunk->next_chunk = chunk;
                chunk->next_chunk = first_allocated_chunk;
                first_allocated_chunk->prev_chunk = chunk;

                return &chunk->usable_size;
        }
}

void x_free(void *data_ptr)
{
        if (data_ptr == NULL) {
                return;
        }
        char *ptr = data_ptr;
        char *pptr = ptr - 20;
        *pptr = 'y';
}

void* search_free_chunk(int bytes)
{
        struct Chunk *chunk = first_allocated_chunk;
        do {
                if (chunk->size >= bytes) {
                        return &chunk->usable_size;
                }
                chunk = chunk->next_chunk;
        } while (chunk != first_allocated_chunk);

        return NULL;
}

int main()
{
        char *buffer = x_malloc(sizeof(char) * 16);
        x_free(buffer);

        return 0;
}
