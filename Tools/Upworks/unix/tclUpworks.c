#include <string.h>
char TclLoadLibraries_Upworks [] =
  "@LIBS: -L/other/kunhp1/ext/users/vuurpijl/uptools3/lib -luplib3 -L/usr/local/lib -ltcl7.6 -lm_G0 -lc_G0";
extern int Unipen_Init (); 
extern int add_penstream (); 
extern int add_sample (); 
extern int add_segment_to_signal (); 
extern int add_y_extremum (); 
extern int copy_current_delineation (); 
extern int do_get_child_markers (); 
extern int do_get_edited_segment (); 
extern int do_get_segment_delineation (); 
extern int do_set_segm_markers (); 
extern int initiate_segment_walk (); 
extern int jump_segment_to_signal (); 
extern int walk_via_penstream (); 
static struct {
  char * name;
  int (*value)();
}dictionary [] = {
  { "Unipen_Init", Unipen_Init },
  { "add_penstream", add_penstream },
  { "add_sample", add_sample },
  { "add_segment_to_signal", add_segment_to_signal },
  { "add_y_extremum", add_y_extremum },
  { "copy_current_delineation", copy_current_delineation },
  { "do_get_child_markers", do_get_child_markers },
  { "do_get_edited_segment", do_get_edited_segment },
  { "do_get_segment_delineation", do_get_segment_delineation },
  { "do_set_segm_markers", do_set_segm_markers },
  { "initiate_segment_walk", initiate_segment_walk },
  { "jump_segment_to_signal", jump_segment_to_signal },
  { "walk_via_penstream", walk_via_penstream },
{0,0}
};
typedef struct Tcl_Interp Tcl_Interp;
typedef int Tcl_PackageInitProc (Tcl_Interp *);
Tcl_PackageInitProc *
TclLoadDictionary_Upworks (symbol)
    char * symbol;
{
    int i;
    for (i = 0; dictionary [i] . name != 0; ++i) {
      if (!strcmp (symbol, dictionary [i] . name)) {
	return dictionary [i].value;
      }
    }
    return 0;
}
