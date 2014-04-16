local xpath = require("xpath")

local tree, err = xpath.loads("xxxx")
print(tree:dump())

print(tree, err)
