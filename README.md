lua-xpath
=========


A Lua XPath library based on libxml2. It provides easy-to-use APIs inspired
by [Scrapy](http://www.scrapy.org/)'s selector classes.



Examples
--------

    local xpath = require("xpath")



    -- load data from file
    selector = xpath.loadfile(html)
    -- load data from string
    selector = xpath.loads("<html></html>")


    ------------------ OOP style ----------------------------

    items = selector:xpath("//div[@name='shit']")

    for k, v in ipairs(items) do
        print(v:extract())
    end


    ------------------ general lua style --------------------

    items = xpath.select(selector, "//div[@name='shit']")

    for k, v in ipairs(items) do
        print(xpath.extract(v))
    end



Installation
------------

* To install from source

        ./configure --libdir=/usr/local/lib/lua/5.1 --datadir=/usr/local/share/lua/5.1
        make
        make install

* To install from source with luarocks

        luarocks build rockspec/lua-xpath-<version>.rockspec






