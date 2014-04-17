#include "lua_xpath.h"



#define LUA_XPATH_NAME          "xpath"
#define LUA_XPATH_MT_NAME       "xpath_mt"


#define LUA_XPATH_TREE     1
#define LUA_XPATH_ITEM     2


typedef struct xpath_object_s {
    int type;
} xpath_object_t;

typedef struct xpath_tree_s {
    /* must be the first */
    xpath_object_t object;
} xpath_tree_t;

typedef struct xpath_item_s {
    /* must be the first */
    xpath_object_t object;
} xpath_item_t;

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
static int xpath_select(lua_State *L);
static int xpath_extract(lua_State *L);
static int xpath_dump(lua_State *L);
static int xpath_object_gc(lua_State *L);


static luaL_Reg funcs[] = {
    { "loads", xpath_loads },
    { "loadfile", xpath_loadfile },
    { NULL, NULL },
};

static luaL_Reg mt_funcs[] = {
    { "select", xpath_select },
    { "extract", xpath_extract },
    { "dump", xpath_dump },
    { NULL, NULL }
};


/*
 * @params string
 * @return (xpath_tree_t) or (nil, err)
 */
static int
xpath_loads(lua_State *L)
{
    size_t length;
    const char *input;
    xpath_tree_t *tree;


    input = luaL_optlstring(L, 1, NULL, &length);

    if (input == NULL || length == 0) {
        lua_pushnil(L);
        lua_pushstring(L, "input string empty");
        return 2;
    }

    tree = (xpath_tree_t *) lua_newuserdata(L, sizeof(*tree));
    if (tree == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, "memory allocation error");
        return 2;
    }

    tree->object.type = LUA_XPATH_TREE;

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
{
    return 0;
}


static int
xpath_select(lua_State *L)
{
    return 0;
}


static int
xpath_extract(lua_State *L)
{
    return 0;
}

static int
xpath_dump(lua_State *L)
{
    xpath_object_t *object;


    object = (xpath_object_t *) lua_touserdata(L, -1);

    if (object->type == LUA_XPATH_TREE) {
        lua_pushstring(L, "tree");

    } else {
        lua_pushstring(L, "item");
    }

    return 1;
}


static int
xpath_object_gc(lua_State *L)
{
    xpath_object_t *object;


    if (!lua_isuserdata(L, -1)) {
        printf("__gc on no-userdata");
        return 0;
    }

    object = (xpath_object_t *) lua_touserdata(L, -1);

    if (object->type == LUA_XPATH_TREE) {
        printf("__gc on tree called\n");

    } else {
        printf("__gc on item called\n");
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
    lua_pushcfunction(L, xpath_object_gc);
    lua_rawset(L, -3); /* set metatable's __gc to xpath_object_gc */

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


