#!/usr/bin/env python

import xml.etree.ElementTree as ElementTree


tree = ElementTree.parse("contry_data.xml")
root = tree.getroot()


print root.tag, root.attrib


for child in root:
    print type(child), child.tag, child.attrib

print root[0][1].text


for neighbor in root.iter('neighbor'):
    print neighbor.attrib


print root.findall("country")

for country in root.findall("country"): # direct children
    rank = country.find("rank").text # value
    name = country.get("name") # attribute

    print name, rank


for rank in root.iter("rank"):
    rank.text = str(int(rank.text) + 1)
    rank.set("updated", "yes")  # set attribute

tree.write("output.xml")


for country in root.findall("country"):
    rank = int(country.find("rank").text)
    if rank > 50:
        root.remove(country)

tree.write("output2.xml")


# limited support for XPath expressions

year = root.findall(".//*[@name='Singapore']/year")
print year[0].text


