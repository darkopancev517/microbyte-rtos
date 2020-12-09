#ifndef LIST_H
#define LIST_H

namespace microbyte {

class List
{
    public:

    List *next;

    List();

    void add(List *node);

    List *removeHead();

    static List *remove(List *list, List *node);
};

} // namespace microbyte

#endif /* LIST_H */
