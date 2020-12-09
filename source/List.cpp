#include <stddef.h>

#include "List.h"

namespace microbyte {

List::List()
{
    next = NULL;
}

void List::add(List *node)
{
    node->next = this->next;
    this->next = node;
}

List *List::removeHead()
{
    List *head = this->next;

    if (head != NULL)
    {
        this->next = head->next;
    }

    return head;
}

List *List::remove(List *list, List *node)
{
    while (list->next)
    {
        if (list->next == node)
        {
            list->next = node->next;
            return node;
        }
        list = list->next;
    }

    return list->next;
}

} // namespace microbyte
