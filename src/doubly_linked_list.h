//
//  doubly_linked_list.h
//  CSEN79_StockBook_Project
//
//  Created by Emily Chen on 2026/11/18.
//

#ifndef doubly_linked_list_h
#define doubly_linked_list_h

#include <cstddef>
#include <cassert>
#include <iterator> //for bidirectional_iterator_tag


namespace CSEN79_StockBook
{

template <typename T>
class doubly_linked_list_node;

struct ListMetrics //the counters
{
    size_t comparisons = 0; //counts how many times a node's stored value is compared against a search target
    size_t nodesVisited = 0; //counts how many nodes are traversed during list operations
    size_t allocations = 0; //counts how many heap nodes are allocated with "new"
    size_t deallocations = 0; //counts how many heap nodes are freed with delete
    
    void reset()
    {
        comparisons = 0;
        nodesVisited = 0;
        allocations = 0;
        deallocations = 0;
    }
};

/*
 Doubly Linked List class template
 - implements doubly linked list with a head pointer (front) and a tail pointer (back).
 - FIFO ordering: pushBack() enqueues newest at tail; popFront() dequeues oldest from head.
 Invariant:
 - When empty: head_ptr == nullptr, count == 0
 - When not empty: count > 0, head node's prev == nullptr, and tail node's next == nullptr; head_ptr points to head node and tail_ptr points to tail node.
 Value Semantics:
 - Assignment operator and Copy constructor are provided by user.
 */

template <typename T>
class doubly_linked_list
{
public:
    typedef size_t size_type;
    class iterator;
    class const_iterator;
    
    //constructors, deconstructors
    doubly_linked_list();
    doubly_linked_list(const T& init_data);
    ~doubly_linked_list();
    doubly_linked_list(const doubly_linked_list& s);
    
    //assignment operator
    doubly_linked_list& operator=(const doubly_linked_list& s);
    
    //modification member functions
    iterator begin();
    iterator end();
    void push_back(const T& value);
    void pop_front();
    T& front();
    bool remove(const T& target);
    void clear();
    bool empty() const; //for caller to check preconditions, throw errors accordingly
    size_type size() const;
    
    //const member functions
    const_iterator begin() const;
    const_iterator end() const;
    const T& front() const;
    
    //iterator class
    /*
     class iterator : public std::iterator<std::bidirectional_iterator_tag, T>
     
     the following defines the five standard iterator member type aliases directly inside the iterator class
     instead of inheriting from std::iterator to avoid triggering a deprecation compiler warning
     */
    class iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        iterator(doubly_linked_list_node<T>* initial = nullptr)
        {
            current = initial;
        }
        
        T& operator*() const
        {
            return current->get_data();
        }
        
        iterator& operator++() // Prefix ++
        {
            current = current->next();
            return *this;
        }
        
        iterator operator++(int) // Postfix ++
        {
            iterator original(current);
            current = current->next();
            return original;
        }
        
        iterator& operator--() // Prefix --
        {
            if(current == nullptr) //if at end()
            {
                current = end();
            }
            else
            {
                current = current->prev();
            }
            return *this;
        }
        
        iterator operator--(int) // Postfix --
        {
            iterator original(current);
            if(current == nullptr) //if at end()
            {
                current = end();
            }
            else
            {
                current = current->prev();
            }
            return original;
        }
        
        bool operator==(const iterator other) const
        {
            return current == other.current;
        }
        
        bool operator!=(const iterator other) const
        {
            return current != other.current;
        }
        
    private:
        doubly_linked_list_node<T>* current;
        friend class doubly_linked_list<T>;
    };
    
    /*
     class const_iterator : public std::iterator<std::bidirectional_iterator_tag, const T>
     
     the following defines the five standard iterator member type aliases directly inside the iterator class
     instead of inheriting from std::iterator to avoid triggering a deprecation compiler warning
     */
    class const_iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const T*;
        using reference         = const T&;

        const_iterator(const doubly_linked_list_node<T>* initial = nullptr)
        {
            current = initial;
        }
        
        // Allow implicit conversion from Iterator to ConstIterator
        const_iterator(const iterator& it)
        {
            current = it.current;
        }
        
        const T& operator*() const
        {
            return current->get_data();
        }
        
        const_iterator& operator++() // Prefix ++
        {
            current = current->next();
            return *this;
        }
        
        const_iterator operator++(int) // Postfix ++
        {
            const_iterator original(current);
            current = current->next();
            return original;
        }
        
        const_iterator& operator--() // Prefix --
        {
            current = current->prev();
            return *this;
        }
        
        const_iterator operator--(int) // Postfix --
        {
            const_iterator original(current);
            current = current->prev();
            return original;
        }
        
        bool operator==(const const_iterator other) const
        {
            return current == other.current;
        }
        
        bool operator!=(const const_iterator other) const
        {
            return current != other.current;
        }
        
    private:
        const doubly_linked_list_node<T>* current;
        friend class doubly_linked_list<T>;
    };
    
    const ListMetrics& metrics() const
    {
        return metrics_;
    }
    void resetMetrics() const
    {
        metrics_.reset();
    }
    
private:
    doubly_linked_list_node<T> * head_ptr;
    doubly_linked_list_node<T> * tail_ptr;
    size_type count;
    
    mutable ListMetrics metrics_;
};

//implementations for doubly_linked_list

//constructors, deconstructors
template <typename T>
doubly_linked_list<T>::doubly_linked_list()
{
    head_ptr = nullptr;
    tail_ptr = nullptr;
    count = 0;
}

template <typename T>
doubly_linked_list<T>::doubly_linked_list(const T& init_data)
    : head_ptr(nullptr), tail_ptr(nullptr), count(0)
{
    push_back(init_data);
}

template <typename T>
doubly_linked_list<T>::~doubly_linked_list()
{
    //Postcondition: Release all remaining nodes to prevent memory leaks.
    clear();
}

template <typename T>
doubly_linked_list<T>::doubly_linked_list(const doubly_linked_list<T>& s)
    : head_ptr(nullptr), tail_ptr(nullptr), count(0)
{
    for (const_iterator it = s.begin(); it != s.end(); it++)
    {
        push_back(*it);
    }
}

//assignment operator
template <typename T>
doubly_linked_list<T>& doubly_linked_list<T>::operator=(const doubly_linked_list<T>& s)
{
    if(this == &s)
    {
        return *this;
    }
    
    clear();
    
    for (const_iterator it = s.begin(); it != s.end(); it++)
    {
        push_back(*it);
    }
    
    return *this;
}

//modification member functions

template <typename T>
typename doubly_linked_list<T>::iterator doubly_linked_list<T>::begin()
{
    return iterator(head_ptr);
}

template <typename T>
typename doubly_linked_list<T>::iterator doubly_linked_list<T>::end()
{
    return iterator(nullptr);
}

template <typename T>
void doubly_linked_list<T>::push_back(const T& value)
{
    //Precondition: value is a valid input
    doubly_linked_list_node<T> * temp = new doubly_linked_list_node<T>(value);
    metrics_.allocations++;
    
    if(head_ptr == nullptr) //if empty
    {
        head_ptr = temp;
        tail_ptr = temp;
        count++;
    }
    else
    {
        tail_ptr->set_next(temp);
        temp->set_prev(tail_ptr);
        tail_ptr = temp;
        count++;
    }
    
    return;
    //Postcondition: appends a node containing the value at tail (back), updates member variables.
}

template <typename T>
void doubly_linked_list<T>::pop_front()
{
    //Precondition: linked-list can not be empty
    assert(head_ptr != nullptr);
    
    if(head_ptr->next() == nullptr) //if it's the last node
    {
        delete head_ptr;
        metrics_.deallocations++;
        
        head_ptr = nullptr;
        tail_ptr = nullptr;
        count = 0;
    }
    else
    {
        doubly_linked_list_node<T> * temp = head_ptr;
        head_ptr = head_ptr->next();
        delete temp;
        metrics_.deallocations++;
        
        count--;
        head_ptr->set_prev(nullptr);
    }
    
    return;
    //Postcondition: removes the node at head (front), updates member variables
}

template <typename T>
T& doubly_linked_list<T>::front()
{
    //Precondition: linked-list can not be empty
    assert(head_ptr != nullptr);
    
    return head_ptr->get_data();
    //Postcondition: returns head node's data
}

template <typename T>
bool doubly_linked_list<T>::remove(const T& target)
{
    //Precondition: data type T has an operator== to check if identical data
    //(T is an Order* for this project)
    
    for(doubly_linked_list_node<T>* temp = head_ptr; temp != nullptr; temp = temp->next()) //traverse list from head
    {
        //for each visited node, increment metrics_.nodesVisited and metrics_.comparisons.
        metrics_.nodesVisited++;
        metrics_.comparisons++;
        if(temp->get_data() == target) //when found
        {
            if(temp->prev() != nullptr) //target node is not the head node
            {
                temp->prev()->set_next(temp->next());
            }
            else //target node is head node
            {
                head_ptr = temp->next();
            }
            
            if(temp->next() != nullptr) //target node is not the tail node
            {
                temp->next()->set_prev(temp->prev());
            }
            else
            {
                tail_ptr = temp->prev();
            }
            
            delete temp; //deallocate memory
            metrics_.deallocations++;
            
            count--;
            return true;
        }
    }
    
    return false;
    
    //Postcondition: returns true if removed successfully, returns false if not; for each visited node, increment metrics_.nodesVisited and metrics_.comparisons; deallocate node memory to remove, and increment metrics_.deallocations, decrement size_
}

template <typename T>
void doubly_linked_list<T>::clear()
{
    if((head_ptr == nullptr) && (count == 0)) //if empty already
    {
        return;
    }
    
    while(head_ptr != nullptr)
    {
        doubly_linked_list_node<T> * temp = head_ptr;
        head_ptr = head_ptr->next();
        delete temp;
        metrics_.deallocations++;
    }
    
    head_ptr = nullptr;
    tail_ptr = nullptr;
    count = 0;
    
    return;
    //Postcondition: deallocate all nodes in the linked list and updates member variable.
}

template <typename T>
bool doubly_linked_list<T>::empty() const
{
    return ((head_ptr == nullptr) && (count == 0));
    //Postcondition: return true if head_ptr == nullptr;
}

template <typename T>
size_t doubly_linked_list<T>::size() const
{
    return count;
    //Postcondition: return number of items in list
}

//const member functions

template <typename T>
typename doubly_linked_list<T>::const_iterator doubly_linked_list<T>::begin() const
{
    return const_iterator(head_ptr);
}

template <typename T>
typename doubly_linked_list<T>::const_iterator doubly_linked_list<T>::end() const
{
    return const_iterator(nullptr);
}

template <typename T>
const T& doubly_linked_list<T>::front() const
{
    //Precondition: linked-list can not be empty
    assert(head_ptr != nullptr);
    
    return (head_ptr->get_data());
    //Postcondition: returns head node's data
}


//doubly_linked_list_node template clsss
template <typename T>
class doubly_linked_list_node
{
public:
    
    //constructors
    doubly_linked_list_node(const T& init_data = T(),
                            doubly_linked_list_node* init_prev = nullptr,
                            doubly_linked_list_node* init_next = nullptr)
    : data(init_data), prev_link(init_prev), next_link(init_next) //member initializer list
    {
    }

    doubly_linked_list_node(const doubly_linked_list_node& s);
    
    //assignment operator
    doubly_linked_list_node& operator=(const doubly_linked_list_node& s);
    
    //modification member functions
    T& get_data();
    doubly_linked_list_node*& prev();
    doubly_linked_list_node*& next();
    void set_data(const T& d);
    void set_prev(doubly_linked_list_node* p);
    void set_next(doubly_linked_list_node* n);
    
    //const member functions
    const T& get_data() const;
    doubly_linked_list_node* const& prev() const;
    doubly_linked_list_node* const& next() const;
    
private:
    T data;
    doubly_linked_list_node * prev_link;
    doubly_linked_list_node * next_link;
};



//implementations

//copy constructor
template <typename T>
doubly_linked_list_node<T>::doubly_linked_list_node(const doubly_linked_list_node& s)
{
    data = s.data;
    prev_link = s.prev_link;
    next_link = s.next_link;
}

//assignment operator
template <typename T>
doubly_linked_list_node<T>& doubly_linked_list_node<T>::operator=(const doubly_linked_list_node& s)
{
    if(this == &s)
    {
        return *this;
    }
    
    data = s.data;
    prev_link = s.prev_link;
    next_link = s.next_link;
    
    return *this;
}

//modification member functions
template <typename T>
T& doubly_linked_list_node<T>::get_data()
{
    return data;
}

template <typename T>
doubly_linked_list_node<T>*& doubly_linked_list_node<T>::prev()
{
    return prev_link;
}

template <typename T>
doubly_linked_list_node<T>*& doubly_linked_list_node<T>::next()
{
    return next_link;
}

template <typename T>
void doubly_linked_list_node<T>::set_data(const T& d)
{
    data = d;
}

template <typename T>
void doubly_linked_list_node<T>::set_prev(doubly_linked_list_node* p)
{
    prev_link = p;
}

template <typename T>
void doubly_linked_list_node<T>::set_next(doubly_linked_list_node* n)
{
    next_link = n;
}

//const member functions
template <typename T>
const T& doubly_linked_list_node<T>::get_data() const
{
    return data;
}

template <typename T>
doubly_linked_list_node<T>* const& doubly_linked_list_node<T>::prev() const
{
    return prev_link;
}

template <typename T>
doubly_linked_list_node<T>* const& doubly_linked_list_node<T>::next() const
{
    return next_link;
}


}

#endif // doubly_linked_list_h
