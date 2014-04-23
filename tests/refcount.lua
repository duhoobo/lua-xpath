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


local sel, err = xpath.loads(xml)

if not sel then
    print(err)
    return nil
end

print(sel)

local list, err = sel:xpath("//body/name/@color")

if not list then
    print(err)
    return nil
end

for k, v in pairs(list) do
    print(k, v)
end

print(sel)

print "release root `sel`"
sel = nil

print "\n--- enter 1st gc"
collectgarbage("collect")
print "--- leave 1st gc\n"

print "keep the first node, release root `sel` and the second `sel`"
sel = list[1]
print(sel)
list = nil

print "\n--- enter 2nd gc ---"
collectgarbage("collect")
print "--- leave 2nd gc ---\n"

print(sel)

sel = nil

print "release the first node"

print "\n--- enter 3rd gc ---"
collectgarbage("collect")
print "--- leave 3rd gc ---\n"

print "references are all clear"

print "\n--- enter 4th gc ---"
collectgarbage("collect")
print "--- leave 4th gc ---\n"

print "new root `sel`"
sel, err = xpath.loads(xml)

