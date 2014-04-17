#!/usr/bin/env python

from lxml import etree

# From: https://pypi.python.org/pypi/lxml/2.3
#
# lxml is a Pythonic, mature binding for the libxml2 and libxslt libraries. It
# provides safe and convenient access to these libraries using the ElementTree
# API.
# It extends the ElementTree API significantly to offer support for XPath,
# RelaxNG, XML Schema, XSLT, C14N and much more.
#

tree = etree.parse("./country_data.xml")

print tree.xpath("//year/text()")

# An ElementTree is mainly a document wrapper around a tree with a root node. It
# provides a couple of methods for parsing, serialisation and general document
# handling. One of the bigger differences is that it serialises as a complete
# document, as opposed to a single Element. This includes top-level processing
# instructions and comments, as well as a DOCTYPE and other DTD content in the
# document.

