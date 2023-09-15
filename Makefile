include Makefile.header

all: uplib tools UPTOOLS_SETTINGS
	@(echo made it in $(UL_LIB) and $(UL_BIN))
	@(echo make sure to source the file 'UPTOOLS_SETTINGS' before starting any application)

configure_tcl:
	@(echo "cd Tools/Upworks/unix; ./configure $(VERSION)")
	@(cd Tools/Upworks/unix; ./configure $(VERSION))

uplib:
	@(cd Uplib; make)

tools:
	@(cd Tools; make)

uni2animgif:
	@(cd Tools; make uni2animgif)

unipen2eps:
	@(cd Tools; make unipen2eps)

upread:
	@(cd Tools; make upread)

upview:
	@(cd Tools; make upview)

upworks:
	@(cd Tools; make upworks)

UPTOOLS_SETTINGS: configure Makefile.template
	@(./configure UPTOOLS_SETTINGS)

cleanup:
	@(cd Tools; make clean)
	@(cd Uplib; make clean)

clean:
	@(cd Tools; make clean)
	@(cd Uplib; make clean)
	@(\rm -f UPTOOLS_SETTINGS)
	@(cd $(UL_BIN); \rm -f upview upread unipen2eps uni2animgif upworks)
	@(cd $(UL_LIB); \rm -f libink* libupworks* libuplib* pkgIndex.tcl)

BIN_TARGETS = upview$(VERSION) upread$(VERSION) unipen2eps$(VERSION) uni2animgif$(VERSION)
LIB_TARGETS = libink$(VERSION)$(SLSUFFIX) libupworks$(VERSION)$(SLSUFFIX) libuplib$(VERSION)$(SLSUFFIX)

################################################################################
# below stuff for installation

clean_install:
	@(cd $(UP_INSTALL_BIN); \rm -f libink* libupworks* libuplib*)
	@(cd $(UP_INSTALL_BIN); \rm -f upview$(VERSION) upread$(VERSION)\
		unipen2eps$(VERSION) uni2animgif$(VERSION) upworks$(VERSION) )
	@(cd $(UP_INSTALL_UL_LIB); \rm -f UPTOOLS_SETTINGS)

install: install_bin install_lib install_tcl install_tcl_stuff
	@(./configure $(UP_INSTALL_UL_LIB) $(UP_INSTALL_BIN) $(UP_INSTALL_LIB) $(UP_INSTALL_TCL))
	@(cd Uplib; make $(INSTALL_MAKE_FLAGS))
	@(cd Tools; make $(INSTALL_MAKE_FLAGS))
	@(cd Tools; make  $(INSTALL_MAKE_FLAGS) install_upworks install_upread2)
	@(cp -f data/unipen.def $(UP_INSTALL_UL_LIB))
	@(cp -f UpworksDefaults $(UP_INSTALL_UL_LIB))
	@(if test -x allow_work ; then allow_work LOCK_UPTOOLS ; fi)

install_bin:
	@(if test ! -d $(UP_INSTALL_BIN) ; then mkinstalldirs $(UP_INSTALL_BIN) ; fi)
#
#	@(cp -f $(UL_BIN)/upview      $(UP_INSTALL_BIN)/upview$(VERSION))
#	@(cp -f $(UL_BIN)/upread      $(UP_INSTALL_BIN)/upread$(VERSION))
#	@(cp -f $(UL_BIN)/unipen2eps  $(UP_INSTALL_BIN)/unipen2eps$(VERSION))
#	@(cp -f $(UL_BIN)/uni2animgif $(UP_INSTALL_BIN)/uni2animgif$(VERSION))
#

install_lib:
	@(if test ! -d $(UP_INSTALL_LIB) ; then mkinstalldirs $(UP_INSTALL_LIB) ; fi)
#
#	@(cp -f $(UL_LIB)/libuplib$(VERSION)$(SLSUFFIX)  $(UP_INSTALL_LIB))
#	@(cp -f $(UL_LIB)/libink$(VERSION)$(SLSUFFIX)    $(UP_INSTALL_LIB))
#	@(cp -f $(UL_LIB)/libupworks$(VERSION)$(SLSUFFIX) $(UP_INSTALL_LIB))
#	@(cd $(UL_LIB); $(UPTOOLS)/Tools/Upworks/unix/pkgInstall.tcl $(UP_INSTALL_LIB))
#

install_tcl:
	@(if test ! -d $(UP_INSTALL_TCL) ; then mkinstalldirs $(UP_INSTALL_TCL) ; fi)
	@(cp -f $(UL_BIN)/upworks  $(UP_INSTALL_TCL)/upworks$(VERSION))


install_tcl_stuff:
	echo uptools library becomes $(UP_INSTALL_UL_LIB)
	@(if test ! -d $(UP_INSTALL_UL_LIB) ; then mkinstalldirs $(UP_INSTALL_UL_LIB) ; fi)
	@(if test ! -d $(UP_INSTALL_UL_LIB)/library ; \
		then cd $(UPTOOLS)/Tools/Upworks; cp -f -r library $(UP_INSTALL_UL_LIB) ; \
		fi)
#
#	cd $(UPTOOLS)/Tools/Upworks; cp -f -r library $(UP_INSTALL_UL_LIB)
