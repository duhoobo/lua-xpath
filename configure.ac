
AC_INIT([lua-xpath], [0.0.1], [Hungpu DU <alecdu@gmail.com>])
AC_CONFIG_AUX_DIR([build-aux])
# automake options
AM_INIT_AUTOMAKE([-Wall -Werror dist-zip no-dist-gzip])

AC_MSG_NOTICE([configuring for $PACKAGE_NAME-$PACKAGE_VERSION])


AC_CONFIG_MACRO_DIR([m4])
AM_CONFIG_HEADER(config.h)


AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_SED
AM_PROG_AR
LT_INIT([disable-static shared])
# by autoconf-archive
AX_WITH_PROG([MD5SUM], [md5sum], [false])


AC_CHECK_HEADERS([string.h stdint.h])


# lua
AX_PROG_LUA([5.1], [5.2])
AX_LUA_HEADERS  # LUA_INCLUDE is set if suttable headers found
AX_LUA_LIBS     # LUA_LIB is set if suitable library found

# libxml2
# https://developer.gnome.org/anjuta-build-tutorial/stable/library-autotools.html.en
# supplied by libxml.m4 of package `libxml2-dev` 
AM_PATH_XML2([2.9.0])   # higher or equals to, XML_CPPFLAGS, XML_LIBS


AC_MSG_CHECKING([whether XPath of libxml2 enabled])

AC_LANG_PUSH([C])
saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$XML_CPPFLAGS"

AC_COMPILE_IFELSE([
  AC_LANG_SOURCE([[
    #include <libxml/xpath.h>
    void main() { int ok = XPATH_EXPRESSION_OK; }
  ]])],
  AC_MSG_RESULT([yes]),
  AC_MSG_ERROR([no]),
])

CPPFLAGS="$saved_CPPFLAGS"
AC_LANG_POP


AC_CONFIG_FILES([Makefile 
                 src/Makefile 
                 rockspec/$PACKAGE.rockspec
                 tests/Makefile
                 ])
AC_OUTPUT

