#include <string.h>
char TclLoadLibraries_Ink [] =
  "@LIBS: -L/usr/local/lib -ltk4.2 -L/usr/lib -lX11 -L/usr/local/lib -ltcl7.6 -lm_G0 -lc_G0 -L/usr/lib -lX11";
extern int DrawCursor (); 
extern int inkConfigure (); 
extern int inkCreate (); 
extern int inkDelete (); 
extern int inkDisplay (); 
extern int inkSetColors (); 
extern int inkSetFonts (); 
extern int inkSetMinMax (); 
extern int inkSetTabletInfo (); 
extern int inkUnSetMinMax (); 
extern int Ink_Init (); 
extern int hwrCreate (); 
extern int hwrDelete (); 
extern int hwrDisplay (); 
extern int hwrGetCursorInSignal (); 
extern int USE_THE_DISPLAY_FROM_HWR_BUT_WITH_DIFFERENT_SCALING (); 
extern int make_annot_xaxis (); 
extern int make_annot_yaxis (); 
extern int make_sig_axes (); 
extern int make_sig_title (); 
extern int make_sig_value (); 
extern int pretty_scale_down (); 
extern int pretty_scale_up (); 
extern int vabsCreate (); 
extern int vabsDelete (); 
extern int vabsDisplay (); 
extern int siglib_angle (); 
extern int siglib_differ (); 
extern int siglib_fir_filter (); 
extern int siglib_get_signal (); 
extern int siglib_iget_signam (); 
extern int siglib_interpolate (); 
extern int siglib_polar (); 
extern int siglib_repeated_smooth (); 
extern int siglib_rmean (); 
extern int siglib_sig_title (); 
extern int siglib_sig_type (); 
extern int siglib_sig_units (); 
extern int siglib_smooth (); 
extern int siglib_spatial_sampler (); 
extern int siglib_spatial_z_sampler (); 
static struct {
  char * name;
  int (*value)();
}dictionary [] = {
  { "DrawCursor", DrawCursor },
  { "inkConfigure", inkConfigure },
  { "inkCreate", inkCreate },
  { "inkDelete", inkDelete },
  { "inkDisplay", inkDisplay },
  { "inkSetColors", inkSetColors },
  { "inkSetFonts", inkSetFonts },
  { "inkSetMinMax", inkSetMinMax },
  { "inkSetTabletInfo", inkSetTabletInfo },
  { "inkUnSetMinMax", inkUnSetMinMax },
  { "Ink_Init", Ink_Init },
  { "hwrCreate", hwrCreate },
  { "hwrDelete", hwrDelete },
  { "hwrDisplay", hwrDisplay },
  { "hwrGetCursorInSignal", hwrGetCursorInSignal },
  { "USE_THE_DISPLAY_FROM_HWR_BUT_WITH_DIFFERENT_SCALING", USE_THE_DISPLAY_FROM_HWR_BUT_WITH_DIFFERENT_SCALING },
  { "make_annot_xaxis", make_annot_xaxis },
  { "make_annot_yaxis", make_annot_yaxis },
  { "make_sig_axes", make_sig_axes },
  { "make_sig_title", make_sig_title },
  { "make_sig_value", make_sig_value },
  { "pretty_scale_down", pretty_scale_down },
  { "pretty_scale_up", pretty_scale_up },
  { "vabsCreate", vabsCreate },
  { "vabsDelete", vabsDelete },
  { "vabsDisplay", vabsDisplay },
  { "siglib_angle", siglib_angle },
  { "siglib_differ", siglib_differ },
  { "siglib_fir_filter", siglib_fir_filter },
  { "siglib_get_signal", siglib_get_signal },
  { "siglib_iget_signam", siglib_iget_signam },
  { "siglib_interpolate", siglib_interpolate },
  { "siglib_polar", siglib_polar },
  { "siglib_repeated_smooth", siglib_repeated_smooth },
  { "siglib_rmean", siglib_rmean },
  { "siglib_sig_title", siglib_sig_title },
  { "siglib_sig_type", siglib_sig_type },
  { "siglib_sig_units", siglib_sig_units },
  { "siglib_smooth", siglib_smooth },
  { "siglib_spatial_sampler", siglib_spatial_sampler },
  { "siglib_spatial_z_sampler", siglib_spatial_z_sampler },
{0,0}
};
typedef struct Tcl_Interp Tcl_Interp;
typedef int Tcl_PackageInitProc (Tcl_Interp *);
Tcl_PackageInitProc *
TclLoadDictionary_Ink (symbol)
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
