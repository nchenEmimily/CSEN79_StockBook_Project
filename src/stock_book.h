//
//  stock_book.h
//  CSEN79_StockBook_Project
//
//  Created by Emily Chen on 2026/11/21.
//

#ifndef stock_book_h
#define stock_book_h

#include "order.h"
#include "doubly_linked_list_node.h"

#include <iostream>
#include <string>
#include <cassert>
#include <utility> //for pair
#include <algorithm> //for min
#include <cstddef> //for size_t
#include <functional> //for std::greater, std::less

//hand-built core structures
#include "B_tree.h" //TODO: implement
#include "doubly_linked_list.h" //list
#include "singly_linked_list.h" //forward list TODO: implement
#include "map.h"

//stl containers
#include <map> //B-tree and map
#include <list>
#include <forward_list> //TODO: compare with using list as well


namespace CSEN79_StockBook
{

struct historyTag
{
    char action;
    size_t num; //command number
};

template <typename ForwardList>
struct TailForwardList
{
    using iterator = typename ForwardList::iterator;
    using const_iterator = typename ForwardList::const_iterator;
    
    ForwardList list;
    iterator last; //Points to the last valid element (when non-empty)
    bool has_last = false;
    std::size_t count = 0; //Tracks the total number of elements/events
    
    void push_front(const std::pair<Order*, historyTag>& val);
    void push_back(const std::pair<Order*, historyTag>& val);
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
    bool empty() const;
};

struct HandBuiltTraits
{
    using BuyBTree = B_tree<double, doubly_linked_list<Order*>*, 4, std::greater<double>>;
    using SellBTree = B_tree<double, doubly_linked_list<Order*>*, 4, std::less<double>>;
    using IdOrderMap = map<Order::id_type, Order*>; //TODO: Map
    using OrderList = doubly_linked_list<Order*>;
    using forward_list = singly_linked_list<std::pair<Order*, struct historyTag>>; //TODO: singly_linked_list
    using UserHistoryMap = map<std::string, TailForwardList<forward_list>*>;
};

struct StdTraits
{
    using BuyBTree = std::map<double, std::list<Order*>*, std::greater<double>>;
    using SellBTree = std::map<double, std::list<Order*>*, std::less<double>>;
    using IdOrderMap = std::map<Order::id_type, Order*>;
    using OrderList = std::list<Order*>;
    using forward_list = std::forward_list<std::pair<Order*, struct historyTag>>;
    using UserHistoryMap = std::map<std::string, TailForwardList<forward_list>*>;
};

template <typename Traits>
struct stock_book
{
    using BuyBTree = typename Traits::BuyBTree;
    using SellBTree = typename Traits::SellBTree;
    using UserHistoryMap = typename Traits::UserHistoryMap; //a Map mapping String to a List of Order*
    using IdOrderMap = typename Traits::IdOrderMap;
    using OrderList = typename Traits::OrderList;
    using ForwardList = typename Traits::forward_list;
    
    //stack creations
    BuyBTree buyBook;
    SellBTree sellBook;
    UserHistoryMap userHistoryMap;
    IdOrderMap idOrderMap;
    
    ~stock_book();
    
    void buyLimit(const std::string& user, Order::id_type id, Order::size_type qty, double price, size_t num);
    void sellLimit(const std::string& user, Order::id_type id, Order::size_type qty, double price, size_t num);
    void buyMarket(const std::string& user, Order::id_type id, Order::size_type qty, size_t num);
    void sellMarket(const std::string& user, Order::id_type id, Order::size_type qty, size_t num);
    void cancelOrder(Order::id_type id);
    void bookPrint(std::size_t depth) const;
    void historyPrint(const std::string& user, std::size_t n) const;
    void historyPrint(const std::string& user) const;
};

template <typename Traits>
stock_book<Traits>::~stock_book()
{
    // 1. Clean up sellBook price-level lists and the Order objects pointed to by Order* stored
    for (auto it = sellBook.begin(); it != sellBook.end(); ++it)
    {
        OrderList* list = it->second;
        if (list != nullptr)
        {
            for (auto orderIt = list->begin(); orderIt != list->end(); ++orderIt)
            {
                delete (*orderIt);
            }
            delete list;
        }
    }
    
    // 2. Clean up buyBook price-level lists and the Order objects pointed to by Order* stored
    for (auto it = buyBook.begin(); it != buyBook.end(); ++it)
    {
        OrderList* list = it->second;
        if (list != nullptr)
        {
            for (auto orderIt = list->begin(); orderIt != list->end(); ++orderIt)
            {
                delete (*orderIt);
            }
            delete list;
        }
    }
    
    // 3. Clean up all user history lists and their stored history Order* copies
    for (auto it = userHistoryMap.begin(); it != userHistoryMap.end(); ++it)
    {
        TailForwardList<ForwardList>* history = it->second;
        if (history != nullptr)
        {
            for (auto histIt = history->begin(); histIt != history->end(); ++histIt)
            {
                delete histIt->first;
            }
            delete history;
        }
    }
    
    //buyBook, sellBook, userHistoryMap, idOrderMap will automatically run their own destructors afterwards since they are stack allocated
    
    // Post-condition: Release heap-allocated Order* and history structures cleanly.
    /*
     - stack creations:
     BuyBTree buyBook;
     SellBTree sellBook;
     UserHistoryMap userHistoryMap;
     IdOrderMap idOrderMap;
     
     - heap creations:
     1) OrderList
     in B-trees that store pointers to orders of the same price
     2) TailForwardList
     in map UserHistoryMap that stores pair{Order*,historyTag}(as the value) to a username(as the key)
     using forward_list = std::forward_list<std::pair<Order*, historyTag>>;
     using UserHistoryMap = std::map<std::string, TailForwardList<forward_list>>;
     */
}

template <typename Traits>
void stock_book<Traits>::buyLimit(const std::string& user, Order::id_type id, Order::size_type qty, double price, size_t num)
{
    //Pre-condition: id is a positive integer that have not been used; qty is a positive integer; price is a positive double
    assert((id > 0) && (qty > 0) && (price > 0.00));
    if(idOrderMap.find(id) != idOrderMap.end()) //if id existed
    {
        std::cout << "ERROR duplicate id " << id << std::endl;
        return;
    }
    
    //create Order object (heap creation)
    Order* current = new Order(id, price, qty, user);
    Order* copy = new Order(*current);
    
    //append Order copy in user's HISTORY to print "#<n> PLACE <id> BUY <qty> <price>"
    typename UserHistoryMap::iterator userHistoryMap_it = userHistoryMap.find(user);
    if(userHistoryMap_it != userHistoryMap.end()) //user is inserted and history (TailForwardList) has been created
    {
        //'b' means "PLACE...BUY"
        historyTag tag{'b',num};
        userHistoryMap_it->second->push_back(std::make_pair(copy, tag));
    }
    else
    {
        TailForwardList<ForwardList> * history = new TailForwardList<ForwardList>(); //heap creation
        historyTag tag{'b', num};
        history->push_back(std::make_pair(copy, tag));
        userHistoryMap_it = userHistoryMap.insert(std::make_pair(user, history))->first; //insert and update userHistoryMap_it to point to inserted
    }
    
    //match and fill from the best sell orders, any ask_price <= price
    while (!sellBook.empty() && (qty > 0)) //match until qty == 0 OR book ran out of matches
    {
        typename SellBTree::iterator bestIt = sellBook.begin(); //from lowest-to-highest priced orders
        double bestPrice = bestIt->first; //dereference a map iterator yields a std::pair<Key, Value>
        if(bestPrice <= price) //if match
        {
            OrderList* bestOrders = bestIt->second; //get the list of best opposite orders
            if(bestOrders->empty()) //if list is empty
            {
                delete bestOrders; //deallocate list
                sellBook.erase(bestIt); //deallocate B-tree node
                continue;
            }
            
            Order* oldestOrderAtBest = bestOrders->front(); //get the oldest order
            Order::size_type filled = std::min(oldestOrderAtBest->get_shares(), qty); //get amount of filled shared
            
            //append #<m> FILL <id> <fill_qty> <trade_price> in buyer & seller's HISTORY
            //userHistoryMap_it points to a pair of the buyer's name and History
            //'f' means "FILL..."
            Order* buyCopy = new Order(id, bestPrice, filled, user);
            historyTag tag{'f', num};
            userHistoryMap_it->second->push_back(std::make_pair(buyCopy, tag)); //append to buyer's History
            
            Order::id_type sellID = oldestOrderAtBest->get_id();
            std::string seller = oldestOrderAtBest->get_user();
            Order* sellCopy = new Order(sellID, bestPrice, filled, seller);
            userHistoryMap_it = userHistoryMap.find(seller);
            userHistoryMap_it->second->push_back(std::make_pair(sellCopy, tag)); //append to seller's History
            
            //print one line of TRADE <buy id> <sell id> <qty> <price>
            std::cout << "TRADE " << id << " " << sellID << " " << filled << " " << bestPrice << std::endl;
            
            //fill both orders
            oldestOrderAtBest->reduceShares(filled);
            current->reduceShares(filled);
            
            if (oldestOrderAtBest->isFilled()) //if ask is completely filled
            {
                //pop that order from its list in sell/ask B-tree
                bestOrders->pop_front(); //remove from list
                delete oldestOrderAtBest; //deallocate object
                
                auto mapIt = idOrderMap.find(sellID);
                if (mapIt != idOrderMap.end())
                {
                    mapIt->second = nullptr; //to avoid idOrderMap returning a dangling pointer
                }
                
                if(bestOrders->empty()) //if list is now empty, remove the node from sell/ask B-tree
                {
                    delete bestOrders; //deallocate list
                    sellBook.erase(bestIt); //unlink and deallocate node
                }
            }
            
            userHistoryMap_it = userHistoryMap.find(user); //points to buyer
            qty -= filled; //while-loop condition
        }
        else
        {
            break;
        }
    }
    
    //qty now holds the unfilled qty
    if(qty > 0)
    {
        current->setShares(qty);
        //rest by pushing the updated Order into the buy/bid B-tree
        typename BuyBTree::iterator buyBook_it = buyBook.find(price);
        
        if(buyBook_it != buyBook.end()) //if this price level already exists at it
        {
            //TODO: here assumed push_back() deals with empty lists; B-tree never leaves nodes with deallocated list
            (buyBook_it->second)->push_back(current);
        }
        else
        {
            OrderList * temp = new OrderList; //heap creation
            temp->push_back(current); //push order to list
            buyBook.insert(std::make_pair(price, temp)); //insert to buy/bid B-tree
        }
        //insert the Order's id and pointer as key and value pair to the idOrderMap
        idOrderMap.insert(std::make_pair(id, current));
        
        //print REST <id> <qty> <price>
        std::cout << "REST " << id << " " << qty << " " << price << std::endl;
    }
    else if(qty == 0)
    {
        delete current;
        
        //insert the Order's id and nullptr to the idOrderMap (for duplicate ID check)
        idOrderMap.insert(std::make_pair(id, nullptr));
    }
    return;
    /* Post-condition of buyLimit():
     1) check inputs, id not existed, create Order object (heap creation)
     - append pointer to Order's copy(also heap creation), #<n> PLACE <id> BUY <qty> <price> in user's HISTORY
     - print ERROR duplicate id <id> if id existed
     2) match and fill from the best sell orders, as a limit order (any ask_price <= price)
     - front, reduce (or pop_front), until qty == 0 OR book ran out of matches
     - reduceShares(lowest(qty, ask_qty)) for both orders
     - if ask is completely filled, pop that order from sell/ask B-tree (remove from list, and remove the price level from B-tree if necessary)
     - append #<m> FILL <id> <fill_qty> <trade_price> in buyer & seller's HISTORY
     - for each fill, print one line of TRADE <buy id> <sell id> <qty> <price>
     3) rest by pushing the Order into the buy/bid B-tree
     - insert the Order's id and pointer as key and value pair to the idOrderMap
     - if anything is left over, print REST <id> <qty> <price>
     */
}

template <typename Traits>
void stock_book<Traits>::sellLimit(const std::string& user, Order::id_type id, Order::size_type qty, double price, size_t num)
{
    //Pre-condition: id is a positive integer that have not been used; qty is a positive integer; price is a positive double
    assert((id > 0) && (qty > 0) && (price > 0.00));
    if(idOrderMap.find(id) != idOrderMap.end()) //if id existed
    {
        std::cout << "ERROR duplicate id " << id << std::endl;
        return;
    }
    
    //create Order object (heap creation)
    Order* current = new Order(id, price, qty, user);
    Order* copy = new Order(*current);
    
    //append Order copy in user's HISTORY to print "#<n> PLACE <id> SELL <qty> <price>"
    typename UserHistoryMap::iterator userHistoryMap_it = userHistoryMap.find(user);
    if(userHistoryMap_it != userHistoryMap.end()) //user is inserted and history (TailForwardList) has been created
    {
        //'s' means "PLACE...SELL"
        historyTag tag{'s', num};
        userHistoryMap_it->second->push_back(std::make_pair(copy, tag));
    }
    else
    {
        TailForwardList<ForwardList> * history = new TailForwardList<ForwardList>(); //heap creation
        historyTag tag{'s', num};
        history->push_back(std::make_pair(copy, tag));
        userHistoryMap_it = userHistoryMap.insert(std::make_pair(user, history))->first; //insert and update userHistoryMap_it to point to inserted
    }
    
    //match and fill from the best buy orders, any bid_price >= price
    while (!buyBook.empty() && (qty > 0)) //match until qty == 0 OR book ran out of matches
    {
        typename BuyBTree::iterator bestIt = buyBook.begin(); //from highest-to-lowest priced orders
        double bestPrice = bestIt->first; //dereference a map iterator yields a std::pair<Key, Value>
        if(bestPrice >= price) //if match
        {
            OrderList* bestOrders = bestIt->second; //get the list of best opposite orders
            if(bestOrders->empty()) //if list is empty
            {
                delete bestOrders; //deallocate list
                buyBook.erase(bestIt); //deallocate B-tree node
                continue;
            }
            
            Order* oldestOrderAtBest = bestOrders->front(); //get the oldest order
            Order::size_type filled = std::min(oldestOrderAtBest->get_shares(), qty); //get amount of filled shared
            
            //append #<m> FILL <id> <fill_qty> <trade_price> in buyer & seller's HISTORY
            //userHistoryMap_it points to a pair of the seller's name and History
            //'f' means "FILL..."
            Order* sellCopy = new Order(id, bestPrice, filled, user);
            historyTag tag{'f', num};
            userHistoryMap_it->second->push_back(std::make_pair(sellCopy, tag)); //append to seller's History
            
            Order::id_type buyID = oldestOrderAtBest->get_id();
            std::string buyer = oldestOrderAtBest->get_user();
            Order* buyCopy = new Order(buyID, bestPrice, filled, buyer);
            userHistoryMap_it = userHistoryMap.find(buyer);
            userHistoryMap_it->second->push_back(std::make_pair(buyCopy, tag)); //append to buyer's History
            
            //print one line of TRADE <buy id> <sell id> <qty> <price>
            std::cout << "TRADE " << buyID << " " << id << " " << filled << " " << bestPrice << std::endl;
            
            //fill both orders
            oldestOrderAtBest->reduceShares(filled);
            current->reduceShares(filled);
            
            if (oldestOrderAtBest->isFilled()) //if bid is completely filled
            {
                //pop that order from its list in buy/bid B-tree
                bestOrders->pop_front(); //remove from list
                delete oldestOrderAtBest; //deallocate object
                
                auto mapIt = idOrderMap.find(buyID);
                if (mapIt != idOrderMap.end())
                {
                    mapIt->second = nullptr; //to avoid idOrderMap returning a dangling pointer
                }
                
                if(bestOrders->empty()) //if list is now empty, remove the node from buy/bid B-tree
                {
                    delete bestOrders; //deallocate list
                    buyBook.erase(bestIt); //unlink and deallocate node
                }
            }
            
            userHistoryMap_it = userHistoryMap.find(user); //points to seller
            qty -= filled; //while-loop condition
        }
        else
        {
            break;
        }
    }
    
    //qty now holds the unfilled qty
    if(qty > 0)
    {
        current->setShares(qty);
        //rest by pushing the updated Order into the sell/ask B-tree
        typename SellBTree::iterator sellBook_it = sellBook.find(price);
        
        if(sellBook_it != sellBook.end()) //if this price level already exists at it
        {
            //TODO: here assumed push_back() deals with empty lists; B-tree never leaves nodes with deallocated list
            (sellBook_it->second)->push_back(current);
        }
        else
        {
            OrderList * temp = new OrderList; //heap creation
            temp->push_back(current); //push order to list
            sellBook.insert(std::make_pair(price, temp)); //insert to sell/ask B-tree
        }
        //insert the Order's id and pointer as key and value pair to the idOrderMap
        idOrderMap.insert(std::make_pair(id, current));
        //print REST <id> <qty> <price>
        std::cout << "REST " << id << " " << qty << " " << price << std::endl;
    }
    else if(qty == 0)
    {
        delete current;
        
        //insert the Order's id and nullptr to the idOrderMap (for duplicate ID check)
        idOrderMap.insert(std::make_pair(id, nullptr));
    }
    
    return;
    /* Post-condition of sellLimit():
     1) check inputs, id not existed, create Order object (heap creation)
     - append pointer to Order's copy(also heap creation), #<n> PLACE <id> SELL <qty> <price> in user's HISTORY
     - print ERROR duplicate id <id> if id existed
     2) match and fill from the best buy orders, as a limit order (any bid_price >= price)
     - front, reduce (or pop_front), until qty == 0 OR book ran out of matches
     - reduceShares(lowest(qty, ask_qty)) for both orders
     - if bid is completely filled, pop that order from buy/bid B-tree (remove from list, and remove the price level from B-tree if necessary)
     - append #<m> FILL <id> <fill_qty> <bid_price> in buyer & seller's HISTORY
     - for each fill, print one line of TRADE <buy id> <sell id> <qty> <price>
     3) rest by pushing the Order into the sell/ask B-tree
     - insert the Order's id and pointer as key and value pair to the idOrderMap
     - if anything is left over, print REST <id> <qty> <price>
     */
}

template <typename Traits>
void stock_book<Traits>::buyMarket(const std::string& user, Order::id_type id, Order::size_type qty, size_t num)
{
    //Pre-condition: id is a positive integer that have not been used; qty is a positive integer
    assert((id > 0) && (qty > 0));
    if(idOrderMap.find(id) != idOrderMap.end()) //if id existed
    {
        std::cout << "ERROR duplicate id " << id << std::endl;
        return;
    }
    
    //create Order object (heap creation)
    Order* current = new Order(id, 0.00, qty, user); //for historyPrint, price set to 0.00 means MKT
    Order* copy = new Order(*current);
    
    //append Order copy in user's HISTORY to print "#<n> PLACE <id> BUY <qty> MKT"
    typename UserHistoryMap::iterator userHistoryMap_it = userHistoryMap.find(user);
    if(userHistoryMap_it != userHistoryMap.end()) //user is inserted and history (TailForwardList) has been created
    {
        //'b' means "PLACE...BUY"
        historyTag tag{'b', num};
        userHistoryMap_it->second->push_back(std::make_pair(copy, tag));
    }
    else
    {
        TailForwardList<ForwardList> * history = new TailForwardList<ForwardList>(); //heap creation
        historyTag tag{'b', num};
        history->push_back(std::make_pair(copy, tag));
        userHistoryMap_it = userHistoryMap.insert(std::make_pair(user, history))->first; //insert and update userHistoryMap_it to point to inserted
    }
    
    //match and fill from the best sell orders, any price
    while (!sellBook.empty() && (qty > 0)) //match until qty == 0 OR book ran out of matches
    {
        typename SellBTree::iterator bestIt = sellBook.begin(); //from lowest-to-highest priced orders
        double bestPrice = bestIt->first; //dereference a map iterator yields a std::pair<Key, Value>
        OrderList* bestOrders = bestIt->second; //get the list of best opposite orders
        if(bestOrders->empty()) //if list is empty
        {
            delete bestOrders; //deallocate list
            sellBook.erase(bestIt); //deallocate B-tree node
            continue;
        }
        
        Order* oldestOrderAtBest = bestOrders->front(); //get the oldest order
        Order::size_type filled = std::min(oldestOrderAtBest->get_shares(), qty); //get amount of filled shared
        
        //append #<m> FILL <id> <fill_qty> <trade_price> in buyer & seller's HISTORY
        //userHistoryMap_it points to a pair of the buyer's name and History
        Order* buyCopy = new Order(id, bestPrice, filled, user);
        historyTag tag{'f', num};
        userHistoryMap_it->second->push_back(std::make_pair(buyCopy, tag)); //append to buyer's History
        
        Order::id_type sellID = oldestOrderAtBest->get_id();
        std::string seller = oldestOrderAtBest->get_user();
        Order* sellCopy = new Order(sellID, bestPrice, filled, seller);
        userHistoryMap_it = userHistoryMap.find(seller);
        userHistoryMap_it->second->push_back(std::make_pair(sellCopy, tag)); //append to seller's History
        
        //print one line of TRADE <buy id> <sell id> <qty> <price>
        std::cout << "TRADE " << id << " " << sellID << " " << filled << " " << bestPrice << std::endl;
        
        //fill both orders
        oldestOrderAtBest->reduceShares(filled);
        current->reduceShares(filled);
        
        if (oldestOrderAtBest->isFilled()) //if ask is completely filled
        {
            //pop that order from its list in sell/ask B-tree
            bestOrders->pop_front(); //remove from list
            delete oldestOrderAtBest; //deallocate object
            
            auto mapIt = idOrderMap.find(sellID);
            if (mapIt != idOrderMap.end())
            {
                mapIt->second = nullptr; //to avoid idOrderMap returning a dangling pointer
            }
            
            if(bestOrders->empty()) //if list is now empty, remove the node from sell/ask B-tree
            {
                delete bestOrders; //deallocate list
                sellBook.erase(bestIt); //unlink and deallocate node
            }
        }
        
        userHistoryMap_it = userHistoryMap.find(user); //points to buyer
        qty -= filled; //while-loop condition
    }
    
    //qty now holds the unfilled qty
    current->setShares(qty);
    
    //append #<n> UNFILLED <id> <qty> in user's HISTORY
    //'u' means "UNFILLED..."
    userHistoryMap_it = userHistoryMap.find(user);
    historyTag tag{'u', num};
    userHistoryMap_it->second->push_back(std::make_pair(current, tag));
    
    //print UNFILLED <id> <unfilled_qty>
    std::cout << "UNFILLED " << id << " " << qty << std::endl;
    
    //insert the Order's id and nullptr to the idOrderMap (for duplicate ID check)
    idOrderMap.insert(std::make_pair(id, nullptr));
    
    //discard the unfilled
    return;
    /* Post-condition of buyMarket():
     1) check inputs, id not existed, create Order object (heap creation)
     - append Order copy, #<n> PLACE <id> BUY <qty> MKT in user's HISTORY
     - for historyPrint, price set to 0.00 means MKT
     - limit orders can't set price to 0.00
     - print ERROR duplicate id <id> if id existed
     2) match and fill from lowest-to-highest priced orders in sell/ask B-tree, as a market order
     - front, reduce (or pop_front), until qty == 0 OR reaching end of book
     - reduceShares(lowest(qty, ask_qty)) for both orders
     - if ask is completely filled, pop that order from sell/ask B-tree (remove from list, and remove the price level from B-tree if necessary)
     - append #<m> FILL <id> <fill_qty> <trade_price> in buyer & seller's HISTORY
     - for each fill, print one line of TRADE <buy id> <sell id> <qty> <price>
     3) record then discard the unfilled
     - append #<n> UNFILLED <id> <qty> in user's HISTORY
     - if anything is left over, print UNFILLED <id> <unfilled_qty>
     - insert the Order's id and nullptr to the idOrderMap (for duplicate ID check)
     */
}

template <typename Traits>
void stock_book<Traits>::sellMarket(const std::string& user, Order::id_type id, Order::size_type qty, size_t num)
{
    //Pre-condition: id is a positive integer that have not been used; qty is a positive integer
    assert((id > 0) && (qty > 0));
    if(idOrderMap.find(id) != idOrderMap.end()) //if id existed
    {
        std::cout << "ERROR duplicate id " << id << std::endl;
        return;
    }
    
    //create Order object (heap creation)
    Order* current = new Order(id, 0.00, qty, user); //for historyPrint, price set to 0.00 means MKT
    Order* copy = new Order(*current);
    
    //append Order copy in user's HISTORY to print "#<n> PLACE <id> SELL <qty> MKT"
    typename UserHistoryMap::iterator userHistoryMap_it = userHistoryMap.find(user);
    if(userHistoryMap_it != userHistoryMap.end()) //user is inserted and history (TailForwardList) has been created
    {
        //'s' means "PLACE...SELL"
        historyTag tag{'s', num};
        userHistoryMap_it->second->push_back(std::make_pair(copy, tag));
    }
    else
    {
        TailForwardList<ForwardList> * history = new TailForwardList<ForwardList>(); //heap creation
        historyTag tag{'s', num};
        history->push_back(std::make_pair(copy, tag));
        userHistoryMap_it = userHistoryMap.insert(std::make_pair(user, history))->first; //insert and update userHistoryMap_it to point to inserted
    }
    
    //match and fill from the best buy orders, any price
    while (!buyBook.empty() && (qty > 0)) //match until qty == 0 OR book ran out of matches
    {
        typename BuyBTree::iterator bestIt = buyBook.begin(); //from highest-to-lowest priced orders
        double bestPrice = bestIt->first; //dereference a map iterator yields a std::pair<Key, Value>
        OrderList* bestOrders = bestIt->second; //get the list of best opposite orders
        if(bestOrders->empty()) //if list is empty
        {
            delete bestOrders; //deallocate list
            buyBook.erase(bestIt); //deallocate B-tree node
            continue;
        }
        
        Order* oldestOrderAtBest = bestOrders->front(); //get the oldest order
        Order::size_type filled = std::min(oldestOrderAtBest->get_shares(), qty); //get amount of filled shared
        
        //append #<m> FILL <id> <fill_qty> <trade_price> in buyer & seller's HISTORY
        //userHistoryMap_it points to a pair of the seller's name and History
        Order* sellCopy = new Order(id, bestPrice, filled, user);
        historyTag tag{'f', num};
        userHistoryMap_it->second->push_back(std::make_pair(sellCopy, tag)); //append to seller's History
        
        Order::id_type buyID = oldestOrderAtBest->get_id();
        std::string buyer = oldestOrderAtBest->get_user();
        Order* buyCopy = new Order(buyID, bestPrice, filled, buyer);
        userHistoryMap_it = userHistoryMap.find(buyer);
        userHistoryMap_it->second->push_back(std::make_pair(buyCopy, tag)); //append to buyer's History
        
        //print one line of TRADE <buy id> <sell id> <qty> <price>
        std::cout << "TRADE " << buyID << " " << id << " " << filled << " " << bestPrice << std::endl;
        
        //fill both orders
        oldestOrderAtBest->reduceShares(filled);
        current->reduceShares(filled);
        
        if (oldestOrderAtBest->isFilled()) //if bid is completely filled
        {
            //pop that order from its list in buy/bid B-tree
            bestOrders->pop_front(); //remove from list
            delete oldestOrderAtBest; //deallocate object
            
            auto mapIt = idOrderMap.find(buyID);
            if (mapIt != idOrderMap.end())
            {
                mapIt->second = nullptr; //to avoid idOrderMap returning a dangling pointer
            }
            
            if(bestOrders->empty()) //if list is now empty, remove the node from buy/bid B-tree
            {
                delete bestOrders; //deallocate list
                buyBook.erase(bestIt); //unlink and deallocate node
            }
        }
        
        userHistoryMap_it = userHistoryMap.find(user); //points to seller
        qty -= filled; //while-loop condition
    }
    
    //qty now holds the unfilled qty
    current->setShares(qty);
    
    //append #<n> UNFILLED <id> <qty> in user's HISTORY
    //'u' means "UNFILLED..."
    userHistoryMap_it = userHistoryMap.find(user);
    historyTag tag{'u', num};
    userHistoryMap_it->second->push_back(std::make_pair(current, tag));
    
    //print UNFILLED <id> <unfilled_qty>
    std::cout << "UNFILLED " << id << " " << qty << std::endl;
    
    //insert the Order's id and nullptr to the idOrderMap (for duplicate ID check)
    idOrderMap.insert(std::make_pair(id, nullptr));
    
    //discard the unfilled
    return;
    /* Post-condition of sellMarket():
     1) check inputs, id not existed, create Order object (heap creation)
     - append Order copy, #<n> PLACE <id> SELL <qty> MKT in user's HISTORY
     - for historyPrint, price set to 0.00 means MKT
     - limit orders can't set price to 0.00
     - print ERROR duplicate id <id> if id existed
     2) match and fill from highest-to-lowest priced orders in buy/bid B-tree, as a market order
     - front, reduce (or pop_front), until qty == 0 OR reaching end of book
     - reduceShares(lowest(qty, ask_qty)) for both orders
     - if bid is completely filled, pop that order from buy/bid B-tree (remove from list, and remove the price level from B-tree if necessary)
     - append #<m> FILL <id> <fill_qty> <bid_price> in buyer & seller's HISTORY
     - for each fill, print one line of TRADE <buy id> <sell id> <qty> <price>
     3) record then discard the unfilled
     - append #<n> UNFILLED <id> <qty> in user's HISTORY
     - if anything is left over, print UNFILLED <id> <unfilled_qty>
     - insert the Order's id and nullptr to the idOrderMap (for duplicate ID check)
     */
}

template <typename Traits>
void stock_book<Traits>::cancelOrder(Order::id_type id)
{
    //using map, retrieve Order* with input id
    typename IdOrderMap::iterator idOrderMap_it = idOrderMap.find(id);
    if(idOrderMap_it == idOrderMap.end()) //if not found
    {
        std::cout << "ERROR unknown order " << id << std::endl;
        return;
    }
    else
    {
        Order* orderPtr = idOrderMap_it->second;
        if(orderPtr == nullptr) //that id belonged to a market/canceled/filled order
        {
            return;
        }
        
        //locate node with (Order->getPrice) in both Buy/Bid and Sell/Ask B-tree
        double targetPrice = orderPtr->get_price();
        
        //first, search in sellBook
        typename SellBTree::iterator sellFind = sellBook.find(targetPrice);
        if(sellFind != sellBook.end()) //if found in sellBook
        {
            //print CANCELLED <id> <remaining_qty>
            std::cout << "CANCELLED " << id << " " << orderPtr->get_shares() << std::endl;
            
            //pop the order from sell/ask B-tree (remove from list, and remove the price level from B-tree if necessary)
            sellFind->second->remove(orderPtr); //unlink/remove from list
            delete orderPtr; //deallocate object
            idOrderMap_it->second = nullptr; // avoid dangling pointer in idOrderMap
            
            if(sellFind->second->empty()) //if list is now empty, remove the node from sell/ask B-tree
            {
                delete sellFind->second; //deallocate list
                sellBook.erase(sellFind); //unlink and deallocate node
            }
        }
        else
        {
            //search in buyBook
            typename BuyBTree::iterator buyFind = buyBook.find(targetPrice);
            if(buyFind != buyBook.end()) //if found in buyBook
            {
                //print CANCELLED <id> <remaining_qty>
                std::cout << "CANCELLED " << id << " " << orderPtr->get_shares() << std::endl;
                
                //pop the order from buy/bid B-tree (remove from list, and remove the price level from B-tree if necessary)
                buyFind->second->remove(orderPtr); //unlink/remove from list
                delete orderPtr; //deallocate object
                idOrderMap_it->second = nullptr; // avoid dangling pointer in idOrderMap
                
                if(buyFind->second->empty()) //if list is now empty, remove the node from buy/bid B-tree
                {
                    delete buyFind->second; //deallocate list
                    buyBook.erase(buyFind); //unlink and deallocate node
                }
            }
            else
            {
                //not found in both books, print ERROR unknown order <id>
                std::cout << "ERROR unknown order " << id << std::endl;
            }
        }
    }
    
    return;
    /* Post-condition of cancelOrder():
     1) using map, retrieve Order* with input id
     - if not found (id doesn't exist), print ERROR unknown order <id>
     - class invariant: if Order* == nullptr, that id belonged to a market/canceled/filled order
     2) locate node with (Order->getPrice) in both Buy/Bid and Sell/Ask B-tree
     - if found, pop that order from B-tree (remove from list, and remove the price level from B-tree if necessary), and print CANCELLED <id> <remaining_qty>
     - if not found in both books, print ERROR unknown order <id>
     */
}

template <typename Traits>
void stock_book<Traits>::bookPrint(size_t depth) const
{
    //lists up to depth from lowest to highest SELL/ASK price levels
    size_t sellDepth = (sellBook.size() < depth)? sellBook.size() : depth;
    typename SellBTree::const_iterator sellBook_it = sellBook.begin();
    for(size_t i=0; i<sellDepth; i++)
    {
        double price_level = sellBook_it->first;
        OrderList * list = sellBook_it->second;
        size_t order_count = list->size();
        size_t sum_qty = 0;
        auto temp = list->begin();
        for(size_t j=0; j<order_count; j++)
        {
            sum_qty += temp->get_shares();
            temp++;
        }
        //for each price level, prints <price> <total_remaining_qty> <active_order_count>
        std::cout << price_level << " " << sum_qty << " " << order_count << std::endl;
        sellBook_it++;
    }
    //lists up to depth from highest to lowest BUY/BID price levels
    size_t buyDepth = (sellBook.size() < depth)? sellBook.size() : depth;
    typename BuyBTree::const_iterator buyBook_it = buyBook.begin();
    for(size_t i=0; i<buyDepth; i++)
    {
        double price_level = buyBook_it->first;
        OrderList * list = buyBook_it->second;
        size_t order_count = list->size();
        size_t sum_qty = 0;
        auto temp = list->begin();
        for(size_t j=0; j<order_count; j++)
        {
            sum_qty += temp->get_shares();
            temp++;
        }
        //for each price level, prints <price> <total_remaining_qty> <active_order_count>
        std::cout << price_level << " " << sum_qty << " " << order_count << std::endl;
        buyBook_it++;
    }
    std::cout << "END" << std::endl;
    
    return;
    /* Post-condition of bookPrint():
     1)lists up to depth SELL/ASK price levels from lowest to highest, then up to depth BUY/BID price levels from highest to lowest
     - for each price level, print <price> <total_remaining_qty> <active_order_count>
     - print "END" in last line
     */
}

template <typename Traits>
void stock_book<Traits>::historyPrint(const std::string& user, size_t n) const
{
    //using userHistoryMap, retrieve user's history list (TailForwardList) with user's name
    auto userHistoryMap_it = userHistoryMap.find(user);
    if(userHistoryMap_it != userHistoryMap.end()) //if found
    {
        //using forward iterator of TailForwardList, print while iterates forward for nth events in user's history
        TailForwardList<ForwardList>* TailForwardList_ptr = userHistoryMap_it->second;
        auto it = TailForwardList_ptr->begin();
        while((it != TailForwardList_ptr->end()) && (n > 0))
        {
            Order* temp = it->first;
            if(it->second.action == 'b')
            {
                double price = temp->get_price();
                
                if(price != 0.00)
                {
                    //print #<n> PLACE <id> BUY <qty> <price>
                    std::cout << "#" << it->second.num << " PLACE " << temp->get_id() << " BUY " << temp->get_shares() << " " << price << std::endl;
                }
                else
                {
                    //print #<n> PLACE <id> BUY <qty> MKT
                    std::cout << "#" << it->second.num << " PLACE " << temp->get_id() << " BUY " << temp->get_shares() << " MKT" << std::endl;
                }
            }
            else if(it->second.action == 's')
            {
                double price = temp->get_price();
                
                if(price != 0.00)
                {
                    //print #<n> PLACE <id> SELL <qty> <price>
                    std::cout << "#" << it->second.num << " PLACE " << temp->get_id() << " SELL " << temp->get_shares() << " " << price << std::endl;
                }
                else
                {
                    //print #<n> PLACE <id> SELL <qty> MKT
                    std::cout << "#" << it->second.num << " PLACE " << temp->get_id() << " SELL " << temp->get_shares() << " MKT" << std::endl;
                }
            }
            else if(it->second.action == 'f')
            {
                //print #<n> FILL <id> <fill_qty> <trade_price>
                std::cout << "#" << it->second.num << " FILL " << temp->get_id() << " " << temp->get_shares() << " " << temp->get_price() << std::endl;
            }
            else if(it->second.action == 'u')
            {
                //print #<n> UNFILLED <id> <qty>
                std::cout << "#" << it->second.num << " UNFILLED " << temp->get_id() << " " << temp->get_shares() << std::endl;
            }
            ++it;
            --n;
        }
    }
    //if not found, history doesn't exists, print "END"
    std::cout << "END" << std::endl;
    
    return;
    /* Post-condition of bookPrint(user, n):
     1) using userHistoryMap, retrieve user's history list (TailForwardList) with user's name
     - if not found, history doesn't exists, print "END"
     2) using forward iterator of TailForwardList, print while iterates forward for nth events in user's history
     - print templates:
     - #<n> PLACE <id> BUY <qty> <price> //'b'
     - #<n> PLACE <id> SELL <qty> <price> //'s'
     - #<n> PLACE <id> BUY <qty> MKT //second == 'b' && price == 0.00
     - #<n> PLACE <id> SELL <qty> MKT //second == 's' && price == 0.00
     - #<m> FILL <id> <fill_qty> <trade_price> //'f'
     - #<n> UNFILLED <id> <qty> //'u'
     - print "END" in last line
     */
}

template <typename Traits>
void stock_book<Traits>::historyPrint(const std::string& user) const
{
    //using userHistoryMap, retrieve user's history list (TailForwardList) with user's name
    auto userHistoryMap_it = userHistoryMap.find(user);
    if(userHistoryMap_it != userHistoryMap.end()) //if found
    {
        //using forward iterator of TailForwardList, print while iterates forward for nth events in user's history
        TailForwardList<ForwardList>* TailForwardList_ptr = userHistoryMap_it->second;
        for(auto it = TailForwardList_ptr->begin(); it != TailForwardList_ptr->end(); ++it)
        {
            Order* temp = it->first;
            if(it->second.action == 'b')
            {
                double price = temp->get_price();
                
                if(price != 0.00)
                {
                    //print #<n> PLACE <id> BUY <qty> <price>
                    std::cout << "#" << it->second.num << " PLACE " << temp->get_id() << " BUY " << temp->get_shares() << " "  << price << std::endl;
                }
                else
                {
                    //print #<n> PLACE <id> BUY <qty> MKT
                    std::cout << "#" << it->second.num << " PLACE " << temp->get_id() << " BUY " << temp->get_shares() << " MKT" << std::endl;
                }
            }
            else if(it->second.action == 's')
            {
                double price = temp->get_price();
                
                if(price != 0.00)
                {
                    //print #<n> PLACE <id> SELL <qty> <price>
                    std::cout << "#" << it->second.num << " PLACE " << temp->get_id() << " SELL " << temp->get_shares() << " " << price << std::endl;
                }
                else
                {
                    //print #<n> PLACE <id> SELL <qty> MKT
                    std::cout << "#" << it->second.num << " PLACE " << temp->get_id() << " SELL " << temp->get_shares() << " MKT" << std::endl;
                }
            }
            else if(it->second.action == 'f')
            {
                //print #<n> FILL <id> <fill_qty> <trade_price>
                std::cout << "#" << it->second.num << " FILL " << temp->get_id() << " " << temp->get_shares() << " " << temp->get_price() << std::endl;
            }
            else if(it->second.action == 'u')
            {
                //print #<n> UNFILLED <id> <qty>
                std::cout << "#" << it->second.num << " UNFILLED " << temp->get_id() << " " << temp->get_shares() << std::endl;
            }
        }
    }
    //if not found, history doesn't exists, print "END"
    std::cout << "END" << std::endl;
    
    return;
    /* Post-condition of bookPrint(user, n):
     1) using userHistoryMap, retrieve user's history list (TailForwardList) with user's name
     - if not found, history doesn't exists, print "END"
     2) using forward iterator of TailForwardList, print while iterates forward till end of user's history
     - print templates:
     - #<n> PLACE <id> BUY <qty> <price> //'b'
     - #<n> PLACE <id> SELL <qty> <price> //'s'
     - #<n> PLACE <id> BUY <qty> MKT //second == 'b' && price == 0.00
     - #<n> PLACE <id> SELL <qty> MKT //second == 's' && price == 0.00
     - #<m> FILL <id> <fill_qty> <trade_price> //'f'
     - #<n> UNFILLED <id> <qty> //'u'
     - print "END" in last line
     */
}

template <typename ForwardList>
void TailForwardList<ForwardList>::push_front(const std::pair<Order*, historyTag>& val)
{
    list.push_front(val);
    ++count;
    if (!has_last)
    {
        last = list.begin();
        has_last = true;
    }
}

template <typename ForwardList>
void TailForwardList<ForwardList>::push_back(const std::pair<Order*, historyTag>& val) //O(1) with a tail pointer
{
    if (!has_last)
    {
        list.push_front(val);
        last = list.begin();
        has_last = true;
    }
    else
    {
        last = list.insert_after(last, val);
    }
    ++count;
}

template <typename ForwardList>
typename TailForwardList<ForwardList>::iterator TailForwardList<ForwardList>::begin()
{
    return list.begin();
}

template <typename ForwardList>
typename TailForwardList<ForwardList>::iterator TailForwardList<ForwardList>::end()
{
    return list.end();
}

template <typename ForwardList>
typename TailForwardList<ForwardList>::const_iterator TailForwardList<ForwardList>::begin() const
{
    return list.begin();
}

template <typename ForwardList>
typename TailForwardList<ForwardList>::const_iterator TailForwardList<ForwardList>::end() const
{
    return list.end();
}

template <typename ForwardList>
bool TailForwardList<ForwardList>::empty() const
{
    return list.empty();
}


}


#endif /* stock_book_h */
