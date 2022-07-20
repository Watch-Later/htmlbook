#include "htmlbook.h"
#include "cssstylesheet.h"

#include <iostream>

using namespace htmlbook;

RefPtr<CSSValue> f() {
    CSSValueList values;
    values.push_back(CSSFunctionValue::create(CSSValueID::ListItem, CSSColorValue::create(0)));
    return CSSListValue::create(std::move(values));
}

int main()
{
    auto g = f();
//    htmlbook::Book book(htmlbook::PageSize::A5);
//    book.setUserStyleSheet("@font-face { font-family : Hello; src : local(Arial) format('truetype')}");
//    book.load("<a class='a' id='a'> Hello World </a>");
//    book.save("hello.pdf");

//    std::cout << book << std::endl;
    std::cout << "Hello World" << std::endl;
    return 0;
}
