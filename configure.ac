AC_INIT([hello-libcurl], [0.1], [you@example.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Checks for programs.
AC_PROG_CC

# Use pkg-config to check for libcurl
PKG_CHECK_MODULES([LIBCURL], [libcurl])

# Export flags
AC_SUBST([LIBCURL_CFLAGS])
AC_SUBST([LIBCURL_LIBS])

# Create output files
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
