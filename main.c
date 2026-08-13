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

void* my_malloc(int bytes)
{
        struct Chunk *chunk = sbrk(sizeof(struct Chunk) + bytes);
        chunk->size = bytes;
        chunk->free = 'n';
        chunk->next_chunk = NULL;
        chunk->prev_chunk = sbrk(0);

        return &chunk->usable_size;
}

int main()
{
        char *buffer = my_malloc(sizeof(char) * 16);
        buffer[0] = 'h';
        printf("%c\n", buffer[0]);

        return 0;
}
