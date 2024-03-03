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
    struct list_head *head =
        (struct list_head *) malloc(sizeof(struct list_head));
    if (head)
        INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (head == NULL)
        return;

    element_t *current = NULL, *next = NULL;
    list_for_each_entry_safe (current, next, head, list)
        q_release_element(current);

    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;

    element_t *elm = malloc(sizeof(element_t));
    if (elm == NULL)
        return false;

    size_t len = strlen(s);
    char *scpy = malloc(sizeof(char) * (len + 1));
    if (scpy == NULL) {
        free(elm);
        return false;
    }

    strncpy(scpy, s, len);
    scpy[len] = '\0';

    elm->value = scpy;
    list_add(&elm->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;

    element_t *elm = malloc(sizeof(element_t));
    if (elm == NULL)
        return false;

    size_t len = strlen(s);
    char *scpy = malloc(sizeof(char) * (len + 1));
    if (scpy == NULL) {
        free(elm);
        return false;
    }

    strncpy(scpy, s, len);
    scpy[len] = '\0';

    elm->value = scpy;
    list_add_tail(&elm->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;

    element_t *popped = list_entry(head->next, element_t, list);
    list_del_init(head->next);

    if (sp != NULL) {
        size_t len = bufsize - 1;
        strncpy(sp, popped->value, len);
        sp[len] = '\0';
    }

    return popped;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;

    element_t *popped = list_entry(head->prev, element_t, list);
    list_del_init(head->prev);

    if (sp != NULL) {
        size_t len = bufsize - 1;
        strncpy(sp, popped->value, len);
        sp[len] = '\0';
    }

    return popped;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (head == NULL)
        return 0;

    int size = 0;
    struct list_head *node = NULL;

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
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (head == NULL || list_empty(head) || list_is_singular(head))
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
    if (head == NULL || list_empty(head))
        return;

    struct list_head tmp;
    struct list_head *node, *safe;

    list_for_each_safe (node, safe, head) {
        tmp = *node;
        node->next = tmp.prev;
        node->prev = tmp.next;
    }

    tmp = *head;
    head->next = tmp.prev;
    head->prev = tmp.next;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (head == NULL || list_empty(head) || list_is_singular(head))
        return;

    int i = 0;
    LIST_HEAD(tmp);
    LIST_HEAD(dummy);
    struct list_head *node, *safe;

    list_for_each_safe (node, safe, head) {
        if (++i == k) {
            list_cut_position(&tmp, head, node);
            q_reverse(&tmp);
            list_splice_tail_init(&dummy, &tmp);
            i = 0;
        }
    }

    list_splice_tail_init(&dummy, node);
    *head = dummy;
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend) {}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (head == NULL || list_empty(head))
        return 0;

    int size = 1;
    element_t *node, *safe;
    char *min = list_last_entry(head, element_t, list)->value;

    q_reverse(head);

    list_for_each_entry_safe (node, safe, head->next, list) {
        if (strcmp(node->value, min) > 0) {
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
    if (head == NULL || list_empty(head))
        return 0;

    int size = 1;
    element_t *node, *safe;
    char *max = list_last_entry(head, element_t, list)->value;

    q_reverse(head);

    list_for_each_entry_safe (node, safe, head->next, list) {
        if (strcmp(node->value, max) < 0) {
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
    return 0;
}
