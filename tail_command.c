#include <stdio.h>
#include <malloc.h>

#define STRING_INCREASE 100
#define STRINGS_NUMBER 10

typedef enum {
    false, true
} bool;

typedef struct Node {
    const char *string;
    struct Node *next;
} Node;

Node *MakeNode(const char *new_string) {
    Node *node = (Node *) malloc(sizeof(Node));
    node->string = new_string;
    node->next = NULL;
    return node;
}

typedef struct Queue {
    Node *head;
    Node *tail;
    size_t queue_size;
} Queue;

bool IsEmpty(Queue *queue) {
    if (!queue) {
        return false;
    }

    if (queue->queue_size) {
        return false;
    } else {
        return true;
    }
}

bool Enqueue(Queue *queue, Node *node) {
    if (!queue || (!node)) {
        return false;
    }

    if (!queue->queue_size) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }

    queue->queue_size++;
    return true;
}

Node *Dequeue(Queue *queue) {
    Node *front;

    if (IsEmpty(queue)) {
        return NULL;
    }

    front = queue->head;
    queue->head = queue->head->next;
    queue->queue_size--;
    return front;
}

Queue *MakeQueue() {
    Queue *queue = (Queue *) malloc(sizeof(Queue));

    if (!queue) {
        return NULL;
    }

    queue->queue_size = 0;
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
};

void DeleteQueue(Queue *queue) {
    Node *current;

    while (!IsEmpty(queue)) {
        current = Dequeue(queue);
        free(current);
    }

    free(queue);
}

void PushStringToQueue(Queue *queue, char *string) {
    Node *node = MakeNode(string);

    if (queue->queue_size < STRINGS_NUMBER) {
        Enqueue(queue, node);
    } else {
        Dequeue(queue);
        Enqueue(queue, node);
    }
}

Queue *GetLastStrings(FILE *file, Queue *queue) {
    size_t string_index = 0, string_buffer = 100 * sizeof(char);
    char *string = (char *) malloc(string_buffer);
    int symbol;
    char last = '\0';

    while ((symbol = fgetc(file)) != EOF) {

        last = (char) symbol;
        if ((char) symbol != '\n') {
            string[string_index] = (char) symbol;
            ++string_index;
        } else {
            string[string_index] = '\n';
            ++string_index;
            string[string_index] = '\0';
            string_index = 0;
            string_buffer = 100 * sizeof(char);
            PushStringToQueue(queue, string);
            string = NULL;
            string = realloc(string, string_buffer);
        }

        if (string_index * sizeof(char) >= string_buffer) {
            string_buffer += STRING_INCREASE;
            string = realloc(string, string_buffer);
        }
    }

    if (last != '\n') {
        string[string_index] = '\0';
        PushStringToQueue(queue, string);
    }

    return queue;
}

void PrintLastStrings(Queue *queue) {
    Node *node;

    while (!IsEmpty(queue)) {
        node = Dequeue(queue);
        printf("%s", node->string);
    }
}

int main(int argc, char **argv) {

    if (argc == 1) {
        FILE *file = stdin;
        Queue *queue_pointer = MakeQueue();
        queue_pointer = GetLastStrings(file, queue_pointer);
        PrintLastStrings(queue_pointer);
        DeleteQueue(queue_pointer);
        fclose(file);
    } else {
        for (size_t i = 1; i < argc; ++i) {
            FILE *file = fopen(argv[i], "r");
            printf("==> %s <==\n", argv[i]);

            if (!file) {
                perror(argv[i]);
                continue;
            }

            Queue *queue_pointer = MakeQueue();
            queue_pointer = GetLastStrings(file, queue_pointer);
            PrintLastStrings(queue_pointer);
            if (i != (argc - 1)) {
                printf("\n");
            }
            DeleteQueue(queue_pointer);
            fclose(file);
        }
    }

    return 0;
}