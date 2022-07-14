#include "htmlbook.h"

#include <iostream>

using namespace htmlbook;

int main()
{
    htmlbook::Book book(htmlbook::PageSize::A5);
    book.load("<a class='a' id='a'> Hello World </a>");
    book.save("hello.pdf");

    std::cout << book << std::endl;
    return 0;
}
