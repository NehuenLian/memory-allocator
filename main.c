#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

#define TRUE 1
#define FALSE 0

#define BYTES_MINIMUM_REQUIRED 16
#define ALIGNMENT_MULTIPLE 16

#define BACKOFF_3_POINTERS (sizeof(size_t)) * (3)

// 24/32 bytes
struct SbrkChunk {
        size_t size;
        int free;
        struct SbrkChunk *next_chunk;
        struct SbrkChunk *prev_chunk;
        void *usable_size;
};

// 8/16 bytes
struct MMAPCHunk {
        size_t size;
        void *usable_size;
};

int is_first_allocation = TRUE;
static struct SbrkChunk *first_allocated_chunk;

size_t round_up_bytes(size_t bytes_requested)
{
/*
        Rounds the number of bytes requested to a multiple of 16
        to match the page alignment.
        Multiplies the alignment value until it reaches the first
        number that is greater than the bytes requested.
        Then, gets the difference between that number and the
        bytes_requested value.
        Finally, makes an add between the diff value and the initial
        bytes_requested number, getting the nearest-round value to it.

        Is not the most efficient or the fastest round, but you
        probably don't want to assignate memory using a not page-aligned
        value. Although, a protection for that is needed.
*/
        if (bytes_requested < BYTES_MINIMUM_REQUIRED) {
                return BYTES_MINIMUM_REQUIRED;
        }

        int alignment = ALIGNMENT_MULTIPLE;
        size_t result;
        for (int i = 1; result < bytes_requested; i++) {
                result = alignment*i;
        }
        size_t diff = result - bytes_requested;

        return bytes_requested + diff;
}

void* alloc_set_break(size_t bytes_requested)
{

        if (is_first_allocation == TRUE) {
                first_allocated_chunk = sbrk(sizeof(struct SbrkChunk) + bytes_requested);
                first_allocated_chunk->size = bytes_requested;
                first_allocated_chunk->free = FALSE;
                first_allocated_chunk->next_chunk = first_allocated_chunk;
                first_allocated_chunk->prev_chunk = first_allocated_chunk;

                is_first_allocation = FALSE;

                return &first_allocated_chunk->usable_size;
        } else {
                struct SbrkChunk *chunk = sbrk(sizeof(struct SbrkChunk) + bytes_requested);
                chunk->size = bytes_requested;
                chunk->free = FALSE;

                chunk->prev_chunk = first_allocated_chunk->prev_chunk;
                first_allocated_chunk->prev_chunk->next_chunk = chunk;
                chunk->next_chunk = first_allocated_chunk;
                first_allocated_chunk->prev_chunk = chunk;

                return &chunk->usable_size;
        }
}

void* alloc_memory_map(size_t bytes_requested)
{

        struct MMAPCHunk *chunk = mmap(NULL, sizeof(struct MMAPCHunk) + bytes_requested, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        chunk->size = bytes_requested;

        return &chunk->usable_size;
}

void* x_malloc(size_t bytes_requested)
{
        if (bytes_requested % ALIGNMENT_MULTIPLE != 0) {
                bytes_requested = round_up_bytes(bytes_requested);
        }

        if (bytes_requested < 1024) {
                void *chunk_usable_size = alloc_set_break(bytes_requested);
                return chunk_usable_size;
        } else {
                void *chunk_usable_size = alloc_memory_map(bytes_requested);
                return chunk_usable_size;
        }
}

void x_free(void *data_ptr)
{
/*
        Uses the pointer to the usable_size to go back n
        memory addresses to reach the free attribute of
        the current chunk and changes it to TRUE.
        Setting the free attribute to TRUE allows the search
        algorithm to select it for a new allocation.
*/
        if (data_ptr == NULL) {
                return;
        }
        char *ptr = data_ptr;
        char *pptr = ptr - BACKOFF_3_POINTERS;
        *pptr = TRUE;
}

void* search_free_chunk(size_t bytes_requested)
{
/*
        Using a First-Fit policy, searches for an already free
        chunk with the requested size or larger and returns a
        pointer to usable_size sector of the chunk found or NULL
        if doesn't find any.
        Returning NULL makes easier to verify in the caller if a
        free chunk has been found or not.
*/
        struct SbrkChunk *chunk = first_allocated_chunk;
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
