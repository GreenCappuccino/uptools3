include ../Makefile.header

all: \
	upread2\
	upread\
	upview\
	unipen2eps\
	uni2animgif\
	upworks

TARGETS = \
	$(UL_BIN)/upread2\
	$(UL_BIN)/upread\
	$(UL_BIN)/upview\
	$(UL_BIN)/unipen2eps\
	$(UL_BIN)/uni2animgif

upread: $(UL_BIN)/upread $(LIB_UPLIB)
$(UL_BIN)/upread: Upread/upread.o $(LIB_UPLIB)
	@(umask 007)
	$(CC) -o $@ Upread/upread.o $(LINK_UPLIB)
Upread/upread.o: Upread/upread.c
	@(umask 007)
	$(CC) $(CFLAGS) -DUR_VERSION=$(UR_VERSION) -c Upread/upread.c -o $@

upread2: $(LIB_UPLIB)
	@(cd Upread2; make)
install_upread2:
	@(cd Upread2; make $(INSTALL_MAKE_FLAGS))

upview: $(UL_BIN)/upview $(LIB_UPLIB)
$(UL_BIN)/upview: Upview/upview.o $(LIB_UPLIB)
	@(umask 007)
	$(CC) -o $@ Upview/upview.o $(LINK_UPLIB) $(LINK_LIBS) -lm
Upview/upview.o: Upview/upview.c Upview/upview.h
	@(umask 007)
	$(CC) $(CFLAGS) -DUV_VERSION=$(UV_VERSION) -c Upview/upview.c -o $@

unipen2eps: $(UL_BIN)/unipen2eps $(LIB_UPLIB)
$(UL_BIN)/unipen2eps: Unipen2eps/unipen2eps.o $(LIB_UPLIB)
	@(umask 007)
	$(CC) -o $@ Unipen2eps/unipen2eps.o $(LINK_UPLIB) -lm
Unipen2eps/unipen2eps.o: Unipen2eps/unipen2eps.c
	@(umask 007)
	$(CC) $(CFLAGS) -c Unipen2eps/unipen2eps.c -o $@

U2GIFANIM_OBJ = Uni2animgif/examine_gif.o\
	Uni2animgif/scale.o\
	Uni2animgif/uni2animgif.o\
	Uni2animgif/gifmerge_patch.o

uni2animgif: $(UL_BIN)/uni2animgif
$(UL_BIN)/uni2animgif: $(U2GIFANIM_OBJ) $(LIB_UPLIB)
	@(umask 007)
	$(CC) -o $@ $(U2GIFANIM_OBJ) $(LINK_UPLIB) -lm
Uni2animgif/examine_gif.o: Uni2animgif/examine_gif.c
	@(umask 007)
	$(CC) $(CFLAGS) -c Uni2animgif/examine_gif.c -o $@
Uni2animgif/scale.o: Uni2animgif/scale.c
	@(umask 007)
	$(CC) $(CFLAGS) -c Uni2animgif/scale.c -o $@
Uni2animgif/uni2animgif.o: Uni2animgif/uni2animgif.c
	@(umask 007)
	$(CC) $(CFLAGS) -c Uni2animgif/uni2animgif.c -o $@
Uni2animgif/gifmerge_patch.o: Uni2animgif/gifmerge_patch.c
	@(umask 007)
	$(CC) $(CFLAGS) -c Uni2animgif/gifmerge_patch.c -o $@

upworks: $(LIB_TARGETS)
	@(cd Upworks/unix; make)

install_upworks:
	@(cd Upworks/unix; make $(INSTALL_MAKE_FLAGS))

clean:
	\rm -f $(TARGETS) */*.o
	@(if test -d Upworks ; then cd Upworks/unix ; make clean ; fi)
