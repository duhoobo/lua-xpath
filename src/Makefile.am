
lib_LTLIBRARIES = xpath.la

xpath_la_SOURCES = lua_xpath.c
xpath_la_CPPFLAGS = $(LUA_INCLUDE) $(XML_CPPFLAGS) -DLUA_USE_APICHECK

# -module: creates a library that can be dlopened, and module names don't need
# to be prefixed with `lib`.
xpath_la_LDFLAGS = -module -avoid-version
xpath_la_LIBADD = $(XML_LIBS) $(LUA_LIB)


xpath.so: $(lib_LTLIBRARIES)
	@echo creating xpath.so...
	cp .libs/xpath.so xpath.so


# local version of `all`. automake will supplement the standard target with
# these user-supplied targets.
all-local: xpath.so


EXTRA_DIST = lua_xpath.h
