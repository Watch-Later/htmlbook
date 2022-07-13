#include "htmlbook.h"

#include "cssparser.h"
#include "pdfdocument.h"
#include "cssstylebuilder.h"
#include "url.h"

#include <iostream>
#include <sstream>
#include <limits>

using namespace htmlbook;

int main()
{
    htmlbook::Url url("httpr://site1.com:80/a/b/c/");
    auto completeurl = url.complete("../pic1.gif#hello/g");
    std::cout << "Url : " << url << std::endl;
    std::cout << "CompleteUrl : " << completeurl << std::endl;

    htmlbook::Book book(htmlbook::PageSize::A5);
    book.load("<a class='a' id='a'> Hello World </a>");
    book.save("hello.pdf");

    std::cout << book << std::endl;
    return 0;
}
