//
//  order.cpp
//  CSEN79_StockBook_Project
//
//  Created by Emily Chen on 2026/11/18.
//

#include "order.h"
#include <cassert>

namespace CSEN79_StockBook
{

Order::Order(id_type init_id, double init_price, size_type init_shares, std::string init_user)
{
    //Precondition: price must be a positive double (price = 0.00 for market orders), shares must be a positive integer, user name not empty
    assert((init_price >= 0.00) && (init_shares > 0) && (!init_user.empty()));
    
    id = init_id;
    price = init_price;
    shares = init_shares;
    user = init_user;
}

Order::Order(const Order& s)
{
    id = s.get_id();
    price = s.get_price();
    shares = s.get_shares();
    user = s.get_user();
}

Order::id_type Order::get_id() const
{
    return id;
}

double Order::get_price() const
{
    return price;
}

Order::size_type Order::get_shares() const
{
    return shares;
}

std::string Order::get_user() const
{
    return user;
}

void Order::setShares(Order::size_type s)
{
    shares = s;
}

Order::size_type Order::reduceShares(Order::size_type filled)
{
    //Precondition: filled is valid as a positive number
    assert(filled > 0);
    
    if (filled > shares) //if filled is larger than the shares of this order
    {
        //set shares of this order as 0
        size_type unfilled = filled - shares;
        shares = 0;
        return unfilled; //returns the unfilled share quantity by this order (unfilled/resting)
    }
    else
    {
        //updates share
        shares -= filled;
        return 0;
    }
    
    //Postcondition: returns the unfilled/resting share quantity after reducing filled shares from this order;
    // returns 0 if this order's shares satisfies the reduced amount (>=);
    // otherwise, returns the remaining unfilled share.
}

bool Order::isFilled() const
{
    //return true if this order is filled
    return (shares == 0);
}

std::ostream& operator<<(std::ostream& os, const Order& order)
{
    // TODO: Format order details for logs/debugging (e.g., name [ID=1 @ $100 x 50]).
    os << order.user << " [ID=" << order.id << " @ $" << order.price << " x " << order.shares << "]";
    return os;
}

}
