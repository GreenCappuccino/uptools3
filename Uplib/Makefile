include ../Makefile.header

all: $(SL_TARGET) $(L_TARGET)

#all: $(L_TARGET)

UPLIBL = uplib.o\
	upsiglib.o\
	uphierarchy.o\
	up_segment_io.o

UPLIBSL = sluplib.o\
	slupsiglib.o\
	sluphierarchy.o\
	slup_segment_io.o

$(L_TARGET): $(UPLIBL)
	@(umask 007)
	$(L_LD) $@ $(UPLIBL)
	$(MAY_USE_RANLIB) $@
$(SL_TARGET): $(UPLIBSL)
	@(umask 007)
	$(SL_LD) $@ $(UPLIBSL)

sluplib.o: uplib.c uplib.h
	@(umask 007)
	$(CC) -c $(SLFLAGS) -DUPLIB_VERSION=$(UL_VERSION) uplib.c -o $@
slupsiglib.o: upsiglib.c upsiglib.h
	@(umask 007)
	$(CC) -c $(SLFLAGS) upsiglib.c -o $@
sluphierarchy.o: uphierarchy.c uphierarchy.h
	@(umask 007)
	$(CC) -c $(SLFLAGS) uphierarchy.c -o $@
slup_segment_io.o: up_segment_io.c up_segment_io.h
	@(umask 007)
	$(CC) -c $(SLFLAGS) up_segment_io.c -o $@

uplib.o: uplib.c uplib.h
	@(umask 007)
	$(CC) -c $(CFLAGS) -DUPLIB_VERSION=$(UL_VERSION) uplib.c -o $@
upsiglib.o: upsiglib.c upsiglib.h
	@(umask 007)
	$(CC) -c $(CFLAGS) upsiglib.c -o $@
uphierarchy.o: uphierarchy.c uphierarchy.h
	@(umask 007)
	$(CC) -c $(CFLAGS) uphierarchy.c -o $@
up_segment_io.o: up_segment_io.c up_segment_io.h
	@(umask 007)
	$(CC) -c $(CFLAGS) up_segment_io.c -o $@

###################     clean     ###################

clean:
	\rm -f $(TARGET) *.o
