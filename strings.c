#include <stdio.h>
#include <malloc.h>

#define QUEUE_CAPACITY 4

typedef struct Node {
    char symbol;
    struct Node *next;
} Node;

Node *MakeNode() {
    Node *node = (Node *) malloc(sizeof(Node));
    node->symbol = '\0';
    node->next = NULL;
    return node;
}

typedef struct Queue {
    Node *head;
    Node *tail;
    size_t queue_size;
} Queue;

Queue *MakeQueue(Node *node_1, Node *node_2, Node *node_3, Node *node_4) {
    Queue *queue = (Queue *) malloc(sizeof(Queue));
    queue->queue_size = 0;

    node_1->next = node_2;
    node_2->next = node_3;
    node_3->next = node_4;
    node_4->next = NULL;

    queue->head = node_1;
    queue->tail = node_1;
    return queue;
};

void PushSymbolToQueue(char symbol, Queue *queue) {
    if (queue->queue_size < QUEUE_CAPACITY) {
        ++queue->queue_size;
        queue->tail->symbol = symbol;
        if (queue->queue_size != QUEUE_CAPACITY) {
            queue->tail = queue->tail->next;
        }
    } else {
        printf("%c", queue->head->symbol);
        queue->head->symbol = symbol;
        queue->tail->next = queue->head;
        queue->tail = queue->head;
        queue->head = queue->head->next;
    }
}

void PrintAndClearQueue(Queue *queue) {
    if (queue->queue_size < QUEUE_CAPACITY) {
        queue->queue_size = 0;
        queue->tail = queue->head;
    } else {
        Node *current = queue->head;
        while (queue->queue_size) {
            --queue->queue_size;
            printf("%c", queue->head->symbol);
            queue->head->symbol = '\0';
            queue->head = queue->head->next;
        }
        queue->head = current;
        queue->tail = current;
        printf("%c", '\n');
    }
}

void Strings(FILE *file, Queue *queue) {
    int symbol;

    while ((symbol = fgetc(file)) != EOF) {
        if ((symbol < 32 && (char) symbol != '\t') || symbol > 126) {
            PrintAndClearQueue(queue);
        } else {
            PushSymbolToQueue((char) symbol, queue);
        }
    }
    PrintAndClearQueue(queue);
}

int main(int argc, char **argv) {
    Node *node_1 = MakeNode();
    Node *node_2 = MakeNode();
    Node *node_3 = MakeNode();
    Node *node_4 = MakeNode();

    Queue *queue = MakeQueue(node_1, node_2, node_3, node_4);

    if (argc == 1) {
        FILE *file = stdin;
        Strings(file, queue);
        fclose(file);
    } else {
        for (size_t i = 1; i < argc; ++i) {
            FILE *file = fopen(argv[i], "r");

            if (!file) {
                perror(argv[i]);
                continue;
            }
            Strings(file, queue);
            fclose(file);
        }
    }

    free(node_1);
    free(node_2);
    free(node_3);
    free(node_4);
    free(queue);

    return 0;
}