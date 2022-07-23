#include "htmlbook.h"

#include <iostream>

int main()
{
    htmlbook::Book book(htmlbook::PageSize::A5);
    book.load("<b> Hello World </b>");
    book.save("hello.pdf");

    std::cout << book << std::endl;
    return 0;
}
