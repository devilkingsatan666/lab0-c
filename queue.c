#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head)
        INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (head == NULL)
        return;

    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list)
        q_release_element(entry);

    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;

    size_t len = s == NULL ? 0 : strlen(s);
    element_t *entry = malloc(sizeof(element_t));
    if (entry == NULL)
        return false;
    entry->value = malloc(sizeof(char) * (len + 1));
    if (entry->value == NULL) {
        free(entry);
        return false;
    }
    strncpy(entry->value, s, len);
    entry->value[len] = '\0';
    list_add(&entry->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;

    size_t len = s == NULL ? 0 : strlen(s);
    element_t *entry = malloc(sizeof(element_t));
    if (entry == NULL)
        return false;
    entry->value = malloc(sizeof(char) * (len + 1));
    if (entry->value == NULL) {
        free(entry);
        return false;
    }
    strncpy(entry->value, s, len);
    entry->value[len] = '\0';
    list_add_tail(&entry->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;

    element_t *first = list_entry(head->next, element_t, list);
    list_del_init(head->next);

    if (sp == NULL)
        return first;

    size_t len = first->value == NULL ? 0 : bufsize - 1;
    strncpy(sp, first->value, len);
    sp[len] = '\0';

    return first;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;

    element_t *last = list_entry(head->prev, element_t, list);
    list_del_init(head->prev);

    if (sp == NULL)
        return last;

    size_t len = last->value == NULL ? 0 : bufsize - 1;
    strncpy(sp, last->value, len);
    sp[len] = '\0';

    return last;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (head == NULL)
        return 0;

    int size = 0;
    struct list_head *node;

    list_for_each (node, head)
        size++;

    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    if (head == NULL || list_empty(head))
        return false;

    struct list_head *slow = head->next, *fast = head->next;

    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }

    list_del(slow);
    q_release_element(list_entry(slow, element_t, list));

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (head == NULL)
        return false;

    bool state = false;
    element_t *node, *safe;
    struct list_head *cut_from = head, *dump = q_new(), tmp;

    list_for_each_entry_safe (node, safe, head, list) {
        bool dup =
            (&safe->list != head && strcmp(node->value, safe->value) == 0);
        if (state) {
            if (!dup) {
                list_cut_position(&tmp, cut_from, &node->list);
                list_splice_tail_init(&tmp, dump);
            }
        } else {
            if (dup) {
                cut_from = node->list.prev;
            }
        }
        state = dup;
    }

    q_free(dump);
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (head == NULL)
        return;

    struct list_head *node = head->next;
    while (node != head && node->next != head) {
        list_move_tail(node->next, node);
        node = node->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (head == NULL)
        return;

    LIST_HEAD(tmp);
    struct list_head *node, *safe;
    list_splice_init(head, &tmp);
    list_for_each_safe (node, safe, &tmp) {
        list_move(node, head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (head == NULL)
        return;

    int i = 0;
    LIST_HEAD(tmp);
    struct list_head *node, *safe, *begin = head;
    list_for_each_safe (node, safe, head) {
        if (++i == k) {
            list_cut_position(&tmp, begin, node);
            q_reverse(&tmp);
            list_splice_tail_init(&tmp, safe);
            begin = safe->prev;
            i = 0;
        }
    }
}

void static inline __merge(struct list_head *L1,
                           struct list_head *L2,
                           bool descend)
{
    struct list_head *head1 = L1, *head2 = L2;

    for (L1 = L1->next, L2 = L2->next; L1 != head1 && L2 != head2;) {
        element_t *E1 = list_entry(L1, element_t, list);
        if (E1->value == NULL) {
            L1 = L1->next;
            continue;
        }

        element_t *E2 = list_entry(L2, element_t, list);
        if (E2->value == NULL) {
            L2 = L2->next;
            continue;
        }

        if ((strcmp(E1->value, E2->value) > 0) ^ descend) {
            L2 = L2->next;
            list_move_tail(L2->prev, L1);
        } else {
            L1 = L1->next;
        }
    }

    list_splice_tail_init(head2, head1);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (head == NULL || list_empty(head) || list_is_singular(head))
        return;

    LIST_HEAD(tmp);
    struct list_head *left = head, *right = &tmp;
    struct list_head *slow = head->next, *fast = head->next;
    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }

    list_cut_position(right, head, slow->prev);
    q_sort(left, descend);
    q_sort(right, descend);

    __merge(left, right, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (head == NULL)
        return 0;

    int size = 0;
    element_t *node, *safe;
    char *min = list_last_entry(head, element_t, list)->value;

    q_reverse(head);
    list_for_each_entry_safe (node, safe, head, list) {
        if (node->value == NULL || strcmp(node->value, min) > 0) {
            list_del(&node->list);
            q_release_element(node);
        } else {
            size++;
            min = node->value;
        }
    }
    q_reverse(head);

    return size;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (head == NULL)
        return 0;

    int size = 0;
    element_t *node, *safe;
    char *max = list_last_entry(head, element_t, list)->value;

    q_reverse(head);
    list_for_each_entry_safe (node, safe, head, list) {
        if (node->value == NULL || strcmp(node->value, max) < 0) {
            list_del(&node->list);
            q_release_element(node);
        } else {
            size++;
            max = node->value;
        }
    }
    q_reverse(head);

    return size;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (head == NULL || list_empty(head))
        return 0;

    struct list_head *prev = head->prev;
    while (prev != head->next)
        for (struct list_head *next = head->next;
             prev != next && prev->next != next && next->prev != prev;
             prev = prev->prev, next = next->next) {
            queue_contex_t *prev_ctx = list_entry(prev, queue_contex_t, chain);
            queue_contex_t *next_ctx = list_entry(next, queue_contex_t, chain);
            if (next_ctx->q == NULL)
                next_ctx->q = prev_ctx->q;
            else if (prev_ctx->q != NULL)
                __merge(next_ctx->q, prev_ctx->q, descend);
            next_ctx->size += prev_ctx->size;
            prev_ctx->size = 0;
        }

    queue_contex_t *merged = list_first_entry(head, queue_contex_t, chain);
    return merged->size;
}
