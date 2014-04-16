xml-notes
=========


xml.etree
---------

XML is an inherently hierarchical data format, and the most natural way to
represent it is with a tree. 

* *ElementTree* represents the whole XML document as a tree. Interactions with 
the whole document are usually done on the *ElementTree* level.
    
* *Element* represents a single node in this tree. Interactions with a single 
XML element and its sub-elements are done on the *Element* level.


Each XML element has a number of properties associated with it:

* a tag which is a string identifying what kind of data this element 
represents (the element type, in other words)

* a number of attributes, stored in a Python dictionary.

* a text string.

* an optional tail string.

* a number of child elements, stored in a Python sequence.


libxml2
-------

    /* data types and sample function */
    
    xmlDocPtr
    xmlChar
    xmlNodeSetPtr
    xmlXPathObjectPtr
    xmlXPathContextPtr
    xmlInitParser()
    xmlCleanupParser()
    xmlParseFile
    xmlXPathNewContent
    xmlXPathEvalExpression
    xmlNodePtr
    
    void parse_story(xmlDocPtr doc, xmlNodePtr cur) 
    {
        xmlChar *key;
        cur = cur->xmlChildrenNode;
        
        while (cur != NULL) {
        
            if ((!xmlStrcmp(cur->name, (const xmlChar *) "keyword"))) {
                key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                ...
                xmlFree(key);
            }
            
            cur = cur->next;
        }
        
        return;
    }


lxml
----

lxml extends the *ElementTree* API significantly to offer support for XPath,
RelaxNG, XML Schema, XSLT, C14N and much more, and provides safe and convenient
access to these libraries(libxml2, libxslt) using the *ElementTree* API.

* XML namespace - [XML_namespace](http://en.wikipedia.org/wiki/XML_namespace)

* Cython - It is so close to Python that the Cython compiler can actually
compile many, many Python programs to C without major modifications. But the
real speed gains of a C compilation come from type annotations that were added
to the language and that allow Cython to generate very efficient C code.

    * It generates all the necessary glue code for the Python API, including 
    Python types, calling conventions and reference counting.
    
    * On the other side of the table, calling into C code is not more that
    declaring the signature of the function and maybe some variables as being
    C types, pointers or structs, and then calling it. 
    
    * The reset of the code is just plain Python code.
    
    * `.pyx` - all main moudle
    
    * `cimport` - compile-time import for Cython, imports C declarations, either
    from external libraries or from other Cython modules.
    

* lxml source code conventions

    * The main extension modules in lxml are lxml.etree and lxml.objectify. All 
    main modules have the file extension .pyx, which shows the descendence from 
    Pyrex. As usual in Python, the main files start with a short description 
    and a couple of imports. Cython distinguishes between the run-time import 
    statement (as known from Python) and the compile-time cimport statement, 
    which imports C declarations, either from external libraries or from other 
    Cython modules.
    
    * lxml's tree API is based on proxy objects. That means, every Element 
    object (or rather `_Element` object) is a proxy for a libxml2 node structure.
    
    * It is a naming convention that C variables and C level class members that 
    are passed into libxml2 start with a prefixed `c_` (commonly libxml2 struct 
    pointers), and that C level class members are prefixed with an underscore. 
    So you will often see names like `c_doc` for an `xmlDoc*` variable (or 
    `c_node` for an `xmlNode*`), or the above `_c_node` for a class member that 
    points to an xmlNode struct (or `_c_doc` for an `xmlDoc*`).

    * The main module, `lxml.etree`, is in the file `lxml.etree.pyx`. It 
    implements the main functions and types of the ElementTree API, as well as 
    all the factory functions for proxies. It is the best place to start if you 
    want to find out how a specific feature is implemented.
    
    * The main include files are:
    
        * apihelpers.pxi
        
            Private C helper functions. Except for the factory functions, most 
            of the little functions that are used all over the place are defined 
            here. This includes things like reading out the text content of a 
            libxml2 tree node, checking input from the API level, creating a new 
            Element node or handling attribute values. If you want to work on 
            the lxml code, you should keep these functions in the back of your 
            head, as they will definitely make your life easier.
            
        * classlookup.pxi
            Element class lookup mechanisms. The main API and engines for those 
            who want to define custom Element classes and inject them into lxml.
            
        * docloader.pxi
            Support for custom document loaders. Base class and registry for 
            custom document resolvers.
            
        * extensions.pxi
            Infrastructure for extension functions in XPath/XSLT, including 
            XPath value conversion and function registration.
            
        * iterparse.pxi
            Incremental XML parsing. An iterator class that builds iterparse 
            events while parsing.
            
        * nsclasses.pxi
            Namespace implementation and registry. The registry and engine for 
            Element classes that use the ElementNamespaceClassLookup scheme.
            
        * parser.pxi
            Parsers for XML and HTML. This is the main parser engine. It's the 
            reason why you can parse a document from various sources in two 
            lines of Python code. It's definitely not the right place to start 
            reading lxml's soure code.
            
        * parsertarget.pxi
            An ElementTree compatible parser target implementation based on the 
            SAX2 interface of libxml2.
            
        * proxy.pxi
            Very low-level functions for memory allocation/deallocation and 
            Element proxy handling. Ignoring this for the beginning will safe 
            your head from exploding.
            
        * public-api.pxi
            The set of C functions that are exported to other extension modules 
            at the C level. For example, lxml.objectify makes use of these. See 
            the C-level API documentation.
            
        * readonlytree.pxi
            A separate read-only implementation of the Element API. This is used 
            in places where non-intrusive access to a tree is required, such as 
            the PythonElementClassLookup or XSLT extension elements.
            
        * saxparser.pxi
            SAX-like parser interfaces as known from ElementTree's TreeBuilder.
            
        * serializer.pxi
            XML output functions. Basically everything that creates byte 
            sequences from XML trees.
            
        * xinclude.pxi
            XInclude support.
            
        * xmlerror.pxi
            Error log handling. All error messages that libxml2 generates 
            internally walk through the code in this file to end up in lxml's 
            Python level error logs. 
            At the end of the file, you will find a long list of named error 
            codes. It is generated from the libxml2 HTML documentation (using 
            lxml, of course). See the script update-error-constants.py for this.
            
        * xmlid.pxi
            XMLID and IDDict, a dictionary-like way to find Elements by their 
            XML-ID attribute.
            
        * xpath.pxi
            XPath evaluators.
            
        * xslt.pxi
            XSL transformations, including the XSLT class, document lookup 
            handling and access control.
            
    * pure Python modules
    
        * builder.py
            The E-factory and the ElementBuilder class. These provide a simple 
            interface to XML tree generation.
        * cssselect.py
            A CSS selector implementation based on XPath. The main class is 
            called CSSSelector.
        * doctestcompare.py
            A relaxed comparison scheme for XML/HTML markup in doctest.
        * ElementInclude.py
            XInclude-like document inclusion, compatible with ElementTree.
        * _elementpath.py
            XPath-like path language, compatible with ElementTree.
        * sax.py
            SAX2 compatible interfaces to copy lxml trees from/to SAX compatible 
            tools.
        * usedoctest.py
            Wrapper module for doctestcompare.py that simplifies its usage from 
            inside a doctest.
            
    * lxml.objectify

        A Cython implemented extension module that uses the public C-API of 
        lxml.etree. It provides a Python object-like interface to XML trees. 
        The implementation resides in the file lxml.objectify.pyx.
        
    * lxml.html

        A specialised toolkit for HTML handling, based on lxml.etree. This is 
        implemented in pure Python.

    
###代码结构

    _ElementTree                        # return by _elementTreeFactory()
        ._doc = doc                     # xmlDocPtr
        ._content_node = doc.getroot()  # xmlNodePtr
        .xpath
            evaluator = XPathDocumentEvaluator(...)
            return evaluator(_path, **_variables)
        
                    xmlXPathCompiledEval
        
    从 xpath 这个功能来讲，lxml 只是在 libxml2 基础上做了类型变换的工作。
    从 ElementTree 角度讲，lxml 也是在 libxml2 基础上做了类型变换的工作。
    
    
    

