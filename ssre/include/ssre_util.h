#ifndef _SSRE_UTIL_H_
#define _SSRE_UTIL_H_

namespace ssre 
{
    template<typename T>
    class LinkedListNode {
    public:
        LinkedListNode<T> *next = nullptr;
        LinkedListNode<T> *prev = nullptr;
        T* const payload;
        LinkedListNode(T *_payload) : payload(_payload) {};
        void insert(LinkedListNode<T> *node)
        {
            node->next = next;
            if (next)
                next->prev = node;
            next = node;
            node->prev = this;
        }
        void remove()
        {
            if (prev)
                prev->next = next;
            if (next)
                next->prev = prev;
        }
    };
}

#endif
