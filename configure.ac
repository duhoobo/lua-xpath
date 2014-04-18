
AC_INIT([lua-xpath], [0.0.1], [Hungpu DU <alecdu@gmail.com>])
AC_CONFIG_AUX_DIR([build-aux])
# automake options
AM_INIT_AUTOMAKE([-Wall -Werror dist-zip no-dist-gzip])

AC_MSG_NOTICE([configuring for $PACKAGE_NAME-$PACKAGE_VERSION])


AC_CONFIG_MACRO_DIR([m4])
AM_CONFIG_HEADER(config.h)
AC_DISABLE_STATIC


AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_SED
AM_PROG_AR
LT_INIT
# by autoconf-archive
AX_WITH_PROG([MD5SUM], [md5sum], [false])


AC_CHECK_HEADERS(string.h)


# lua
AX_PROG_LUA([5.1], [5.2])
AX_LUA_HEADERS  # LUA_INCLUDE is set if suttable headers found
AX_LUA_LIBS     # LUA_LIB is set if suitable library found

# libxml2
# https://developer.gnome.org/anjuta-build-tutorial/stable/library-autotools.html.en
# supplied by libxml.m4 of package `libxml2-dev` 
AM_PATH_XML2([2.9.0])   # higher or equals to, XML_CPPFLAGS, XML_LIBS


AC_CONFIG_FILES([Makefile 
                 src/Makefile 
                 rockspec/$PACKAGE.rockspec
                 tests/Makefile
                 ])
AC_OUTPUT

