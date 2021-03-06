#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68]) #TODO Update when travis moves to a newer version
AC_INIT([protoc-gen-php], [1.0], [github@bramp.net])
AC_CONFIG_SRCDIR([src/protoc-gen-php.cc])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

AC_CHECK_PROG([PROTOC], [protoc], [protoc])
AS_IF([test "x${PROTOC}" == "x"],
    [AC_MSG_ERROR([Protobuf compiler "protoc" not found.])])

# TODO Check PHP version
AC_CHECK_PROG([PHP], [php], [php])
AS_IF([test "x${PHP}" == "x"],
    [AC_MSG_ERROR([PHP not found.])])

AC_CHECK_PROGS([PHP_OR_HHVM], [php hhvm])
AS_IF([test "x${PHP_OR_HHVM}" == "x"],
    [AC_MSG_ERROR([Both PHP and HHVM not found. Atleast one must be available.])])

# The following is needed for dev/testing
AC_CHECK_PROG([COMPOSER], [composer], [composer])
AS_IF([test "x${COMPOSER}" == "x"],
    [AC_MSG_ERROR([composer not found. https://getcomposer.org])])

# Checks for libraries.
PKG_CHECK_MODULES(PROTOBUF, protobuf >= 2.6.1)
#PKG_CHECK_MODULES(PROTOBUF, protobuf >= 3)
AC_SUBST(PROTOBUF_LIBS)
AC_SUBST(PROTOBUF_CFLAGS)
AC_SUBST(PROTOBUF_CFLAGS_ONLY_I)
AC_SUBST(PROTOBUF_VERSION)

# Checks for header files.
AC_CHECK_HEADERS([float.h limits.h stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL #TODO Autoconf > 2.68 is available we can reenable
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_STRTOD
AC_CHECK_FUNCS([localeconv memmove setlocale strchr strpbrk strtol strtoul strtoull])

AC_OUTPUT(Makefile src/Makefile tests/Makefile reference_app/Makefile)
