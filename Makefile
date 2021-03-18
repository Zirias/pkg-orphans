LOCALBASE?=	/usr/local
PREFIX?=	${LOCALBASE}
LIBDIR=		${PREFIX}/lib/pkg
SHLIB_DIR?=	${LIBDIR}
SHLIB_NAME?=	${PLUGIN_NAME}.so

PLUGIN_NAME=	orphans
SRCS=		orphans.c

CFLAGS+=	-I${LOCALBASE}/include

beforeinstall:
	${INSTALL} -d ${DESTDIR}${SHLIB_DIR}

.include <bsd.lib.mk>
