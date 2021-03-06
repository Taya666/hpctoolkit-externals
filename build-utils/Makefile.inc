#
#  Makefile include rules.
#
#  Copyright (c) 2008-2019, Rice University.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
#  * Neither the name of Rice University (RICE) nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
#  This software is provided by RICE and contributors "as is" and any
#  express or implied warranties, including, but not limited to, the
#  implied warranties of merchantability and fitness for a particular
#  purpose are disclaimed. In no event shall RICE or contributors be
#  liable for any direct, indirect, incidental, special, exemplary, or
#  consequential damages (including, but not limited to, procurement of
#  substitute goods or services; loss of use, data, or profits; or
#  business interruption) however caused and on any theory of liability,
#  whether in contract, strict liability, or tort (including negligence
#  or otherwise) arising in any way out of the use of this software, even
#  if advised of the possibility of such damage.
#
#  $Id$
#

ifneq ($(MAKEFILE_CONF),yes)
include ../Makefile.conf
endif

FETCH_COOKIE =   .fetch-done
CONFIG_COOKIE =  .config-done
BUILD_COOKIE =   .build-done
INSTALL_COOKIE = .install-done
NOBUILD_COOKIE = .dont-build
BAK = .bak

PATCH_DIR ?=  patches
BUILD_DIR ?=  work
BUILD_SUBDIRS  ?= none
INSTALL_SUBDIR ?= $(PKG_NAME)

FETCH_COPY_FILES ?= none
AC_CONFIG_DIR ?= none

FETCH_CMD ?=    true
PATCH_CMD ?= 	default
CONFIG_CMD ?=   ./configure
BUILD_CMD ?=	default
INSTALL_CMD ?=  default
CLEAN_CMD ?=	rm -rf "$(SRC_DIR)" "$(BUILD_DIR)"

POST_FETCH_CMD ?= none
POST_BUILD_CMD ?= none
POST_INSTALL_CMD ?= none

FETCH_FTP =  ../build-utils/fetch-ftp.sh

# Make variables whose default value is empty.
# PREREQS =
# EXTRA_PATCH_FILES =
# EXTRA_TRY_PATCH_FILES =

# Pass make to configure scripts.
export MAKE

.PHONY: all prereqs fetch refetch config reconfig build rebuild
.PHONY: install reinstall clean distclean

all: install

prereqs:
	@for pkg in $(PREREQS) ; do  \
		$(MAKE) -C "../$$pkg" install || exit 2 ;  \
	done

refetch:
	@echo ; echo "===> ($(PKG_NAME)) make fetch ..."
	@test x != "x$(SRC_DIR)"
	@if test -d "$(SRC_DIR)$(BAK)" ; then  \
		echo rm -rf "$(SRC_DIR)$(BAK)" ;  \
		rm -rf "$(SRC_DIR)$(BAK)" ;  \
	fi
	@if test -d "$(SRC_DIR)" ; then  \
		echo mv -f "$(SRC_DIR)" "$(SRC_DIR)$(BAK)" ;  \
		mv -f "$(SRC_DIR)" "$(SRC_DIR)$(BAK)" ;  \
	fi
	@rm -f $(FETCH_COOKIE) $(CONFIG_COOKIE)
	@rm -f $(BUILD_COOKIE) $(INSTALL_COOKIE)
	@if test "x$(FETCH_TARFILE)" != x ; then  \
		for f in $(FETCH_TARFILE) ; do  \
			$(FETCH_FTP) "$$f"  \
				"$(FETCH_URL_1)"  \
				"$(FETCH_URL_2)"  \
				"$(FETCH_URL_3)"  \
				"$(FETCH_URL_4)" || exit 2 ; \
		done ;  \
	elif test 'x$(FETCH_CMD)' != x ; then  \
		echo '$(FETCH_CMD)' ;  \
		$(FETCH_CMD) ;  \
	else  \
		echo "missing FETCH_CMD" ;  \
		exit 2 ;  \
	fi
	-ls -l "$(SRC_DIR)"
	@if test 'x$(PATCH_CMD)' != xdefault ; then  \
		echo ; echo "===> ($(PKG_NAME)) patching ..." ;  \
		$(PATCH_CMD) ;  \
	elif test -d "$(PATCH_DIR)" ; then  \
		echo ; echo "===> ($(PKG_NAME)) patching ..." ;  \
		for p in $(PATCH_DIR)/patch-*  \
			 $(addprefix $(PATCH_DIR)/,$(EXTRA_PATCH_FILES)) ;  \
		do  \
			if test -f "$$p" ; then  \
				echo `basename $$p` ;  \
				patch -d "$(SRC_DIR)" -p1 -N <"$$p" || exit 2 ;  \
			fi ;  \
		done ;  \
		for p in $(PATCH_DIR)/try-patch-*  \
			 $(addprefix $(PATCH_DIR)/,$(EXTRA_TRY_PATCH_FILES)) ;  \
		do  \
			if test -f "$$p" ; then  \
				echo `basename $$p` ;  \
				if patch --dry-run -d "$(SRC_DIR)" -p1 -N <"$$p" ; then  \
					echo "apply: `basename $$p`" ;  \
					patch -d "$(SRC_DIR)" -p1 -N <"$$p" ;  \
				else  \
					echo "skip: `basename $$p`" ;  \
				fi ;  \
			fi ;  \
		done ;  \
	fi
	@if test 'x$(FETCH_COPY_FILES)' != xnone ; then  \
		for f in $(FETCH_COPY_FILES) ; do  \
			echo "copy $(SRC_DIR)/$$f" ;  \
			rm -f "$(SRC_DIR)/$$f.orig" ;  \
			cp -f -p "$(SRC_DIR)/$$f" "$(SRC_DIR)/$$f.orig" ;  \
		done ;  \
	fi
	@if test 'x$(AC_CONFIG_DIR)' != xnone ; then  \
		echo "installing new config.guess, config.sub" ;  \
		for f in config.guess config.sub ; do  \
			rm -f "$(SRC_DIR)/$(AC_CONFIG_DIR)/$$f.orig" ;  \
			mv -f "$(SRC_DIR)/$(AC_CONFIG_DIR)/$$f"  \
			      "$(SRC_DIR)/$(AC_CONFIG_DIR)/$$f.orig" ;  \
			cp -f "../config/$$f"  \
			      "$(SRC_DIR)/$(AC_CONFIG_DIR)/$$f" || exit 2 ;  \
		done ;  \
	fi
	@if test 'x$(POST_FETCH_CMD)' != xnone ; then  \
		echo ; echo "===> ($(PKG_NAME)) post fetch ..." ;  \
		echo '$(POST_FETCH_CMD)' ;  \
		$(POST_FETCH_CMD) ;  \
	fi
	@echo "resetting permissions on: $(SRC_DIR)"
	-find "$(SRC_DIR)" -type d -exec chmod a+rx {} \;
	-chmod -R a+r "$(SRC_DIR)"
	touch $(FETCH_COOKIE)

fetch:
	@if test -f $(NOBUILD_COOKIE) ; then  \
		echo "$(PKG_NAME): `cat $(NOBUILD_COOKIE)`" ;  \
	elif test ! -f $(FETCH_COOKIE) ; then  \
		$(MAKE) refetch ;  \
	fi

reconfig: fetch prereqs
	@echo ; echo "===> ($(PKG_NAME)) make config ..."
	@test x != 'x$(CONFIG_CMD)'
	@rm -f $(CONFIG_COOKIE) $(BUILD_COOKIE) $(INSTALL_COOKIE)
	$(CONFIG_CMD)
	touch $(CONFIG_COOKIE)

config:
	@if test -f $(NOBUILD_COOKIE) ; then  \
		echo "$(PKG_NAME): `cat $(NOBUILD_COOKIE)`" ;  \
	elif test ! -f $(CONFIG_COOKIE) ; then  \
		$(MAKE) reconfig ;  \
	fi

rebuild: config
	@echo ; echo "===> ($(PKG_NAME)) make build ..."
	@test x != 'x$(BUILD_CMD)'
	@rm -f $(BUILD_COOKIE) $(INSTALL_COOKIE)
	@if test 'x$(BUILD_CMD)' != xdefault ; then  \
		echo "$(BUILD_CMD)" ;  \
		$(BUILD_CMD) ;  \
	elif test 'x$(BUILD_SUBDIRS)' = xnone ; then  \
		$(MAKE) -C "$(BUILD_DIR)" ;  \
	else  \
		for dir in $(BUILD_SUBDIRS) ; do  \
			echo "===> ($(PKG_NAME)) make build $$dir ..." ;  \
			$(MAKE) -C "$(BUILD_DIR)/$$dir" || exit 2 ;  \
		done ;  \
	fi
	@if test 'x$(POST_BUILD_CMD)' != xnone ; then  \
		echo ; echo "===> ($(PKG_NAME)) post build ..." ;  \
		echo '$(POST_BUILD_CMD)' ;  \
		$(POST_BUILD_CMD) ;  \
	fi
	touch $(BUILD_COOKIE)

build:
	@if test -f $(NOBUILD_COOKIE) ; then  \
		echo "$(PKG_NAME): `cat $(NOBUILD_COOKIE)`" ;  \
	elif test ! -f $(BUILD_COOKIE) ; then  \
		$(MAKE) rebuild ;  \
	fi

reinstall: build
	@echo ; echo "===> ($(PKG_NAME)) make install ..."
	@test x != 'x$(INSTALL_CMD)'
	@rm -f $(INSTALL_COOKIE)
	@echo "removing old install directory: $(INSTALL_SUBDIR)"
	-rm -rf "$(PREFIX)/$(INSTALL_SUBDIR)"
	@if test 'x$(INSTALL_CMD)' != xdefault ; then  \
		echo "$(INSTALL_CMD)" ;  \
		$(INSTALL_CMD) ;  \
	elif test 'x$(BUILD_SUBDIRS)' = xnone ; then  \
		$(MAKE) -C "$(BUILD_DIR)" install ;  \
	else  \
		for dir in $(BUILD_SUBDIRS) ; do  \
			echo "===> ($(PKG_NAME)) make install $$dir ..." ;  \
			$(MAKE) -C "$(BUILD_DIR)/$$dir" install || exit 2 ;  \
		done ;  \
	fi
	@if test 'x$(POST_INSTALL_CMD)' != xnone ; then  \
		echo ; echo "===> ($(PKG_NAME)) post install ..." ;  \
		echo '$(POST_INSTALL_CMD)' ;  \
		$(POST_INSTALL_CMD) ;  \
	fi
	@echo "resetting permissions on: $(INSTALL_SUBDIR)"
	-find "$(PREFIX)/$(INSTALL_SUBDIR)" -type d -exec chmod a+rx {} \;
	-chmod -R a+r "$(PREFIX)/$(INSTALL_SUBDIR)"
	touch $(INSTALL_COOKIE)
	@echo "$(PKG_NAME) build complete"

install:
	@if test -f $(NOBUILD_COOKIE) ; then  \
		echo "$(PKG_NAME): `cat $(NOBUILD_COOKIE)`" ;  \
	elif test -f $(INSTALL_COOKIE) ; then  \
		echo "$(PKG_NAME): already installed" ;  \
	else  \
		$(MAKE) reinstall ;  \
	fi

clean:
	@echo ; echo "===> ($(PKG_NAME)) make clean ..."
	@test x != 'x$(CLEAN_CMD)'
	$(CLEAN_CMD)
	@rm -f $(FETCH_COOKIE) $(CONFIG_COOKIE) $(BUILD_COOKIE)

distclean:
	@echo ; echo "===> ($(PKG_NAME)) make distclean ..."
	@test x != 'x$(CLEAN_CMD)'
	$(CLEAN_CMD)
	@rm -f $(FETCH_COOKIE) $(CONFIG_COOKIE)
	@rm -f $(BUILD_COOKIE) $(INSTALL_COOKIE)

