include ../../Makefile.header

TARGETS = \
	$(UL_BIN)/upread2

all: $(TARGETS)

#
# At the nici, we have a policy where we assign a unique code to each file in
# all our UNIPEN files.# The file containing the identifications is defined by
# WRITER_ID_FILE.# If you do want something like that, make sure that your file
# confirms to the following specifications. The file contains a line for each
# UNIPEN file. On each line, a unique file_code and the writer_name are
# contained. Furthermore, the dataset to which the writer belongs and a path
# to the UNIPEN file are contained:
#
# int file_code;   char *name;  char *dataset;  char *path;
#
# As an example, consider two writers 'a' and 'b'. The first has contributed
# data to a dataset called 'setA', the second to 'setB'. From the first writer,
# two files are collected, from the second only one:
#
# 00000 a setA a1.dat
# 00001 a setA a2.dat
# 00002 b setB b1.dat
#
# If you want to use this, uncomment the next lines, where
# WID_FILE = filename containing unipen identifications
# WID_ENV  = name of environment variable (overrides WID_FILE)

WID_FILE = $(NICI_UPT)/Tools/Upread2/Unipen_identifications
WID_ENV  = UNIPEN_ID_FILE
USE_WID  = -DUSE_WID -DWID_ENV='"$(WID_ENV)"' -DWRITER_ID_FILE='"$(WID_FILE)"'

OBJ =\
	allo_resample.o\
	output_unipen.o\
	output_featchar.o\
	output_image.o\
	image_routines.o\
	unipen_identifications.o\
	bitmap_routines.o
upread2: $(UL_BIN)/upread2
	@(echo made upread2)
$(UL_BIN)/upread2: upread2.o $(OBJ)
	$(CC) -o $@ upread2.o $(OBJ)\
		-L$(UL_LIB) -luplib$(VERSION) $(LINK_X11) -lXmu -lm

output_featchar.o: output_featchar.c upread2.h
	$(CC) $(CFLAGS) $(USE_WID) -c output_featchar.c -o $@
output_image.o: output_image.c upread2.h
	$(CC) $(CFLAGS) $(USE_WID) -c output_image.c -o $@
unipen_identifications.o: unipen_identifications.c unipen_identifications.h
	$(CC) $(CFLAGS) $(USE_WID) -c unipen_identifications.c -o $@
upread2.o: upread2.c upread2.h
	$(CC) $(CFLAGS) $(USE_WID) -c upread2.c -o $@

bitmap_routines.o: bitmap_routines.c bitmap_routines.h
	$(CC) $(CFLAGS) -c bitmap_routines.c $(XINCS) -o $@

clean:
	\rm -f *.o $(TARGETS)
