//
//  main.cpp
//  CSEN79_StockBook_Project
//
//  Created by Emily Chen on 2026/11/18.
//

#include "order.h"
#include "stock_book.h"

#include <iostream>
#include <sstream>
#include <fstream> //for file (read & write) operations
#include <cstddef>
#include <string>

using namespace CSEN79_StockBook;

//stock_book<HandBuiltTraits> book;
stock_book<StdTraits> book;

int main(int argc, const char * argv[])
{
    std::ifstream fileStream; //(from <fstream>, file input)
    //default: taking input from standard line
    std::istream* input = &std::cin; //(from <iostream>, standard input)
    
    if (argc >= 2) //if at least 1 argument passed
    {
        fileStream.open(argv[1]); //open file
        if (!fileStream) //if fail open
        {
            std::cerr << "Error: Could not open file '" << argv[1] << "'.\n";
            return 1;
        }
        //if success open, taking input from file instead
        input = &fileStream;
    }
    
    //general logic to read and execute commands
    std::string line;
    size_t command_num = 0;
    while (std::getline(*input, line)) //read by line
    {
        std::istringstream iss(line); //splits text at whitespace while converting it to given data type
        std::string cmd;
        
        if (!(iss >> cmd)) //(iss >> cmd) returns false only when the line is empty (because cmd is string, no type mismatch issues)
        {
            continue; //read the next line
        }
        
        if (cmd == "BUY")
        {
            std::string user;
            Order::id_type id;
            Order::size_type qty;
            double price;
            if ((iss >> user) && (iss >> id) && (iss >> qty) && (iss >> price)) //BUY <user> <id> <qty> <price>
            {
                ++command_num;
                //call buyLimit()
                book.buyLimit(user, id, qty, price, command_num);
            }
            else
            {
                std::cerr << "Error: invalid item(s) in command given. Command template: BUY <user> <id> <qty> <price>\n"; //cerr ensures printing error message
            }
        }
        else if (cmd == "SELL")
        {
            std::string user;
            Order::id_type id;
            Order::size_type qty;
            double price;
            if ((iss >> user) && (iss >> id) && (iss >> qty) && (iss >> price)) //SELL <user> <id> <qty> <price>
            {
                ++command_num;
                //call sellLimit()
                book.sellLimit(user, id, qty, price, command_num);
            }
            else
            {
                std::cerr << "Error: invalid item(s) in command given. Command template: SELL <user> <id> <qty> <price>\n";
            }
        }
        else if (cmd == "MARKET")
        {
            iss >> cmd;
            
            std::string user;
            Order::id_type id;
            Order::size_type qty;
            if (cmd == "BUY")
            {
                if ((iss >> user) && (iss >> id) && (iss >> qty)) //MARKET BUY <user> <id> <qty>
                {
                    ++command_num;
                    //call buyMarket()
                    book.buyMarket(user, id, qty, command_num);
                }
                else
                {
                    std::cerr << "Error: invalid item(s) in command given. Command template: MARKET BUY <user> <id> <qty>\n";
                }
            }
            else if (cmd == "SELL")
            {
                if ((iss >> user) && (iss >> id) && (iss >> qty)) //MARKET SELL <user> <id> <qty>
                {
                    ++command_num;
                    //call sellMarket()
                    book.sellMarket(user, id, qty, command_num);
                }
                else
                {
                    std::cerr << "Error: invalid item(s) in command given. Command template: MARKET SELL <user> <id> <qty>\n";
                }
            }
            else
            {
                std::cerr << "Error: invalid item(s) in command given. Command template: MARKET BUY/SELL <user> <id> <qty>\n";
            }
        }
        else if (cmd == "CANCEL")
        {
            Order::id_type id;
            if(iss >> id) //CANCEL <id>
            {
                ++command_num;
                //call cancelOrder()
                book.cancelOrder(id);
            }
            else
            {
                std::cerr << "Error: invalid item(s) in command given. Command template: CANCEL <id>\n";
            }
        }
        else if (cmd == "BOOK") //BOOK [depth]
        {
            ++command_num;
            size_t depth = 5;
            iss >> depth;
            book.bookPrint(depth);
        }
        else if(cmd == "HISTORY")
        {
            std::string user;
            if((iss >> user)) //HISTORY <user> [n]
            {
                ++command_num;
                size_t n;
                if(iss >> n)
                {
                    book.historyPrint(user, n);
                }
                else
                {
                    book.historyPrint(user);
                }
            }
            else
            {
                std::cerr << "Error: invalid item(s) in command given. Command template: HISTORY <user> [n]\n";
            }
        }
        else if(cmd == "STATS")
        {
            //TODO: Prints our custom diagnostic/benchmark counters
            //every line starts with "STATS"
        }
        else
        {
            std::cout << "Unknown command: " << cmd << '\n';
        }
    }
    
    //TODO: call a function and loop to free all the Order objects pointed by pointers in the list history;
    
    
    return 0;
}
