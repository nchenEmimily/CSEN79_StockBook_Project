//
//  order.h
//  CSEN79_StockBook_Project
//
//  Created by Emily Chen on 2026/11/18.
//

#ifndef order_h
#define order_h

#include <stdio.h>
#include <cstddef>
#include <iostream>
#include <string>

namespace CSEN79_StockBook
{

//each Order object represents a single limit/market order(with its id, price, and share quantity)

class Order
{
public:
    typedef size_t id_type;
    typedef size_t size_type;
    
    //constructor
    Order(id_type init_id, double init_price, size_type init_shares, std::string init_user);
    //copy constructor
    Order(const Order& s);
    
    //accessors
    id_type get_id() const;
    double get_price() const;
    size_type get_shares() const;
    std::string get_user() const;
    bool isFilled() const;
    
    //mutators for partial/full fills
    void setShares(size_type s);
    size_type reduceShares(size_type filled);

    //useful for debugging / logging
    friend std::ostream& operator<<(std::ostream& os, const Order& order);

private:
    id_type id;
    double price;
    size_type shares;
    std::string user;
};

}
#endif /* order_h */
