lua-xpath
=========

A Lua XPath library based on libxml2. It provides easy-to-use APIs inspired
by [Scrapy](http://www.scrapy.org/)'s selector classes.


example
-------

    local xpath = require("xpath")



    -- load data from file
    tree = xpath.loadfile(html)
    -- load data from string
    tree = xpath.loads("<html></html>")


    ------------------ OOP style ----------------------------

    items = tree:select("//div[@name='shit']")

    for k, v in ipairs(items) do
        print(v:extract())
    end


    ------------------ general lua style --------------------

    items = xpath.select(tree, "//div[@name='shit']")

    for k, v in ipairs(items) do
        print(xpath.extract(v))
    end



