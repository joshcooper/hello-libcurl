# README

This repo shows how `pkg-config` can be used to resolve build dependencies.

It contains:
* hello.c - C source file
* configure.ac - autoconf source file describing project and its dependencies
* Makefile.am - automake source file describing source files and output

You'll need to install `automake`, `autoconf`, `make` and have a working compiler.

Then run the following:

```
aclocal --verbose
autoconf --verbose
automake --add-missing --verbose
./configure
make
./hello
```

Here's what happens during each step:

## aclocal

`aclocal` scans `configure.ac` to see which m4 macros we're using and copies them from the system directory (`/usr/share/aclocal/*`) to a single file `./aclocal.m4` in the current directory. For example, the `configure.ac` contains

    PKG_CHECK_MODULES([LIBCURL], [libcurl])

So `aclocal` copies the `PKG_CHECK_MODULES` macro from `/usr/share/aclocal/pkg.m4`

    PKG_CHECK_MODULES(VARIABLE-PREFIX, MODULES, ...

and appends it to `./aclocal.m4`.

## autoconf

`autoconf` reads `configure.ac` and generates a `configure` script. When it sees the `PKG_CHECK_MODULES([LIBCURL], [libcurl])` macro in `configure.ac`, it writes this shell script snippet to `configure`:

    pkg_failed=no
    { $as_echo "$as_me:${as_lineno-$LINENO}: checking for LIBCURL" >&5
    $as_echo_n "checking for LIBCURL... " >&6; }

    if test -n "$LIBCURL_CFLAGS"; then
        pkg_cv_LIBCURL_CFLAGS="$LIBCURL_CFLAGS"
     elif test -n "$PKG_CONFIG"; then
        if test -n "$PKG_CONFIG" && \
        { { $as_echo "$as_me:${as_lineno-$LINENO}: \$PKG_CONFIG --exists --print-errors \"libcurl\""; } >&5
      ($PKG_CONFIG --exists --print-errors "libcurl") 2>&5
      ac_status=$?
      $as_echo "$as_me:${as_lineno-$LINENO}: \$? = $ac_status" >&5
      test $ac_status = 0; }; then
      pkg_cv_LIBCURL_CFLAGS=`$PKG_CONFIG --cflags "libcurl" 2>/dev/null`
                  test "x$?" != "x0" && pkg_failed=yes
    else
      pkg_failed=yes
    fi
     else
        pkg_failed=untried
    fi

## automake

`automake` reads `Makefile.am` and produces `Makefile.in`

In `configure.ac` we told `autoconf` that we had a configure-time substitution variable:

    AC_SUBST([LIBCURL_CFLAGS])

`automake` runs `autoconf --trace ...` to see which configure-time substitutions are valid:

    $ autoconf --trace=AC_SUBST_TRACE:\$f:\$l::\$d::\$n::\${::}% ...
    ...
    configure.ac:8::1::AC_SUBST_TRACE::LIBCURL_CFLAGS

Later, `automake` reads `Makefile.am`, which defines

    hello_CFLAGS = $(LIBCURL_CFLAGS)

Since `automake` knows `LIBCURL_CFLAGS` is a configure-time substitution variable, it emits a placeholder in `Makefile.in`:

    LIBCURL_CFLAGS = @LIBCURL_CFLAGS@

The end result is you have a `configure` script and `Makefile.in`.

## ./configure

The `configure` script is what finds your compiler and dependencies and produces the final `Makefile`.

It executes the shell snippet above produced from the m4 macro and stores the result of `pkg-config --cflags libcurl` in the `LIBCURL_CFLAGS` variable.

    ./configure
    ...
    checking for pkg-config... /usr/bin/pkg-config
    checking pkg-config is at least version 0.9.0... yes
    checking for LIBCURL... yes

It then replaces variables like `@LIBCURL_CFLAGS@` in `Makefile.in` with the `pkg-config` output. So this line in `Makefile.in`

    LIBCURL_CFLAGS = @LIBCURL_CFLAGS@

is written to `Makefile` as

    LIBCURL_CFLAGS = -I/usr/include/x86_64-linux-gnu

The end result is the `Makefile` describes how to build everything on *your* system.

## make

`make` reads `Makefile` and parses the rules for building `hello`. Make determines it first needs to build `hello-hello.o` from the source file `hello.c`

```
hello-hello.o: hello.c
    $(AM_V_CC)$(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(hello_CFLAGS) $(CFLAGS) -MT hello-hello.o -MD -MP -MF $(DEPDIR)/hello-hello.Tpo -c -o hello-hello.o `test -f 'hello.c' || echo '$(srcdir)/'`hello.c
```

Where `hello_CFLAGS` is defined as:

    LIBCURL_CFLAGS = -I/usr/include/x86_64-linux-gnu
    hello_CFLAGS = $(LIBCURL_CFLAGS)

Putting it together, `make` executes the following with our `LIBCURL_CFLAGS`:

    gcc -DPACKAGE_NAME=\"hello-libcurl\" -DPACKAGE_TARNAME=\"hello-libcurl\" -DPACKAGE_VERSION=\"0.1\" -DPACKAGE_STRING=\"hello-libcurl\ 0.1\" -DPACKAGE_BUGREPORT=\"you@example.com\" -DPACKAGE_URL=\"\" -DPACKAGE=\"hello-libcurl\" -DVERSION=\"0.1\" -I.    -I/usr/include/x86_64-linux-gnu -g -O2 -MT hello-hello.o -MD -MP -MF .deps/hello-hello.Tpo -c -o hello-hello.o `test -f 'hello.c' || echo './'`hello.c

Then it determines it needs to link the object files and `curl` to build `hello`:

    hello$(EXEEXT): $(hello_OBJECTS) $(hello_DEPENDENCIES) $(EXTRA_hello_DEPENDENCIES)
        @rm -f hello$(EXEEXT)
        $(AM_V_CCLD)$(hello_LINK) $(hello_OBJECTS) $(hello_LDADD) $(LIBS)

Where `hello_LDADD` is defined as:

    LIBCURL_LIBS = -lcurl
    hello_LDADD = $(LIBCURL_LIBS)

So it executes the following with our `LIBCURL_LIBS`, producing the executable `hello`:

    gcc -I/usr/include/x86_64-linux-gnu -g -O2   -o hello hello-hello.o -lcurl

## autoreconf

Rather than run `aclocal`, `autoconf`, `automake`, etc independently, you can run `autoreconf --install` to combine all of the autotools:

```
autoreconf --install
./configure
make
./hello
```
