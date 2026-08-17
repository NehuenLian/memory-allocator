#include <unistd.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0

// 24/32 bytes
struct Chunk {
        size_t size;
        int free;
        struct Chunk *next_chunk;
        struct Chunk *prev_chunk;
        void *usable_size;
};

int is_first_allocation = TRUE;
static struct Chunk *first_allocated_chunk;

int round_up_bytes(int bytes_requested)
{
        if (bytes_requested < 16) {
                return 16;
        }

        int r = 16;
        int result;
        for (int i = 1; result < bytes_requested; i++) {
                result = r*i;
        }
        int diff = result - bytes_requested;

        return bytes_requested + diff;
}

void* x_malloc(int bytes_requested)
{
        if (bytes_requested % 16 != 0) {
                bytes_requested = round_up_bytes(bytes_requested);
        }

        if (is_first_allocation == TRUE) {
                first_allocated_chunk = sbrk(sizeof(struct Chunk) + bytes_requested);
                first_allocated_chunk->size = bytes_requested;
                first_allocated_chunk->free = FALSE;
                first_allocated_chunk->next_chunk = first_allocated_chunk;
                first_allocated_chunk->prev_chunk = first_allocated_chunk;

                is_first_allocation = FALSE;

                return &first_allocated_chunk->usable_size;
        } else {
                struct Chunk *chunk = sbrk(sizeof(struct Chunk) + bytes_requested);
                chunk->size = bytes_requested;
                chunk->free = FALSE;

                chunk->prev_chunk = first_allocated_chunk->prev_chunk;
                first_allocated_chunk->prev_chunk->next_chunk = chunk;
                chunk->next_chunk = first_allocated_chunk;
                first_allocated_chunk->prev_chunk = chunk;

                return &chunk->usable_size;
        }
}

void x_free(void *data_ptr)
{
/*
        Searches for an already free chunk with the requested size
        or larger and returns a pointer to usable_size sector of
        the chunk found or NULL if doesn't find any.
        Returning NULL makes easier to verify in the caller if a
        free chunk has been found or not.
*/
        if (data_ptr == NULL) {
                return;
        }
        char *ptr = data_ptr;
        char *pptr = ptr - 20;
        *pptr = TRUE;
}

void* search_free_chunk(int bytes_requested)
{
        struct Chunk *chunk = first_allocated_chunk;
        do {
                if (chunk->size >= bytes_requested) {
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
