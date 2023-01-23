target_sources(htmlbook
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/layout/boxstyle.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/layout/box.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/layout/boxview.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/layout/linebox.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/layout/textbox.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/layout/inlinebox.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/layout/replacedbox.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/layout/blockbox.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/layout/flexiblebox.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/layout/listitembox.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/layout/tablebox.cpp"

    "${CMAKE_CURRENT_LIST_DIR}/cssparser.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/cssrule.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/csstokenizer.cpp"

    "${CMAKE_CURRENT_LIST_DIR}/htmldocument.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/htmlentityparser.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/htmlparser.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/htmltokenizer.cpp"

    "${CMAKE_CURRENT_LIST_DIR}/counters.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/document.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/globalstring.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/htmlbook.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/resource.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/url.cpp"
)

target_include_directories(htmlbook
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/layout"
)
