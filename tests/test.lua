--package.cpath = "/home/duhoobo/prj/amateur/lua-xpath/src/?.so;" .. package.cpath

local xpath = require("xpath")



local tree, err = xpath.loads("xxxx")
print(tree:dump())

print(tree, err)
print(xpath._VERSION)
