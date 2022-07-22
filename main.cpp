#include "htmlbook.h"

#include <iostream>

int main()
{
    htmlbook::Book book(htmlbook::PageSize::A5);
    book.loadHtml("<b> Hello World </b>", "about:blank");
    book.save("hello.pdf");

    std::cout << book << std::endl;
    return 0;
}
