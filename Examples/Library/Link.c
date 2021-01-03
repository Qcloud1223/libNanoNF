#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

typedef struct Node
{
    uint8_t header[32];
    struct Node *prev, *next;
} Node;

Node *head = NULL, *last = NULL;

void InitializeFunction()
{
    head = malloc(sizeof(Node));
    // printf("[link] head: %p\n", head);
    // printf("checking printf inside the initial function of libLink\n");
    memset(head->header, 0, 32);
    head->prev = NULL;
    Node *cursor = head;
    for (int i = 1; i < 32; i += 1, cursor = cursor->next)
    {
        cursor->next = malloc(sizeof(Node));
        cursor->next->prev = cursor;
        memset(cursor->next->header, 0, 32);
    }
    last = cursor;
    last->next = NULL;
}

void ProcessPacket(uint8_t *packet, size_t packetLength)
{
    // assert(0);
    // printf("[link] start\n");
    // printf("[link] head: %p head->next: %p\n", head, head->next);
    Node *node = malloc(sizeof(Node));
    // printf("[link] after malloc\n");
    memcpy(node->header, packet, 32);
    for (Node *cursor = head; cursor; cursor = cursor->next)
    {
        if ((node->header[29] ^ cursor->header[29]) & 0x01)
        {
            // printf("[link] replace node\n");
            node->prev = cursor->prev;
            node->next = cursor->next;
            if (node->prev)
            {
                node->prev->next = node;
            }
            if (node->next)
            {
                node->next->prev = node;
            }
            assert(cursor);
            if (head == cursor) {
                head = node;
            } else if (last == cursor) {
                last = node;
            }
            free(cursor);
            return;
        }
    }
    // printf("[link] shift node\n");
    last->next = node;
    node->prev = last;
    node->next = NULL;
    last = node;
    head = head->next;
    // printf("[link] head: %p head->next: %p\n", head, head->next);
    // printf("[link] before shift free: %p\n", head->prev);
    assert(head->prev);
    free(head->prev);
    // printf("[link] after shift free\n");
    head->prev = NULL;
}