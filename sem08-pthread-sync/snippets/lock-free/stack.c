#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct stack_node {
  int payload;
  struct stack_node* prev;
} stack_node_t;

typedef struct lf_mpmc_stack {
  struct stack_node* _Atomic top;
} lf_mpmc_stack;

void stack_init(lf_mpmc_stack* stack) {
  stack->top = NULL;
}

void stack_destroy(lf_mpmc_stack* stack) {
  stack_node_t* curr = stack->top;
  while (curr != NULL) {
    stack_node_t* prev = curr->prev;
    free(curr);
    curr = prev;
  }
}

void stack_push(lf_mpmc_stack* stack, int value) {
  stack_node_t* node = malloc(sizeof(stack_node_t));
  node->payload = value;

  while (true) {
    stack_node_t* top = atomic_load(&stack->top);
    node->prev = top;
    if (atomic_compare_exchange_weak(&stack->top, &top, node)) {
      return;
    }
  }
}

stack_node_t* stack_pop(lf_mpmc_stack* stack) {
  stack_node_t* top;
  while ((top = atomic_load(&stack->top)) != NULL) {
    if (atomic_compare_exchange_weak(&stack->top, &top, top->prev)) {
      return top;
    }
  }
}

//////////////////////////////////////////////////////////////////////

const int n_pushes = 100500;

void* producer(void* arg) {
  lf_mpmc_stack* stack = (lf_mpmc_stack*)arg;
  for (int i = 0; i < n_pushes; ++i) {
    stack_push(stack, i);
  }

  return NULL;
}

void* consumer(void* arg) {
  lf_mpmc_stack* stack = (lf_mpmc_stack*)arg;
  for (int i = 0; i < n_pushes; ++i) {
    stack_node_t* node;
    while ((node = stack_pop(stack)) == NULL) {
      // retry
    }

    printf("got: %d\n", node->payload);
    free(node);
  }

  return NULL;
}

int main() {
  lf_mpmc_stack stack;
  stack_init(&stack);

  pthread_t p;
  pthread_create(&p, /*attr=*/NULL, &producer, /*arg=*/&stack);
  pthread_t c;
  pthread_create(&c, /*attr=*/NULL, &consumer, /*arg=*/&stack);

  pthread_join(p, /*thread_return=*/NULL);
  pthread_join(c, /*thread_return=*/NULL);

  stack_destroy(&stack);
  return 0;
}
