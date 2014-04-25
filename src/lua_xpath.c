#include "lua_xpath.h"



#define LUA_XPATH_NAME          "xpath"
#define LUA_XPATH_MT_NAME       "xpath_mt"
#define LUA_XPATH_MAGIC         0x6870786C      /* "lxph" */


typedef struct xpath_selector_s {
    intptr_t                    magic;
    struct xpath_selector_s     *root;
    int                         ref;    /* lua reference, _root_ keeps it */
    int                         count;  /* refcount, _root_ keeps it */
    xmlDoc                      *doc;   /* xml document tree, _root_ keeps it */

    xmlNode                     *node;  /* current node */

    intptr_t                    tmp;    /* arbitrary */
} xpath_selector_t;


static int __extract_element_node(lua_State *L, xpath_selector_t *sel);
static int __extract_attribute_node(lua_State *L, xpath_selector_t *sel);
static int __extract_content_node(lua_State *L, xpath_selector_t *sel);

typedef int (*extract_handler_pt) (lua_State *, xpath_selector_t *);

static struct {
    char const          *name;
    int                 accept;
    extract_handler_pt  handler;
} __node_type[] = {
    { "None",       0, NULL },
    { "Element",    1, __extract_element_node },     /* XML_ELEMENT_NODE */
    { "Attribute",  1, __extract_attribute_node },   /* XML_ATTRIBUTE_NODE */
    { "Text",       1, __extract_content_node },     /* XML_TEXT_NODE */
    { "CDATA",      0, __extract_content_node },
    { "EntityRef",  0, NULL },
    { "Entity",     0, NULL },
    { "PI",         0, NULL },
    { "Comment",    0, __extract_content_node },
    { "Document",   0, NULL },
    /* TODO to be added */
    { NULL,         0, NULL }
};


static int xpath_loads(lua_State *L);
static int xpath_loadfile(lua_State *L);
static int xpath_eval_xpath(lua_State *L);
static int xpath_eval_regex(lua_State *L);
static int xpath_eval_css(lua_State *L);
static int xpath_extract(lua_State *L);
static int xpath_tostring(lua_State *L);
static int xpath_selector_gc(lua_State *L);


static luaL_Reg funcs[] = {
    { "loads",      xpath_loads },
    { "loadfile",   xpath_loadfile },
    { NULL,         NULL },
};

static luaL_Reg mt_funcs[] = {
    { "xpath",      xpath_eval_xpath },
    { "re",         xpath_eval_regex },
    { "css",        xpath_eval_css },
    { "extract",    xpath_extract },
    { NULL,         NULL }
};


#define __get_root(sel)         ((sel)->root ? (sel)->root : (sel))
#define __get_type(sel)         ((sel)->node->type)
#define __get_type_name(sel)    (__node_type[(sel)->node->type].name)
#define __get_node_name(sel)    ((char const *) (sel)->node->name)
#define __get_ref(sel)          (__get_root(sel)->ref)
#define __get_count(self)       (__get_root(sel)->count)


#ifndef DEBUG_LUA_XPATH
# define __dump_selector(sel, prefix) void

static void
__libxml2_error_handler(void *ctxt, char const *fmt, ...)
{
    return;
}

# define __dump_xmlnode(node, prefix) void

#else

static void
__dump_selector(xpath_selector_t *sel, char const *prefix)
{
    fprintf(stderr, "%s: selector: name '%s', type '%s', ref %d, count %d\n", 
        prefix ? prefix : "sd", __get_node_name(sel), __get_type_name(sel), 
        __get_ref(sel), __get_count(sel));
}

static void
__dump_xmlnode(xmlNode *node, char const *prefix)
{
    fprintf(stderr, "%s: node '%s', type %d\n", 
        prefix ? prefix : "xd", node->name, node->type);
}

static void
__libxml2_error_handler(void *ctxt, char const *fmt, ...)
{
    return;
}

#endif 


static void
__library_init()
{
    /* initialization function for the XML parser. this is not reentrant. call
     * once before processing in case of use in multithreaded programs.
     */
    xmlInitParser();

    xmlThrDefSetGenericErrorFunc(NULL, __libxml2_error_handler);
    xmlSetGenericErrorFunc(NULL, __libxml2_error_handler);
}


static void
__set_error(xpath_selector_t *sel, xmlError const *err, char const *def)
{
    if (err == NULL) {
        sel->tmp = (intptr_t) def;
        return;
    } 

    if (err->message && *err->message != '\0') {
        sel->tmp = (intptr_t) err->message;
    } else {
        sel->tmp = (intptr_t) def;
    }
}


static char const *
__get_error(xpath_selector_t *sel, char const *def)
{
    char *err = (char *) sel->tmp;
    return err ? err : def;
}


static int
__parse_string(xpath_selector_t *sel, char const *buf, 
    size_t len)
{
    xmlError *err;
    xmlParserCtxt *ctxt;
   

    xmlResetLastError();
    sel->tmp = (intptr_t) NULL;

    ctxt = xmlCreateMemoryParserCtxt(buf, len);
    if (ctxt == NULL) {
        __set_error(sel, xmlGetLastError(), "create parser error");

        return -1;
    }

    if (xmlParseDocument(ctxt) == -1) {
        __set_error(sel, xmlCtxtGetLastError(ctxt), "parser error");
        xmlFreeParserCtxt(ctxt);

        return -1;
    }

    sel->node = xmlDocGetRootElement(ctxt->myDoc);
    if (sel->node == NULL) {
        __set_error(sel, NULL, "no root element");
        xmlFreeDoc(ctxt->myDoc);
        xmlFreeParserCtxt(ctxt);

        return -1;
    }

    /* xmlNode s' memory will be freed by xmlFreeDoc` */
    sel->doc = ctxt->myDoc;

    xmlFreeParserCtxt(ctxt);

    return 0;
}


static int
__parse_file(xpath_selector_t *sel, char const *fname)
{
    xmlParserCtxt *ctxt = xmlCreateFileParserCtxt(fname);
    xmlFreeParserCtxt(ctxt);
    return 0;
}


static void **
__eval_xpath(lua_State *L, xpath_selector_t *sel, char const *xpath)
{
    int i;
    xmlXPathObject *result = NULL;
    xmlXPathContext *ctxt = NULL; 
    void **nodes = NULL;
    

    do {
        xmlResetLastError();
        /* used to save children number or error message */
        sel->tmp = 0;

        ctxt = xmlXPathNewContext(__get_root(sel)->doc);
        if (ctxt == NULL) {
            __set_error(sel, NULL, "create xpath context error");
            break;
        }

        /* the current node */
        ctxt->node = sel->node;

        result = xmlXPathEval((xmlChar const *) xpath, ctxt);
        if (result == NULL) {
            __set_error(sel, NULL, "invalid xpath");
            break;
        }

        if (result->type != XPATH_NODESET) {
            __set_error(sel, NULL, "only nodeset result supported");
            break;
        }

        if (!result->nodesetval || xmlXPathNodeSetIsEmpty(result->nodesetval)) {
            /* caller'll be aware of empty node set through `tmp` */
            nodes = (void **) !NULL;
            break;
        }

        /* let lua release this block after using */
        nodes = (void **) lua_newuserdata(L, 
            result->nodesetval->nodeNr * sizeof(*nodes));

        for (i = 0; i < result->nodesetval->nodeNr; i++) {
            xmlNode *node = result->nodesetval->nodeTab[i];

            __dump_xmlnode(node, "eval");

            if (__node_type[node->type].accept) {
                nodes[sel->tmp++] = node;
            }
        }

    } while (0);


    if (result) {
        xmlXPathFreeObject(result);
    }

    if (ctxt) {
        xmlXPathFreeContext(ctxt);
    }

    return nodes;
}


static int 
__extract_element_node(lua_State *L, xpath_selector_t *sel)
{
    int len, ret = -1;
    xmlBuffer *buffer = NULL;
    xmlOutputBuffer *output = NULL;


    do {
        xmlResetLastError();
        sel->tmp = (intptr_t) NULL;

        buffer = xmlBufferCreate();
        if (buffer == NULL) {
            __set_error(sel, NULL, "create buffer error");
            break;
        }

        output = xmlOutputBufferCreateBuffer(buffer, NULL);
        if (output == NULL) {
            __set_error(sel, NULL, "create output error");
            break;
        }

        xmlNodeDumpOutput(output, __get_root(sel)->doc, sel->node, 0, 0, NULL); 

        len = xmlOutputBufferClose(output);
        /* `output` is already released by `xmlOutputBuffer` */
        output = NULL;

        lua_pushlstring(L, buffer->content, len);
        ret = 0;

    } while (0);


    if (buffer) {
        xmlBufferFree(buffer);
        buffer = NULL;
    }

    return ret;
}


static int 
__extract_attribute_node(lua_State *L, xpath_selector_t *sel)
{
    xmlChar *value;
    xmlNode *node = sel->node;


    xmlResetLastError();
    sel->tmp = (intptr_t) NULL;

    if (node->ns && node->ns->href) {
        value = xmlGetNsProp(node->parent, node->name, node->ns->href);
        
    } else {
        value = xmlGetNoNsProp(node->parent, node->name);
    }

    if (value == NULL) {
        __set_error(sel, NULL, "no value found");
        return -1;
    }

    /* pushes the zero-terminated string pointed to by `s` onto the stack. Lua
     * makes (or reuse) an internal copy of the given string, so the memory at
     * `s` can be freed or reused immediately after the function returns. the
     * string connot contain embedded zeros; it is assumed to end at the frist
     * zero.
     */
    lua_pushstring(L, (char const *) value);

    xmlFree(value);

    return 0;
}


static int 
__extract_content_node(lua_State *L, xpath_selector_t *sel)
{
    (void ) sel;
    lua_pushstring(L, sel->node->content ? (char const *) sel->node->content : "");

    return 0;
}


/*
 * @params string
 * @return (xpath_selector_t) or (nil, err)
 */
static int
xpath_loads(lua_State *L)
{
    int ref;
    size_t length;
    const char *input;
    xpath_selector_t *sel;


    input = luaL_optlstring(L, 1, NULL, &length);

    if (input == NULL || length == 0) {
        lua_pushnil(L);
        lua_pushstring(L, "input string empty");
        return 2;
    }

    /* FIXME it never fail on memory insufficient ? */
    sel = (xpath_selector_t *) lua_newuserdata(L, sizeof(*sel));

    /* fields initialization */
    sel->magic = LUA_XPATH_MAGIC;
    sel->root = NULL;
    sel->ref = 0;
    sel->count = 0;
    sel->doc = NULL;
    sel->node = NULL;

    if (__parse_string(sel, input, length) == -1) {
        lua_pushnil(L);
        lua_pushstring(L, __get_error(sel, "parser error"));
        return 2;
    }

    /* we have to keep _root_ selector alive as long as there are children
     * alive.
     */
    /* creates and returns a _reference_, in the table at index `t`, for the
     * object at the top of the stack (and pops the object)
     */
    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    /* you can retrieve an object referred by referrence `r` by calling
     * `lua_rawgeti(L, t, r)`
     */
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

    sel = (xpath_selector_t *) lua_touserdata(L, -1);
    sel->ref = ref;
    sel->count = 1; /* referred by `lua_ref` all along */

    /* reuse the already existent metatable 
     */
    luaL_newmetatable(L, LUA_XPATH_MT_NAME);
    /* Pops a table from the stack and sets it as the new metatable for the
     * value at the given acceptable index 
     */
    lua_setmetatable(L, -2);

    return 1;
}


static int
xpath_loadfile(lua_State *L)
{ return 0; }

/*

            if (node->type == XML_TEXT_NODE) {
                printf("content: '%s'\n", node->content);

            } else if (node->type == XML_ATTRIBUTE_NODE) {
                xmlAttr *attr = (xmlAttr *) node;
                xmlChar *value;

                if (attr->ns && attr->ns->href) {
                    value = xmlGetNsProp(attr->parent, attr->name, attr->ns->href);
                } else {
                    value = xmlGetNoNsProp(attr->parent, attr->name);
                }

                printf("attribute: '%s'\n", value);
            }
            */



static int
xpath_eval_xpath(lua_State *L)
{ 
    int i;
    size_t length;
    void **nodes;
    const char *xpath;
    xpath_selector_t *sel, *node;


    if (!lua_isuserdata(L, 1)) {
        lua_pushnil(L);
        lua_pushstring(L, "first arg should be selector object");
        return 2;
    }

    sel = (xpath_selector_t *) lua_touserdata(L, 1);
    if (sel->magic != LUA_XPATH_MAGIC) {
        lua_pushnil(L);
        lua_pushstring(L, "selector object needed");
        return 2;
    }

    /* `lua_tolstring` returns a fully aligned pointer to a string inside the
     * Lua state. This string always has a zero ('\0') after its last character
     * (as in C), but can contain other zeros in its body. Because Lua has GC,
     * there is no guarantee that the pointer returned by `lua_tolstring` will
     * be valid after the correponding value is removed from the stack.
     */
    xpath = luaL_optlstring(L, 2, NULL, &length);
    if (xpath == NULL || length == 0) {
        lua_pushnil(L);
        lua_pushstring(L, "xpath pattern needed");
        return 2;
    }

    nodes = (void **) __eval_xpath(L, sel, xpath);
    if (nodes == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, __get_error(sel, "eval xpath error"));
        return 2;
    }

    /* a list of `selector` object */
    lua_createtable(L, sel->tmp, 0);

    for (i = 0; i < sel->tmp; i++) {
        /* note lua table's base index */
        lua_pushinteger(L, i + 1);

        /* FIXME it never fail on memory insufficient ? */
        node = lua_newuserdata(L, sizeof(*node));

        /* set metatable for new selector object */
        luaL_newmetatable(L, LUA_XPATH_MT_NAME);
        lua_setmetatable(L, -2);

        node->magic = LUA_XPATH_MAGIC;
        node->root = __get_root(sel);
        node->ref = 0;
        node->count = 0;
        node->doc = NULL;
        node->node = nodes[i];

        /* increase refcount to keep root alive at least for 
           this new selector's lifetime */
        __get_root(sel)->count++; 

        lua_settable(L, -3);
    }

    return 1; 
}

static int
xpath_eval_regex(lua_State *L)
{ 
    lua_pushnil(L);
    lua_pushstring(L, "not implemented yet");
    return 2; 
}

static int
xpath_eval_css(lua_State *L)
{ 
    lua_pushnil(L);
    lua_pushstring(L, "not implemented yet");
    return 2; 
}


static int
xpath_extract(lua_State *L)
{
    xpath_selector_t *sel;


    if (!lua_isuserdata(L, 1)) {
        lua_pushnil(L);
        lua_pushstring(L, "first arg should be selector object");
        return 2;
    }

    sel = (xpath_selector_t *) lua_touserdata(L, 1);
    if (sel->magic != LUA_XPATH_MAGIC) {
        lua_pushnil(L);
        lua_pushstring(L, "selector object needed");
        return 2;
    }

    __dump_selector(sel, NULL);

    if (!__node_type[__get_type(sel)].handler) {
        lua_pushnil(L);
        lua_pushstring(L, "node type not supported");
        return 2;
    }
    
    if (__node_type[__get_type(sel)].handler(L, sel) == -1) {
        lua_pushnil(L);
        lua_pushstring(L, __get_error(sel, "extract error"));
        return 2;
    }

    return 1;
}


/* memory deallocation:
 *
 *  from manual of Lua 5.1
 *
 * * Userdata represent C values in Lua. A _full userdata_ represents a block of
 *   memory. It is an object (like a table): you must create it, it can have its
 *   own metatable, and you can detect when it is being collected (through
 *   metamethod `__gc`). A _full userdata_ is only equal to itself (under raw
 *   equality). When Lua collects a full userdata with a `gc` metamethod, Lua
 *   calls the metamethod and marks the userdata as finalized. When this
 *   userdata is collected again then Lua frees its corresponding memory.
 *
 * * At the end of each garbage-collection cycle, the finalizers for userdata
 *   are called in _reverse order_ of their creation, among those collected in
 *   that cycle. The userdata itself is freed only in the next
 *   garbage-collection cycle.
 */
static int
xpath_selector_gc(lua_State *L)
{
    xpath_selector_t *sel, *root;


    sel = (xpath_selector_t *) lua_touserdata(L, 1);

    __dump_selector(sel, "gc");

    /* TODO release resource of current node */

    /* refcount on _root_ selector */
    root = __get_root(sel);
    root->count--;

    if (root->count == 1) {
        /* the last reference is hold by `lua_ref` */

        /* release reference `ref` from the table at index `t`. the entry is
         * removed from the table, so that the referred object can be collected.
         * The reference `ref` is also freed to be used again. 
         */
        luaL_unref(L, LUA_REGISTRYINDEX, root->ref);

    } else if (root->count <= 0) {
        root->node = NULL;
        xmlFreeDoc(root->doc);
        root->doc = NULL;
    }

    return 0;
}


static int
xpath_tostring(lua_State *L)
{
    char brief[64];


    if (lua_istable(L, 1)) {
        snprintf(brief, sizeof(brief), "<module XPath>: %p", 
            lua_topointer(L, 1));

    } else {
        xpath_selector_t *sel = lua_touserdata(L, 1);

        snprintf(brief, sizeof(brief), "<selector %s '%s' r%d c%d>: %p",
            __get_type_name(sel), __get_node_name(sel), __get_ref(sel), 
            __get_count(sel), lua_topointer(L, 1));
    }

    lua_pushstring(L, brief);
    return 1;
}


static int
create_metatable(lua_State *L)
{
    luaL_reg *reg;

    /* If the registry already has the key, returns 0. Otherwise, create a new
     * table to be used as a metatable for userdata, adds it to the registry
     * with the key, and returns 1
     */
    luaL_newmetatable(L, LUA_XPATH_MT_NAME);

    lua_pushstring(L, "__index");
    lua_newtable(L);

    for (reg = mt_funcs; reg->name; reg++) {
        lua_pushstring(L, reg->name);
        lua_pushcfunction(L, reg->func);
        lua_rawset(L, -3);
    }

    lua_rawset(L, -3); /* set metatable's __index to this new table */

    /* only act on userdata, that is, object of xpath_select_t  */
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, xpath_selector_gc);
    lua_rawset(L, -3); /* set metatable's __gc to xpath_selector_gc */

    /* act both on selector and module xpath itself */
    lua_pushstring(L, "__tostring");
    lua_pushcfunction(L, xpath_tostring);
    lua_rawset(L, -3); /* set metatable's __tostring to xpath_tostring */

    /* now the newly created metatable stays on the top of the stack */

    return 1;
}


int
luaopen_xpath(lua_State *L)
{
    __library_init();

    luaL_register(L, LUA_XPATH_NAME, funcs);

    create_metatable(L);

    /* Pops a table from the stack and sets it as the new metatable for the
     * value at the given acceptable index */
    lua_setmetatable(L, -2); /* set module's metatable */

    /* version string */
    lua_pushstring(L, "_VERSION");
    lua_pushstring(L, LUA_XPATH_VERSION);
    lua_rawset(L, -3);

    return 1;
}


