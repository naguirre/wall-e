AC_INIT([wall-e], [0.1.0], [enlightenment-devel@lists.sourceforge.net])
AC_PREREQ([2.52])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE(1.6 dist-bzip2)
AM_CONFIG_HEADER(config.h)

lt_enable_auto_import=""
case "$host_os" in
   mingw* | cegcc*)
      lt_enable_auto_import="-Wl,--enable-auto-import"
esac
AC_SUBST(lt_enable_auto_import)

PKG_CHECK_MODULES([WALLE], [ecore-evas edje ecore-file elementary])

AC_PROG_CC
AM_PROG_CC_C_O

AC_CONFIG_FILES([
Makefile
data/Makefile
data/theme/Makefile
data/theme/default/Makefile
src/Makefile
src/bin/Makefile
])

AC_OUTPUT

echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE_NAME $PACKAGE_VERSION"
echo "------------------------------------------------------------------------"
echo
echo "Configuration Options Summary:"
echo
echo "  Compilation..........: make"
echo
echo "  Installation.........: make install"
echo
echo "    prefix.............: $prefix"
echo
