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

