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
        print(k, v)
    end

    print "--- leave test_xpath ---\n"
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
        

    print "--- leave test_relative_xpath ---\n"
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

    print "--- leave test_re ---\n"
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

    print "--- leave test_css ---\n"
end


function test_tostring()
    print "--- enter test_tostring ---\n"

    print(xpath)

    local sel, err = xpath.loads(xml)
    print(sel, err)

    print "--- leave test_tostring ---\n"
end


--test_xpath()
test_relative_xpath()
--test_re()
--test_css()
--test_tostring()



