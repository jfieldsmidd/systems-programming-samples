/*
 *
 * my-malloc.c
 *
 */

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>


#define PAGE_SIZE 4096
#define METALLOC_SIZE (intRoundUp16(sizeof(struct metalloc)))

struct metalloc {
  size_t num_bytes_requested;
  struct metalloc *next;
  struct metalloc *prev;
  void *start_of_alloc;
};

void *malloc(size_t input_size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
size_t malloc_usable_size(void *ptr);
static struct metalloc *insertMetalloc (struct metalloc *previousMetalloc, struct metalloc *nextMetalloc, size_t size);
static int intRoundUp16(int input);
static int allocTotalSize(struct metalloc *alloc);
static int makeSpaceForAlloc(void *newAllocStart, size_t input_size);


static void *program_break = NULL;

static struct metalloc *head_metalloc = NULL; // will always have NULL prev


void *
malloc(size_t input_size) {
  // program's first call to malloc
  if (!program_break) {
    // initialize program_break
    program_break = sbrk(0);
    if (program_break == (void *) -1) {
      return NULL;
    }
    head_metalloc = (struct metalloc *) program_break;
    if (sbrk(PAGE_SIZE) != (void *) -1) {
      program_break = (void *)((char *) program_break + PAGE_SIZE);
    } else {
      return NULL;
    }

    // initialize head_metalloc
    head_metalloc->num_bytes_requested = (size_t) 0;
    head_metalloc->next = NULL;
    head_metalloc->prev = NULL;
    head_metalloc->start_of_alloc = (void *) ((char*) head_metalloc + METALLOC_SIZE);
  }

  // Go to first element of linked list
  struct metalloc *current_metalloc = head_metalloc;
  // loop through metallocs to see if there is space for the new alloc between any of them
  while (current_metalloc->next) {

    // Check if there is space between the end of the current allocation and
    // the start of the next allocation to fit an allocation of the given size
    char *end_of_current = (char *) current_metalloc + allocTotalSize(current_metalloc);
    int size_of_new_alloc = METALLOC_SIZE + intRoundUp16(input_size);
    if ( end_of_current + size_of_new_alloc <= (char *) current_metalloc->next ) {
    // If so, initialize new element and set next_alloc.previous_alloc
    // to point to new element and set next_alloc to point to new allocation as well
      struct metalloc *next_metalloc = current_metalloc->next;
      current_metalloc->next = insertMetalloc(current_metalloc, current_metalloc->next, input_size);
      next_metalloc->prev = current_metalloc->next;
      return current_metalloc->next->start_of_alloc;
    }
    // Otherwise, go to next element
    current_metalloc = current_metalloc->next;
  }

  // make sure there is enough space between end of last alloc and program_break for new alloc
  void *newAllocStart = (void *)((char *) current_metalloc + allocTotalSize(current_metalloc) + METALLOC_SIZE);
  if (makeSpaceForAlloc(newAllocStart, input_size) == 0) {
    return NULL;
  }

  // Set up new alloc and modify current metalloc accordingly
  current_metalloc->next = insertMetalloc(current_metalloc, NULL, input_size);
  return current_metalloc->next->start_of_alloc;
}

void
free(void *ptr) {
  if (ptr) {
    struct metalloc *current_metalloc = (struct metalloc *) ((char *) ptr - METALLOC_SIZE);
    current_metalloc->prev->next = current_metalloc->next;
    if (current_metalloc->next) {
      current_metalloc->next->prev = current_metalloc->prev;
    }
  }
}

void *
calloc(size_t nmemb, size_t size) {
  if (!nmemb || !size) {
    return NULL;
  } else if (nmemb > INT_MAX || size > INT_MAX || nmemb > INT_MAX / size) {
    errno = EOVERFLOW;
    return NULL;
  } else {
    void *new_alloc = malloc(nmemb * size);
    if (new_alloc) {
      new_alloc = memset(new_alloc, 0, nmemb * size);
    }
    return new_alloc;
  }
}

void *
realloc(void *ptr, size_t size) {
  if (!ptr) {
    return malloc(size);
  } else if (!size){
    free(ptr);
    return NULL;
  }
  struct metalloc *meta_ptr = (struct metalloc *) ((char *) ptr - METALLOC_SIZE);
  size_t orig_size = meta_ptr->num_bytes_requested;
  if (size <= orig_size) {
    meta_ptr->num_bytes_requested = size;
    return ptr;
  } else if (meta_ptr->next) {
    if ((char *) ptr + intRoundUp16(size) < (char *) meta_ptr->next) {
      meta_ptr->num_bytes_requested = size;
      return ptr;
    } else {
      void *new_alloc = malloc(size);
      if (new_alloc) {
        memcpy(new_alloc, ptr, orig_size);
        free(ptr);
      }
      return new_alloc;
    }
  } else {
    // Make sure there is enough space before program_break to increase alloc to specified size
    if (makeSpaceForAlloc(ptr, size) == 0) {
      return NULL;
    }
    meta_ptr->num_bytes_requested = size;
    return ptr;
  }
}

size_t
malloc_usable_size(void *ptr) {
  if (ptr) {
    struct metalloc *meta = (struct metalloc *)((char *) ptr - METALLOC_SIZE);
    return meta->num_bytes_requested;
  } else {
    return (size_t) 0;
  }
}

// ============ HELPER FUNCTIONS ============

// helper function to round integer up to nearest multiple of 16
static int
intRoundUp16(int input) {
  int remainder = input % 16;
  int round_up = 16 - remainder;
  return input + round_up;
}

// insert a new metalloc into linked list using given previous and next metallocs and size
static struct metalloc *
insertMetalloc (struct metalloc *previousMetalloc, struct metalloc *nextMetalloc, size_t input_size) {
  struct metalloc *new_metalloc_ptr = (struct metalloc *) ((char *) previousMetalloc + (allocTotalSize(previousMetalloc)));
  new_metalloc_ptr->prev = previousMetalloc;
  new_metalloc_ptr->next = nextMetalloc;
  new_metalloc_ptr->num_bytes_requested = input_size;
  new_metalloc_ptr->start_of_alloc = (void *) ((char *) new_metalloc_ptr + METALLOC_SIZE);
  return new_metalloc_ptr;
}

static int
allocTotalSize(struct metalloc *alloc) {
  return METALLOC_SIZE + intRoundUp16(alloc->num_bytes_requested);
}

// Make sure there is enough space before program_break to store an alloc of the
// specified size starting at the given address
static int
makeSpaceForAlloc(void *newAllocStart, size_t input_size) {
  intptr_t space_in_heap = (intptr_t)((char*) program_break - (char*) newAllocStart);
  int space_needed = intRoundUp16(input_size);
  if ( space_in_heap < space_needed) {
    int space_to_expand = (((space_needed - space_in_heap) / PAGE_SIZE) * PAGE_SIZE) + PAGE_SIZE;
    if (sbrk(space_to_expand) != (void *) -1) {
      program_break = (void *)((char *) program_break + space_to_expand);
    } else {
      return 0;
    }
  }
  return 1;
}
