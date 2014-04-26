package.cpath = "/home/duhoobo/prj/amateur/lua-xpath/src/?.so;" .. package.cpath

local xpath = require("xpath")

xml = [[
<html>
<head>
</head>
<body>
    <name color="red">alec</name>
    <name color="green">bing</name>
    <sex>male</sex>
</body>
</html>]]


function test_xpath()
    print "--- enter test_xpath ---\n"

    local sel, err = xpath.loads(xml)

    if not sel then
        print(sel, err)
        return
    end

    print(sel)

    local ls, err = sel:xpath("//body/name/@color")

    if not ls then
        print(ls, err)
        return
    end

    for k, v in pairs(ls) do
        print(k, v, v:extract())
    end

    print "\n--- leave test_xpath ---"
end


function test_relative_xpath()
    print "--- enter test_relative_xpath ---\n"

    local sel, err = xpath.loads(xml)

    if not sel then
        print(sel, err)
        return
    end
    

    local ls, err = sel:xpath("//body")
    if not ls then 
        print(ls, err)
        return
    end

    for k, v in pairs(ls) do
        local ls, err = v:xpath("./name/text()")

        if not ls then
            print(ls, err)
            return
        end

        for i, j in pairs(ls) do
            print (i, j)
        end
    end
        

    print "\n--- leave test_relative_xpath ---"
end


function test_re()
    print "--- enter test_re ---\n"

    local sel, err = xpath.loads(xml)

    if not sel then
        print(sel, err)
        return
    end

    print(sel)

    local ls, err = sel:re("//body/name/@color")

    if not ls then
        print(ls, err)
        return
    end

    print "\n--- leave test_re ---"
end


function test_css()
    print "--- enter test_css ---\n"

    local sel, err = xpath.loads(xml)

    if not sel then
        print(sel, err)
        return
    end

    print(sel)

    local ls, err = sel:css("//body/name/@color")

    if not ls then
        print(ls, err)
        return
    end

    print "\n--- leave test_css ---"
end


function test_tostring()
    print "--- enter test_tostring ---\n"

    print(xpath)

    local sel, err = xpath.loads(xml)
    print(sel, err)

    print "\n--- leave test_tostring ---"
end


function test_extract()
    print "-- enter test_extract ---\n"

    local sel, err = xpath.loads(xml)
    if not sel then
        print(sel, err)
        return nil
    end

    -- text
    local ls, err = sel:xpath("/html/body/name/text()")
    if not ls then
        print(sel, err)
        return nil
    end

    for k, v in pairs(ls) do
        print(k, v, v:extract())
    end

    -- attribute
    local ls, err = sel:xpath("/html/body/name/@color")
    if not ls then
        print(sel, err)
        return nil
    end

    for k, v in pairs(ls) do
        print(k, v, v:extract())
    end

    -- element
    local ls, err = sel:xpath("/html/body/name")
    if not ls then
        print(sel, err)
        return nil
    end

    for k, v in pairs(ls) do
        print(k, v, v:extract())
    end

    print "\n--- leave test_extract ---"
end


function test_error() 
    print "--- enter test_error ---\n"

    local xml = [[
    <html>
      <head></head> 
      <body> 
        <name>alec</name>
        <name>yang</name>
      </body>
    </html>]]


    local sel, err = xpath.loads(xml)
    if not sel then
        print(sel, err)
        return nil
    end

    local ls, err = xpath.xpath(sel, "/html/body/name")
    if not ls then
        print(ls, err)
        return nil
    end

    for k, v in pairs(ls) do
        print(k, xpath.extract(v))
    end

    print "\n--- leave test_error ---"
end


test_xpath()
--test_relative_xpath()
--test_re()
--test_css()
--test_tostring()
--test_extract()
--test_error()



