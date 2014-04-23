#include "lua_xpath.h"



#define LUA_XPATH_NAME          "xpath"
#define LUA_XPATH_MT_NAME       "xpath_mt"


typedef struct xpath_selector_s {
    struct xpath_selector_s *root;
    int                     ref;   /* lua reference, _root_ keeps it */
    int                     count; /* refcount, _root_ keeps it */
    xmlDoc                  *doc;  /* xml document tree, _root_ keeps it */

    xmlNode                 *node; /* current node */

    int                     tmp;
} xpath_selector_t;


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


static int xpath_loads(lua_State *L);
static int xpath_loadfile(lua_State *L);
static int xpath_eval_xpath(lua_State *L);
static int xpath_eval_regex(lua_State *L);
static int xpath_eval_css(lua_State *L);
static int xpath_extract(lua_State *L);
static int xpath_dump(lua_State *L);
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
    { "dump",       xpath_dump },
    { NULL,         NULL }
};


#define __get_root(sel) ((sel)->root ? (sel)->root : (sel))


static void
__selector_dump(xpath_selector_t *sel, char const *prefix)
{
    xpath_selector_t *root = __get_root(sel);
    printf("%s: selector: name '%s', ref %d, count %d\n", prefix, 
        sel->node->name, root->ref, root->count);
}


static int
__parse_string(xpath_selector_t *sel, char const *buf, 
    size_t len)
{
    xmlParserCtxt *ctxt;
   

    ctxt = xmlCreateMemoryParserCtxt(buf, len);
    if (ctxt == NULL) {
        return -1;
    }

    if (xmlParseDocument(ctxt) == -1) {
        xmlFreeParserCtxt(ctxt);
        return -1;
    }

    sel->node = xmlDocGetRootElement(ctxt->myDoc);
    if (sel->node == NULL) {
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
__eval_xpath(lua_State *L, xpath_selector_t *sel, char const *rule)
{
    int i;
    xmlXPathObject *result = NULL;
    xmlXPathContext *ctxt = NULL; 
    void **nodes = NULL;
    

    do {
        /* used to save children number */
        sel->tmp = 0;

        ctxt = xmlXPathNewContext(__get_root(sel)->doc);
        if (ctxt == NULL) {
            break;
        }

        /* the current node */
        ctxt->node = sel->node;

        result = xmlXPathEval((xmlChar const *) rule, ctxt);
        if (result == NULL) {
            break;
        }

        if (result->type != XPATH_NODESET) {
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

            printf("node <%s>, type %d\n", node->name, node->type);

            if (node->type == XML_TEXT_NODE || node->type == XML_ATTRIBUTE_NODE)
            {
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
    sel->root = NULL;
    sel->ref = 0;
    sel->count = 0;
    sel->doc = NULL;
    sel->node = NULL;

    if (__parse_string(sel, input, length) == -1) {
        lua_pushnil(L);
        lua_pushstring(L, "xml string parsing error");
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
    const char *rule;
    xpath_selector_t *sel, *node;


    sel = (xpath_selector_t *) lua_touserdata(L, 1);
    if (sel == NULL) {
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
    rule = luaL_optlstring(L, 2, NULL, &length);
    if (rule == NULL || length == 0) {
        lua_pushnil(L);
        lua_pushstring(L, "xpath pattern needed");
        return 2;
    }

    nodes = (void **) __eval_xpath(L, sel, rule);
    if (nodes == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "xpath eval error");
        return 2;
    }

    /* a list of `selector` object */
    lua_createtable(L, sel->tmp, 0);

    printf("found %d elements\n", sel->tmp);

    for (i = 0; i < sel->tmp; i++) {
        /* note lua table's base index */
        lua_pushinteger(L, i + 1);

        /* FIXME it never fail on memory insufficient ? */
        node = lua_newuserdata(L, sizeof(*node));

        /* set metatable for new selector object */
        luaL_newmetatable(L, LUA_XPATH_MT_NAME);
        lua_setmetatable(L, -2);

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
{ return 0; }

static int
xpath_eval_css(lua_State *L)
{ return 0; }


static int
xpath_extract(lua_State *L)
{
    return 0;
}

static int
xpath_dump(lua_State *L)
{
    xpath_selector_t *sel;

    return 0;
}

static int
xpath_selector_gc(lua_State *L)
{
    xpath_selector_t *sel, *root;


    sel = (xpath_selector_t *) lua_touserdata(L, 1);

    __selector_dump(sel, "gc");

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

    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, xpath_selector_gc);
    lua_rawset(L, -3); /* set metatable's __gc to xpath_selector_gc */

    /* now the newly created metatable stays on the top of the stack */

    return 1;
}


int
luaopen_xpath(lua_State *L)
{
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


