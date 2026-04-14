dnl config.m4 for extension PHP Debugger (based on Xdebug)

PHP_ARG_ENABLE(php-debugger, whether to enable PHP Debugger support,
[  --enable-php-debugger   Enable PHP Debugger support])
PHP_ARG_ENABLE(php-debugger-dev, whether to enable PHP Debugger developer build flags,
[  --enable-php-debugger-dev  PHP Debugger: Enable developer flags],, no)



m4_include([m4/pkg.m4])
m4_include([m4/clocks.m4])

if test "$PHP_PHP_DEBUGGER" != "no"; then
  AC_MSG_CHECKING([for supported PHP version])
  PHP_XDEBUG_FOUND_VERSION=`${PHP_CONFIG} --version`
  PHP_XDEBUG_FOUND_VERNUM=`${PHP_CONFIG} --vernum`
  if test "$PHP_XDEBUG_FOUND_VERNUM" -lt "80000"; then
    AC_MSG_ERROR([not supported. Need a PHP version >= 8.0.0 and < 8.7.0 (found $PHP_XDEBUG_FOUND_VERSION)])
  else
    if test "$PHP_XDEBUG_FOUND_VERNUM" -ge "80700"; then
      AC_MSG_ERROR([not supported. Need a PHP version >= 8.0.0 and < 8.7.0 (found $PHP_XDEBUG_FOUND_VERSION)])
    else
      AC_MSG_RESULT([supported ($PHP_XDEBUG_FOUND_VERSION)])
    fi
  fi
  
  AC_DEFINE(HAVE_XDEBUG,1,[ ])

  old_CPPFLAGS=$CPPFLAGS
  CPPFLAGS="$INCLUDES $CPPFLAGS"

  AC_XDEBUG_CLOCK

  AC_CHECK_HEADERS([netinet/in.h poll.h sys/poll.h])
  case $host_os in
  linux*)
    AC_DEFINE(HAVE_XDEBUG_CONTROL_SOCKET_SUPPORT,1,[ do have control socket support? ])
    AC_CHECK_HEADERS([linux/rtnetlink.h], [], [
      case $host_os in
        linux-musl*)
          AC_MSG_ERROR([rtnetlink.h is required, install the linux-headers package: apk add --update linux-headers])
      esac
      AC_MSG_ERROR([rtnetlink.h is required, please make sure it is available by installing the correct package])
    ])
  esac

  PHP_CHECK_FUNC(res_ninit, resolv)
  PHP_CHECK_FUNC(res_nclose, resolv)

  PHP_CHECK_LIBRARY(m, cos, [ PHP_ADD_LIBRARY(m,, PHP_DEBUGGER_SHARED_LIBADD) ])



  CPPFLAGS=$old_CPPFLAGS

  if test "$PHP_PHP_DEBUGGER_DEV" = "yes"; then
    AX_CHECK_COMPILE_FLAG(-Wbool-conversion,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wbool-conversion")
    AX_CHECK_COMPILE_FLAG(-Wdeclaration-after-statement,    _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wdeclaration-after-statement")
    AX_CHECK_COMPILE_FLAG(-Wdiscarded-qualifiers,           _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wdiscarded-qualifiers")
    AX_CHECK_COMPILE_FLAG(-Wduplicate-enum,                 _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wduplicate-enum")
    AX_CHECK_COMPILE_FLAG(-Wempty-body,                     _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wempty-body")
    AX_CHECK_COMPILE_FLAG(-Wenum-compare,                   _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wenum-compare")
    AX_CHECK_COMPILE_FLAG(-Werror,                          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Werror")
    AX_CHECK_COMPILE_FLAG(-Wextra,                          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wextra")
    AX_CHECK_COMPILE_FLAG(-Wformat-nonliteral,              _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wformat-nonliteral")
    AX_CHECK_COMPILE_FLAG(-Wformat-security,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wformat-security")
    AX_CHECK_COMPILE_FLAG(-Wheader-guard,                   _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wheader-guard")
    AX_CHECK_COMPILE_FLAG(-Wincompatible-pointer-types-discards-qualifiers, _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wincompatible-pointer-types-discards-qualifiers")
    AX_CHECK_COMPILE_FLAG(-Wimplicit-fallthrough,           _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wimplicit-fallthrough")
    AX_CHECK_COMPILE_FLAG(-Winit-self,                      _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Winit-self")
    AX_CHECK_COMPILE_FLAG(-Wlogical-not-parentheses,        _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wlogical-not-parentheses")
    AX_CHECK_COMPILE_FLAG(-Wlogical-op,                     _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wlogical-op")
    AX_CHECK_COMPILE_FLAG(-Wlogical-op-parentheses,         _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wlogical-op-parentheses")
    AX_CHECK_COMPILE_FLAG(-Wloop-analysis,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wloop-analysis")
    AX_CHECK_COMPILE_FLAG(-Wmaybe-uninitialized,            _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wmaybe-uninitialized")
    AX_CHECK_COMPILE_FLAG(-Wmissing-format-attribute,       _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wmissing-format-attribute")
    AX_CHECK_COMPILE_FLAG(-Wno-missing-field-initializers,  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-missing-field-initializers")
    AX_CHECK_COMPILE_FLAG(-Wno-sign-compare,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-sign-compare")
    AX_CHECK_COMPILE_FLAG(-Wno-unused-but-set-variable,     _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-unused-but-set-variable")
    AX_CHECK_COMPILE_FLAG(-Wno-unused-parameter,            _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-unused-parameter")
    AX_CHECK_COMPILE_FLAG(-Wno-variadic-macros,             _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wno-variadic-macros")
    AX_CHECK_COMPILE_FLAG(-Wparentheses,                    _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wparentheses")
    AX_CHECK_COMPILE_FLAG(-Wpointer-bool-conversion,        _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wpointer-bool-conversion")
    AX_CHECK_COMPILE_FLAG(-Wsizeof-array-argument,          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wsizeof-array-argument")
    AX_CHECK_COMPILE_FLAG(-Wstring-conversion,              _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wstring-conversion")
    AX_CHECK_COMPILE_FLAG(-Wwrite-strings,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wwrite-strings")
    AX_CHECK_COMPILE_FLAG(-Wpointer-arith,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wpointer-arith")
    AX_CHECK_COMPILE_FLAG(-Wstrict-prototypes,              _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wstrict-prototypes")
    AX_CHECK_COMPILE_FLAG(-Wold-style-definition,           _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -Wold-style-definition")
    AX_CHECK_COMPILE_FLAG(-fdiagnostics-show-option,        _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fdiagnostics-show-option")
    AX_CHECK_COMPILE_FLAG(-fno-exceptions,                  _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fno-exceptions")
    AX_CHECK_COMPILE_FLAG(-fno-omit-frame-pointer,          _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fno-omit-frame-pointer")
    AX_CHECK_COMPILE_FLAG(-fno-optimize-sibling-calls,      _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fno-optimize-sibling-calls")
    AX_CHECK_COMPILE_FLAG(-fsanitize-address,               _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fsanitize-address")
    AX_CHECK_COMPILE_FLAG(-fstack-protector,                _MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS -fstack-protector")

    MAINTAINER_CFLAGS="$_MAINTAINER_CFLAGS"
    STD_CFLAGS="-g -O0 -Wall"
  fi

  PHP_XDEBUG_CFLAGS="$STD_CFLAGS $MAINTAINER_CFLAGS"

  XDEBUG_BASE_SOURCES="src/base/base.c src/base/ctrl_socket.c"
  XDEBUG_LIB_SOURCES="src/lib/usefulstuff.c src/lib/cmd_parser.c src/lib/compat.c src/lib/crc32.c src/lib/file.c src/lib/hash.c src/lib/headers.c src/lib/lib.c src/lib/llist.c src/lib/log.c src/lib/normalize_path.c src/lib/set.c src/lib/str.c src/lib/timing.c src/lib/trim.c src/lib/var.c src/lib/var_export_html.c src/lib/var_export_line.c src/lib/var_export_text.c src/lib/var_export_xml.c src/lib/xdebug_strndup.c src/lib/xml.c src/lib/compat_stubs.c"
  XDEBUG_LIB_MAPS_SOURCES="src/lib/maps/maps.c src/lib/maps/maps_private.c src/lib/maps/parser.c"

  XDEBUG_DEBUGGER_SOURCES="src/debugger/com.c src/debugger/debugger.c src/debugger/frankenphp.c src/debugger/handler_dbgp.c src/debugger/handlers.c src/debugger/ip_info.c"

  PHP_NEW_EXTENSION(php_debugger, xdebug.c $XDEBUG_BASE_SOURCES $XDEBUG_LIB_SOURCES $XDEBUG_LIB_MAPS_SOURCES $XDEBUG_DEBUGGER_SOURCES, $ext_shared,,$PHP_XDEBUG_CFLAGS,,yes)
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(php_debugger)[/src/base])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(php_debugger)[/src/lib])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(php_debugger)[/src/lib/maps])
  PHP_ADD_BUILD_DIR(PHP_EXT_BUILDDIR(php_debugger)[/src/debugger])
  PHP_SUBST(PHP_DEBUGGER_SHARED_LIBADD)
  PHP_ADD_MAKEFILE_FRAGMENT

  PHP_ADD_INCLUDE($ext_srcdir/src)
  PHP_ADD_INCLUDE($ext_builddir/src)
  PHP_ADD_INCLUDE($ext_srcdir)
fi
