#!/bin/sh
#\
exec wish4.2 $0 $@

proc change_fonts {} {
	global fonts fnt

	set w .font_boxje
	catch {destroy $w}
	toplevel $w
	wm title $w "change fonts"

	frame $w.f1
	frame $w.f2
	pack $w.f1 $w.f2 -side left

	set w $w.f1
	set thefont [lindex $fonts 0]
	button $w.b1 -text "set segment font" -font $thefont \
		-command "do_change_font 1" -font $thefont 
puts stdout "segment font is ($thefont)"
	set thefont [lindex $fonts 2]
	button $w.b2 -text "set sub-segment font" -font $thefont \
		-command "do_change_font 2" -font $thefont 
puts stdout "sub-segment font is ($thefont)"
	set thefont [lindex $fonts 4]
	button $w.b3 -text "set signal font" -font $thefont \
		-command "do_change_font 3" -font $thefont 
puts stdout "signal font is ($thefont)"

	set thefont $fnt(Menus)
	button $w.b4 -text "set Menus font" -font $thefont \
		-command "do_change_font 4" -font $thefont 
	set thefont $fnt(Dialogs)
	button $w.b5 -text "set Dialogs font" -font $thefont \
		-command "do_change_font 5" -font $thefont 
	set thefont $fnt(Buttons)
	button $w.b6 -text "set Buttons font" -font $thefont \
		-command "do_change_font 6" -font $thefont 
	set thefont $fnt(Labels)
	button $w.b7 -text "set Labels font" -font $thefont \
		-command "do_change_font 7" -font $thefont 
	set thefont $fnt(Entries)
	button $w.b8 -text "set Entries font" -font $thefont \
		-command "do_change_font 8" -font $thefont 

	pack $w.b1 $w.b2 $w.b3 $w.b4 $w.b5 $w.b6 $w.b7 $w.b8

	add_font_selector .font_boxje.f2
}

proc do_change_font {i} {
	global fonts fnt curfont

	set w .font_boxje.f1
	switch $i {
		1 {
			set fonts "\"$curfont\" [lrange $fonts 1 5]"
			$w.b1 configure -font $curfont
			inkTcl_set_fonts $fonts
		}
		2 {
			set fonts "[lrange $fonts 0 1] \"$curfont\" [lrange $fonts 3 5]"
			$w.b2 configure -font $curfont
			inkTcl_set_fonts $fonts
		}
		3 {
			set fonts "[lrange $fonts 0 3] \"$curfont\" [lindex $fonts 5]"
			$w.b3 configure -font $curfont
			inkTcl_set_fonts $fonts
		}
		4 {
			set fnt(Menus) $curfont
			$w.b4 configure -font $curfont
			change_fontclass . Menubutton $curfont
		}
		5 {
			set fnt(Dialogs) $curfont
			$w.b5 configure -font $curfont
		}
		6 {
			set fnt(Buttons) $curfont
			$w.b6 configure -font $curfont
			change_fontclass . Button $curfont
		}
		7 {
			set fnt(Labels) $curfont
			$w.b7 configure -font $curfont
			change_fontclass . Label $curfont
		}
		8 {
			set fnt(Entries) $curfont
			$w.b8 configure -font $curfont
			change_fontclass . Entry $curfont
		}
	}
	write_default_resources
}

proc new_font {fp} {
	global thefnt

	if { [eof $fp] } {
		return 0
	}
	set thefnt "[gets $fp]"
	return 1
}

proc add_font_selector {w args} {
	global thefnt all_fonts env curfont

	set curfont fixed

	set fntfile "$env(UPWORKS_FONTS)"
	if { $fntfile == "" } {
		puts stderr "no fntfile available, create one using xlsfonts and set"
		puts stderr "your UPWORKS_FONTS environment variable to that file!"
		return null
	}
	if { ! [file exists $fntfile] } {
		puts stderr "unable to open fntfile $fntfile!"
		puts stderr "create one using, e.g., xlsfonts and set"
		puts stderr "your UPWORKS_FONTS environment variable to that file!"
		return null
	}
	set fp [open $fntfile]
	set idx 0
	set all_fonts fixed
	while {[new_font $fp]} {
		set all_fonts "$all_fonts \"$thefnt\""
	}
	close $fp

	frame $w.flist
	pack $w.flist -side top -expand y -fill x -pady 2m
	scrollbar $w.flist.xscroll -command "$w.flist.list xview" -orient horizontal
	scrollbar $w.flist.yscroll -command "$w.flist.list yview"
	listbox $w.flist.list -yscroll "$w.flist.yscroll set" -xscroll "$w.flist.xscroll set"\
		-setgrid 1 -height 15 -font fixed -width 100
	pack $w.flist.yscroll -side right -fill y
	pack $w.flist.xscroll -side bottom -fill x
	pack $w.flist.list -side left -expand 1 -fill both
	bind $w.flist.list <1> "change_font_item $w %W %y"

	set nfonts [llength $all_fonts]
	for {set i [expr $nfonts-1]} {$i>=0} {set i [expr $i - 1]} {
		set fnt [lindex $all_fonts $i]
		$w.flist.list insert 0 "$fnt"
	}

	label $w.l -text "$curfont" -font fixed 
	pack $w.l
	canvas $w.c -width 600 -height 100
	$w.c create text 5 25 -text 0123456789 -font "$curfont" -tags taggie -anchor w
	$w.c create text 5 50 -text abcdefghijklmnopqrstuvwxyz -font "$curfont" -tags taggie -anchor w
	$w.c create text 5 75 -text ABCDEFGHIJKLMNOPQRSTUVWXYZ -font "$curfont" -tags taggie -anchor w
	pack $w.c
	button $w.b -text Dismiss -command "destroy .font_boxje" 
	pack $w.b
}

proc change_font_item {w W y} {
	global all_fonts nfonts curfont

   set idx [$W nearest $y]
	set curfont "[lindex $all_fonts $idx]"
	$w.c itemconfigure taggie  -font $curfont
	$w.l configure -text "$curfont"
}

proc change_fontclass {w class thefont} {
	foreach c [winfo children $w] {
		if { [winfo class $c] == $class } {
			$c configure -font $thefont
		}
		change_fontclass $c $class $thefont
	}
}
