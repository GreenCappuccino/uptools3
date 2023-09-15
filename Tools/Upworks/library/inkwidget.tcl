#!/bin/sh
#\
exec wish $0 $@

#
# exported routines
#
# inkTcl_set_colors  colors            - initializes the colors
# inkTcl_set_fonts  fonts              - initializes the fonts
# inkTcl_createImage attributes        - creates an image with given attributes with 
# inkTcl_destroyImage level idx        - destroys an image with given level and idx
# inkTcl_setImageAttributes attributes - sets the given atributes
# inkTcl_getImageAttributes            - gets all image attributes
#

proc inkTcl_set_colors { colors } {
	global COLORS ORIGINAL_COLORS WIDTHS

	set ORIGINAL_COLORS $colors
	set WIDTHS ""
	foreach c $ORIGINAL_COLORS {
		set WIDTHS "$WIDTHS 3"
	}
	set COLORS $ORIGINAL_COLORS
	eval ink_set_colors $colors
}

proc inkTcl_set_fonts { fonts } {
	eval ink_set_fonts $fonts
}

proc inkTcl_createImage { ispopup w lab name args } {
	global label_windows

	if { $ispopup } {
		set w $w.i
	}
	if { ! [winfo exists $w ] } {
		DisplayMessage "window $w does not exist! creating it as a frame!"
		frame $w
		pack $w -side top
	}
	set im [eval image create ink [list $name] $args]
	if {[winfo exists $lab]} {
		$lab configure -image $im -bg white -borderwidth 5
	} else {
		label $lab -image $im -bg white -borderwidth 5
	}
	bind $lab <Enter> "focus [list $lab]"
	bind $lab <Left> "walk_cursor [list $im] -1"
	bind $lab <Right> "walk_cursor [list $im] +1"
	set label_windows($im) $w
	return $name
}

proc inkTcl_setImageAttributes { im_name args } {
	global image_arrays

	eval [list $im_name] configure $args
	set im [list $im_name]
	set atts [get_image_resources $im "label cursor nsamples"]
	if {[winfo exists $image_arrays(label.$im)]} {
		set e $image_arrays(label.$im)
		$e delete 0 end
		$e insert 0 [lindex $atts 0]
		set e $image_arrays(cursor.$im)
		$e delete 0 end
		$e insert 0 [lindex $atts 1]
		set e $image_arrays(nsamples.$im)
		$e delete 0 end
		$e insert 0 [lindex $atts 2]
	}
}

proc ink_button2id {b} {
	set i [expr [string first .b $b ]+2]
	set j [expr [string first B $b ]+1]
	set l [string range $b $i [expr $j-2]]
	set s [string range $b $j end]
	return "$l $s"
}

proc place_popup {p geom} {
	update
	if { "$geom" != "null" } {
		set xy [wm geometry $p]
		set idx [string first "+" $xy]
		set xy [string range $xy 0 $idx]
		wm geometry $p $xy$geom
	}
}

proc popup_ink_atts {im} {
	global UW_all

	set atts [$im configure]
	set n [llength $atts]
	set labels  ""
	set entries ""
	set kinds   ""
	set label ""

	foreach s $atts {
		set name  [string range [lindex $s 0] 1 end]
		if { [name_not_allowed $name] } {
			set n [expr $n - 1]
			continue
		}
		set c [string index $name 0]
		if { $c == "_" } {
			set k "UW_all($name) \{ \}"
			set kinds "$kinds \"$k\""
			set UW_all($name) [lindex $s 4]
		} else {
			set kinds "$kinds \"0\""
		}
		set labels "$labels \{$name\}"
		set entries "$entries \{[lindex $s 4]\}"
	}
	set newpopup [NewDialogEntryBox . "attributes ALL " "$n" "$labels" "$entries" "$kinds"\
		"set_ink_attributes MainRc ALL $n \{$labels\} UW_all"]
}

proc name_not_allowed { name args } {
	if { "$name" == "data"          || "$name" == "-data"            ||
		"$name" == "subsegments"     || "$name" == "-subsegments"     ||
		"$name" == "request_cursor_x"|| "$name" == "-request_cursor_x"||
		"$name" == "request_cursor_y"|| "$name" == "-request_cursor_y" ||
		"$name" == "foreground"|| "$name" == "-foreground" ||
		"$name" == "background"|| "$name" == "-background"
		} {
		return 1
	} else {
		for {set i 0} {$i<[llength $args]} {incr i} {
			set check [lindex $args $i]
			if { "$name" == "$check" || $name == "-$check" } {
				return 1
			}
		}
		return 0
	}
}

proc create_default_resources {w} {
	global upworks_rc default_colors default_fonts fnt
	global main_geometry viewer_geometry manipulator_geometry

	set fname [eval $w.e(0) get]
	set fp [open $fname w]
	set upworks_rc $fname
	catch {destroy $w}
	puts $fp "Colors {"
	for {set c 0} {$c<[llength $default_colors]} {incr c} {
		puts $fp " [lindex $default_colors $c]"
	}
	puts $fp "}"

	puts $fp "SegmentFont {"
	puts $fp " [lindex $default_fonts 0]"
	puts $fp " [lindex $default_fonts 1]"
	puts $fp "}"
	puts $fp "SubsegmentFont {"
	puts $fp " [lindex $default_fonts 2]"
	puts $fp " [lindex $default_fonts 3]"
	puts $fp "}"
	puts $fp "SignalFont {"
	puts $fp " [lindex $default_fonts 4]"
	puts $fp " [lindex $default_fonts 5]"
	puts $fp "}"
	puts $fp "MenusFont {"
	puts $fp " $fnt(Menus)"
	puts $fp "}"
	puts $fp "DialogsFont {"
	puts $fp " $fnt(Dialogs)"
	puts $fp "}"
	puts $fp "ButtonsFont {"
	puts $fp " $fnt(Buttons)"
	puts $fp "}"
	puts $fp "LabelsFont {"
	puts $fp " $fnt(Labels)"
	puts $fp "}"
	puts $fp "EntriesFont {"
	puts $fp " $fnt(Entries)"
	puts $fp "}"

	puts $fp "MainGeometry {"
	puts $fp " $main_geometry"
	puts $fp "}"
	puts $fp "ViewerGeometry {"
	puts $fp " $viewer_geometry"
	puts $fp "}"
	puts $fp "ManipulatorGeometry {"
	puts $fp " $manipulator_geometry"
	puts $fp "}"

	set im [image create ink]
	set resources [eval $im configure]
	set all_rc "MainRc ViewerRc ManipulatorRc"
	for {set i 0} {$i<[llength $all_rc]} {incr i} {
		puts $fp "[lindex $all_rc $i] \{"
		foreach s $resources {
			set name  [lindex $s 0]
			if { [name_not_allowed $name label nsamples cursor] } {
				continue
			}
			set value "[lindex $s 4]"
			puts $fp " $name $value"
		}
		puts $fp "\}"
	}
	close $fp
	read_default_resources 
	image delete $im
}

proc read_resources {fp resource_name use_quotes} {
	global upworks_rc

	set resources ""
	regsub "\{" [gets $fp] "" line
	set name "[lindex $line 0]"
	if { $name != $resource_name } {
		puts stderr "error reading resources $resource_name from $upworks_rc"
		puts stderr "got $name instead of $resource_name"
		puts stderr "--- you can solve this as follows ---"
		puts stderr " 1) if you know of a valid .unipenrc file, use that as"
		puts stderr "    UNIPEN_RESOURCE_FILE environment variable, or"
		puts stderr " 2) rename or remove $upworks_rc and restart upworks"
		puts stderr "    A dialog-box will appear asking for permission to create"
		puts stderr "    a new, valid .unipenrc file............."
		exit 1
	}
	while { ! [eof $fp] } {
		set line [gets $fp]
		if { $line == "\}" } {
			return $resources
		}
		if { $use_quotes } {
			set resources "$resources \"$line\""
		} else {
			set resources "$resources $line"
		}
	}
	puts stderr "error reading resources!"
}

proc read_default_resources {} {
	global main_rc viewer_rc manipulator_rc upworks_rc fnt
	global main_geometry viewer_geometry manipulator_geometry

	set fp                   [open $upworks_rc r]
	set cols                 [read_resources $fp Colors 0]

	set segf                 [read_resources $fp SegmentFont 1]
	set subf                 [read_resources $fp SubsegmentFont 1]
	set sigf                 [read_resources $fp SignalFont 1]
	set fnts                 "$segf $subf $sigf"
	set fnt(Menus)           [read_resources $fp MenusFont 1]
	set fnt(Dialogs)         [read_resources $fp DialogsFont 1]
	set fnt(Buttons)         [read_resources $fp ButtonsFont 1]
	set fnt(Labels)          [read_resources $fp LabelsFont 1]
	set fnt(Entries)         [read_resources $fp EntriesFont 1]
	set main_geometry        [read_resources $fp MainGeometry 0]
	set viewer_geometry      [read_resources $fp ViewerGeometry 0]
	set manipulator_geometry [read_resources $fp ManipulatorGeometry 0]

	set main_rc              [read_resources $fp MainRc 0]
	set viewer_rc            [read_resources $fp ViewerRc 0]
	set manipulator_rc       [read_resources $fp ManipulatorRc 0]
	close $fp
	return "\{$cols\} \{$fnts\}"
}

proc write_default_resources {} {
	global main_rc viewer_rc manipulator_rc upworks_rc colors fonts fnt
	global main_geometry viewer_geometry manipulator_geometry

	set fp [open $upworks_rc w]

# write colors and fonts for segment,subsegment and signal
	puts $fp "Colors {"
	foreach c $colors {
		puts $fp " $c"
	}
	puts $fp "}"
	puts $fp "SegmentFont {"
	puts $fp "[lindex $fonts 0]"
	puts $fp "[lindex $fonts 1]"
	puts $fp "}"
	puts $fp "SubsegmentFont {"
	puts $fp "[lindex $fonts 2]"
	puts $fp "[lindex $fonts 3]"
	puts $fp "}"
	puts $fp "SignalFont {"
	puts $fp "[lindex $fonts 4]"
	puts $fp "[lindex $fonts 5]"
	puts $fp "}"

	set all_rc "MenusFont DialogsFont ButtonsFont LabelsFont EntriesFont\
		MainGeometry ViewerGeometry ManipulatorGeometry\
		MainRc ViewerRc ManipulatorRc"
	set resources "
		{$fnt(Menus)} {$fnt(Dialogs)} {$fnt(Buttons)} {$fnt(Labels)} {$fnt(Entries)}\
		{$main_geometry} {$viewer_geometry} {$manipulator_geometry}\
		{$main_rc} {$viewer_rc} {$manipulator_rc}"
	for {set i 0} {$i<[llength $all_rc]} {incr i} {
		puts $fp "[lindex $all_rc $i] \{"
		set s [lindex $resources $i]
		set n [expr ([llength $s]+1)/2]
		for {set r 0} {$r<$n} {incr r} {
			set name  [lindex $s [expr 2*$r]]
			if { [name_not_allowed $name label nsamples cursor] } {
				continue
			}
			set value "[lindex $s [expr 2*$r+1]]"
			puts $fp " $name $value"
		}
		puts $fp "\}"
	}
	close $fp
}

proc set_ink_attributes {levelname im n labels VAR w} {
	global $VAR images canvas_images attributes VARMAIN
	global main_rc viewer_rc manipulator_rc

	set VAR $VAR
	set new_attributes ""
	for {set i 0} {$i<$n} {incr i} {
		set name  [lindex $labels $i]
		set c [string index $name 0]
		if { $c == "_" } {
			set var $VAR\($name\)
			eval set value $$var
		} else {
			set value [eval $w.box.e($i) get]
		}
		if { [string range $name 0 0] != "*" &&
				$value != "" && $name != "" && "$name" != "label" } {
			set new_attributes "$new_attributes -$name $value"
		}
	}
	if { $im == "ALL" } {
#		set all_images "$canvas_images $images"
		set all_images "$canvas_images"
		foreach im $all_images {
			catch {eval [list $im] configure $new_attributes}
		}
		set attributes $new_attributes
	} else {
		catch {eval [list $im] configure $new_attributes}
	}
	switch $levelname {
		MainRc {
			set main_rc " $new_attributes "
		}
		ViewerRc {
			set viewer_rc " $new_attributes "
		}
		ManipulatorRc {
			set manipulator_rc " $new_attributes "
		}
	}
	write_default_resources
}

proc pulldown_media {butt im x y} {
	set width [lindex [$im configure -width] 4]
	set height [lindex [$im configure -height] 4]
	set minlength 100
	set apie $butt.apie
	catch {destroy $apie}
	menu $apie -tearoff no
	$apie add command -label "current geometry = ${width}x$height"
	$apie add separator
	$apie add command -label "CHAR geometry (1x1)x${minlength}"\
		-command "setgeom [list $im] $minlength 1 1"
	$apie add command -label "WORD geometry (5x2)x${minlength}"\
		-command "setgeom [list $im] $minlength 5 2"
	$apie add command -label "LINE geometry (10x1)x${minlength}"\
		-command "setgeom [list $im] $minlength 10 1"
	$apie add command -label "PAR geometry (4x3)x${minlength}"\
		-command "setgeom [list $im] $minlength 4 3"
	$apie add command -label "PAGE geometry (1x2)x${minlength}"\
		-command "setgeom [list $im] $minlength 1 2"
	tk_popup $apie $x $y
}

proc setgeom {im w x y} {
	$im configure -width [expr $x*$w] -height [expr $y*$w]
}

proc get_image_resources {im names} {
#	set resources [eval [list $im] configure]
	set resources [eval [list $im] configure]
	set result ""
	foreach n $names {
		foreach r $resources {
			set name [string range [lindex $r 0] 1 end]
			if { "$name" == "$n" } {
				set value "[lindex $r 4]"
				set result "$result \{$value\}"
			}
		}
	}
	return $result
}

proc walk_cursor {im dir} {
	set current   [get_image_resources $im "cursor nsamples"]
	set cursor    [lindex $current 0]
	set nsamples  [lindex $current 1]
	set cursor [expr $cursor + $dir]
	if { $cursor < 0 } {
		set cursor [expr $nsamples-1]
	} else {
		set cursor [expr $cursor % $nsamples]
	}
	inkTcl_setImageAttributes $im -cursor $cursor
}

proc create_canvas_with_scrolls {w xdir ydir} {
	canvas $w.c -xscrollcommand "do_scroll $w.scrollx $xdir x" -yscrollcommand "do_scroll $w.scrolly $ydir y"
	scrollbar $w.scrollx -orient horizontal -command "$w.c xview"
	scrollbar $w.scrolly -orient vertical -command "$w.c yview"
	pack $w.scrollx -side $xdir -fill x
	pack $w.scrolly -side $ydir -fill y
	pack $w.c -fill both -side bottom -expand yes
	frame $w.c.f
	$w.c create window 0 0 -window $w.c.f -anchor nw
	bind $w.c.f <Configure> "$w.c configure -scrollregion \"0 0 %w %h\";\
		$w.c.f configure -width %w -height %h"
	return $w.c
}


proc MyDialogEntryBox { p im msg nentries labels entries kinds callback} {
	global VAR[lindex $im 0] VARMAIN
	global fnt

	set VAR VAR[lindex $im 0]
	if {[winfo exists $p.c]} {
		destroy $p.c
		destroy $p.scrollx
		destroy $p.scrolly
	}
	set cv [create_canvas_with_scrolls $p bottom right]
	catch {destroy $cv.f.box}
	newentrybox $cv.f [list $im] $nentries $labels $entries $kinds "$callback $VAR"
	button $cv.f.ok -text Ok -command "$callback $VAR $cv.f" -font $fnt(Buttons) 
	pack $cv.f.ok -side top -fill both
}

proc my_popup_ink_atts { levelname f1 f2 im} {
	global VAR[lindex $im 0] VARMAIN

	if { "$levelname" == "MainRc" } {
		set VAR VARMAIN
	} else {
		set VAR VAR[lindex $im 0]
		set VAR $VAR
	}

	pack $f2 -side left -fill both
	pane $f1 $f2 -dynamic 1
	set w $f2

	set atts [$im configure]
	set n [llength $atts]
	set labels  ""
	set entries ""
	set kinds   ""
	set entry_idx 0
	set label ""

	foreach s $atts {
		set name  [string range [lindex $s 0] 1 end]
		if { "$name" == "label" } {
			set label \{[lindex $s 4]\}
		}
		if { [name_not_allowed $name] } {
			set n [expr $n - 1]
			continue
		}
		set c [string index $name 0]
		if { $c == "_" } {
			set k "${VAR}($name) \{ \}"
			set kinds "$kinds \"$k\""
			set ${VAR}($name) [lindex $s 4]
		} else {
			set kinds "$kinds \"0\""
		}
		set labels "$labels \{$name\}"
		set entries "$entries \{[lindex $s 4]\}"
	}

	if { "$levelname" == "MainRc" } {
		set newpopup [MyDialogEntryBox $w MAIN\
			"attributes $label " "$n" "$labels" "$entries" " $kinds"\
			"set_ink_attributes MainRc ALL $n \{$labels\}"]
	} else {
		set newpopup [MyDialogEntryBox $w [list $im]\
			"attributes $label " "$n" "$labels" "$entries" " $kinds"\
			"set_ink_attributes $levelname [list $im] $n \{$labels\}"]
	}
}

proc create_image_window {levelname w with_scroll_bars args} {
	global images

	catch {destroy $w.f1.c $w.f2 $w.f1.scrollx $w.f1.scrolly}

	frame $w.f1
	frame $w.f2 -width 20
	pack $w.f1 -side left -expand yes -fill both

	if { $with_scroll_bars } {
		set cv [create_canvas_with_scrolls $w.f1 bottom left]
	} else {
		set cv $w.c
	}

	set im [eval image create ink $args]
	set images "$images [list $im]"
	set lab $cv.f.l
	label $lab -image $im -relief sunken -bg white 

	pack $cv $lab
	bind $lab <Shift-P> "postscript_dump $cv"
	bind $lab <3> "my_popup_ink_atts $levelname $w.f1 $w.f2 [list $im]"
	bind $lab <1> "may_set_cursor_pos $im %x %y"
	bind $lab <Enter> "focus [list $lab]"
	bind $lab <Left> "walk_cursor [list $im] -1"
	bind $lab <Right> "walk_cursor [list $im] +1"
	pack $lab -side left -expand yes -fill both
	return $im
}

proc may_set_cursor_pos {im x y} {
	set cpos [$im configure -request_cursor_x $x -request_cursor_y $y]
	$im configure -cursor $cpos -request_cursor_x -1 -request_cursor_y -1
}

proc do_scroll {w dir pos args} {
	global keep_scrolls_alive

	eval $w set $args
}

proc delete_image_window {w} {
	destroy $w.c $w.scrollx $w.scrolly $w
	frame $w
	pack $w -side left -fill both
}
