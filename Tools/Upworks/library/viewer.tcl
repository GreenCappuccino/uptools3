# viewer.tcl  - defines routines for the viewer window

proc stop_images {l} {
	global image_arrays

	set image_arrays($l.running) 0
}

proc run_images {l direction} {
	global image_arrays segment_names
	
	set image_arrays($l.running) 1
	while {$image_arrays($l.running)} {
		set delay [$image_arrays($l.delay) get]
		after $delay
		view_next_image $l $direction
	}
}

proc get_bounds {level segnr} {
	global COLORS WIDTHS

	set NCOLORS [llength $COLORS]
	set bounds [get_children_bounds $level $segnr]
	set n [lindex $bounds 0]
	set j 0
	set subs ""
	for {set i 0} {$i<$n} {incr i} {
		set name  "[lindex $bounds [incr j]]"
		set start [lindex $bounds [incr j]]
		set end   [lindex $bounds [incr j]]
		set c     [expr $i % $NCOLORS]
		set color [lindex $COLORS $c]
		set width [lindex $WIDTHS $c]
		set subs "$subs $start $end (<([lindex $name 0])>) $color $width"
	}
	return $subs
}

proc next_image {l direction} {
	global image_arrays

	set image_arrays($l.running) 0
	busy view_next_image $l $direction
}

proc view_new_image {l segnr} {
	global image_arrays segment_names curlevel

	set curlevel $l
	set im $image_arrays($l.image)
	set data [get_segment $l $segnr]
	set subs [get_bounds $l $segnr]
	set image_arrays($l.segnr) $segnr
	set image_arrays($l.nsamples) [expr [llength $data]/3]
	set label [lindex [lindex $segment_names($l) [expr $segnr+1]] 0]

	set subs "$subs [may_view_child $l $segnr 1]"
	inkTcl_setImageAttributes $im -data "$data" -subsegments "$subs" -label "$label"
	update
}

proc view_next_image {l direction} {
	global image_arrays segment_names nimages curlevel

	set curlevel $l
	set im $image_arrays($l.image)
	set segnr $image_arrays($l.segnr)
	set n [lindex $segment_names($l) 0]
	set segnr [expr $segnr + $direction]
	if {$segnr<0} {
		set segnr [expr $n-1]
	}
	if {$segnr>=$n} {
		set segnr 0
	}
	set data [get_segment $l $segnr]
	set subs [get_bounds $l $segnr]
	set label [lindex [lindex $segment_names($l) [expr $segnr+1]] 0]
	set image_arrays($l.segnr) $segnr
	set image_arrays($l.nsamples) [expr [llength $data]/3]
	jump_to_word $l $segnr $nimages 1

	set subs "$subs [may_view_child $l $segnr 1]"

	inkTcl_setImageAttributes $im -data "$data" -subsegments "$subs" -label "$label"
	update
}

proc destroy_video {w l im} {
	global image_arrays

	set image_arrays($l.running) 0
	catch {destroy $w}
}

proc loeresize {w width height} {
	newimage configure -width [expr $width -4] -height [expr $height -4]
}

proc remember_geometry {w varname} {
	global $varname

	eval set geom_before $$varname
	set $varname [wm geometry $w]
	eval set geom $$varname
	if { $geom_before != $geom } {
		write_default_resources
	}
}

set editing_parent ""
proc create_new_image {geom im jump} {
	global fnt
	global attributes bitmaps image_arrays popups images nimages curlevel
	global segment_names hierarchies viewer_rc editing_parent
	global viewer_geometry

	if { "$editing_parent" == "" } {
		set editing_parent 0
	}
	set l     [lindex $im 0]
	set curlevel $l
	set segnr [lindex $im 1]
	set image_arrays($l.running) 0
	if {[winfo exists $popups]} {
		view_new_image $l $segnr
		jump_to_word $l $segnr $nimages $jump
		bind $popups <2> "popup_subsegments $popups $l %X %Y"
		raise $popups
		return
	}
	jump_to_word $l $segnr $nimages $jump

	catch {image delete $image_arrays($l.im)}
	catch {destroy $popups}
	set popup [create_popup .$l "UNIPEN([lindex $hierarchies [expr $l+1]] viewer)"]
	frame $popup.i
	frame $popup.f
	pack $popup.i -side top -expand yes -fill both
	pack $popup.f -side top
	set popups $popup
	set image_arrays($l.segnr) $segnr
	set subs [get_bounds $l $segnr]
	set data [get_segment $l $segnr]
	set image_arrays($l.nsamples) [expr [llength $data]/3]
	set label [lindex [lindex $segment_names($l) [expr $segnr+1]] 0]
	set atts $attributes
	if {"$subs"!=""} {
		set im [eval create_image_window ViewerRc $popup.i 1 $viewer_rc -data \{$data\}\
			-label {$label}\
			-subsegments \"$subs\"]
	} else {
		set im [eval create_image_window ViewerRc $popup.i 1 $viewer_rc -data \{$data\}\
			-label {$label}]
	}

	set image_arrays(label.$im) ""
	set image_arrays(cursor.$im) ""
	set image_arrays(nsamples.$im) ""
	set images "$images {$im}"
	set b $popup.f
	entry $b.delay -relief sunken -width 10 -font $fnt(Entries) 
	set image_arrays($l.delay) $b.delay
	set image_arrays($l.image) $im
	$b.delay insert 0 0
	button $b.rewind  -command "run_images  $l  -1"  -bitmap @$bitmaps/rewind.xbm
	button $b.prev    -command "next_image  $l  -1"  -bitmap @$bitmaps/bwdstep.xbm
	button $b.stop    -command "stop_images $l       "  -bitmap @$bitmaps/stop.xbm
	button $b.next    -command "next_image  $l  +1"  -bitmap @$bitmaps/fwdstep.xbm
	button $b.fastfwd -command "run_images  $l  +1"  -bitmap @$bitmaps/fastfwd.xbm
	button $b.dismiss -text Dismiss -command "destroy_video $popup $l [list $im]" -font $fnt(Buttons) 
	pack $b.delay $b.rewind $b.prev $b.stop $b.next\
		$b.fastfwd $b.dismiss -side left -expand n
	bind $popup <2> "popup_subsegments $popup $l %X %Y"
#	place_popup $popups $geom
	may_view_child $l $segnr 1
	my_popup_ink_atts ViewerRc $popup.i.f1 $popup.i.f2 [list $im]
	draw_pane_curtain $popup.i $popup.i.f1 $popup.i.f2
	eval wm geometry $popup $viewer_geometry
	update
	bind $popup <Configure> "remember_geometry $popup viewer_geometry"
}

proc popup_subsegments {p l x y} {
	global image_arrays nchildren hierarchies nhierarchies newfile segment_names

	set segnr $image_arrays($l.segnr)
	set hname [lindex $hierarchies [expr $l+1]]
	set cname [lindex $hierarchies [expr $l+2]]
	set name [lindex $segment_names($l) [expr $segnr+1]]
	set apie $p.apie
	catch {destroy $apie}
	menu $apie -tearoff no
	$apie add separator
	$apie add command -label "edit $hname '$name'" -command "do_edit_segment $l $segnr +$x+$y"
	$apie add separator

	if { $l == [expr $nhierarchies-1]} {
		$apie add command -label "add a new hierarchy" -command {\
			DialogEntryBox "WOW THIS IS COOL!!! You are trying to\
				view a new hierarchy, if you wish to add one,\
				fill in these entries and press Ok (else press <dismiss>)\
				A new file will be created with the extra hierarchy you enter.\
				And this file will be opened."\
				2 "filename hierarchy" "tmp.dat new-hierarchy" "do_view_unipen"
			}
		tk_popup $apie $x $y
		return
	}

	set children [get_children $l $segnr]
	set nchildren [lindex $children 0]

	for {set i 1} {$i<=$nchildren} {incr i} {
		$apie add command -label "edit $cname '[lindex $children $i]'"\
			-command "do_edit_child +$x+$y $l $segnr [lindex [lindex $children $i] 0]"
	}
	if {$nchildren==0} {
		$apie add command -label "add new $cname segment"\
			-command "do_add_first_segment +$x+$y $l $segnr"
	}
	tk_popup $apie $x $y
	update
}

proc loe_blink {w} {
	global thelevel latest_window


	if {![winfo exists $w]} {
		stop_images $thelevel
		return
	}
	catch {$latest_window configure -bg white}
	$w configure -bg lightblue
	set latest_window $w
	update
}

proc jump_to_word {l s n jump} {
	global ncols nimages image_arrays

	if {$nimages<$s} {
#		DisplayMessage "segment $s: is not visible yet (only up to $nimages)!"
		set image_arrays($l.running) 0
		return
	}
	if { $ncols == 0 || $s == 0 } {
		loe_blink .f.f1.c.f.l_0
		return
	}
	set nrows [expr $n / $ncols]
	if { $nrows == 0 } {
		loe_blink .f.f1.c.f.l_$s
		return
	}
	set row [expr $s / $ncols]
	if { $jump } {
		set col [expr $s % $ncols]
		set xfraction [expr 1.0 * $col / $ncols]
		set yfraction [expr 1.0 * ($row-1.0) / $nrows]
		.f.f1.c xview moveto $xfraction
		.f.f1.c yview moveto $yfraction
	}
	loe_blink .f.f1.c.f.l_$s
}

proc goto_word {l w} {
	global segment_names
	global segments_are_sorted sorted_segments

	set segnr     [eval $w.s.itemnr get]
	set segnr     [eval lindex $segnr 0]
	set segname   [lindex $segment_names($l) [expr $segnr+1]]

	set oldsegnr $segnr

	if {$segments_are_sorted} {
		set item [lindex $sorted_segments $segnr]
		set segname [lindex $item 0]
		set segnr [lindex $item 2]
	}

	set n [lindex $segment_names($l) 0]

	$w.flist.list selection clear 0 end
	$w.flist.list selection set $oldsegnr

	create_new_image null "$l $segnr" 1
}

proc do_exec { data } {
	puts stdout "$data"
}

proc view_hierarchy {l destroy_containers thesegnr} {
	global attributes segment_names ncols hierarchies hierarchy opening viewing
	global nimages image_arrays newfile canvas_images frames
	global use_same_scale thelevel
	global some_image
	global hierarchy_finished main_rc

	wm title . "'$newfile' UNIPEN ([lindex $hierarchies $l] LEVELS)"
	reset_segments

	set thelevel $l
	set j -1
	set i 0
	set f .f.f1.c.f
	set nimages 0
	catch {stop_images $thelevel}

	if {$use_same_scale} {
		set bounds [get_all_bounds $l]
		eval ink_set_min_max $bounds
	} else {
		ink_unset_min_max
	}

	set hierarchy_finished 0

#	inactivate_topbar_buttons $l
	create_containers $destroy_containers
	set n [lindex $segment_names($l) 0]
	set viewing 1

	init_progress $n "loading $n segments"

	while {!$opening && $i<$n && $hierarchy==$l} {
		set data [get_segment $l $i]
		if { $data == -1 } {
			puts stderr "error while getting data for $l $i"
			break
		}
		set label [lindex [lindex $segment_names($l) [expr $i+1]] 0]
		if { "$label" == "" } {
			set label "????"
		}
		set lab $f.l_$i
		set col [expr $i % $ncols]
		set row [expr $i / $ncols]
		set subs [get_bounds $l $i]
		if {"$subs"!=""} {
			set ink_image [eval inkTcl_createImage 0 $f $lab [list "$l $i"] $main_rc [list -data $data]\
				-subsegments \"$subs\" -label {$label} -_Show_Signals 0]
		} else {
			set ink_image [eval inkTcl_createImage 0 $f $lab [list "$l $i"] $main_rc [list -data $data]\
				-label {$label} -_Show_Signals 0]
		}
		grid $lab -column $col -row $row
		set some_image $ink_image
		bind $lab <1> "do_exec \"$data\""
		bind $lab <2> "busy create_new_image %X+%Y [list $ink_image] 0"
		bind $lab <3> "my_popup_ink_atts MainRc .f.f1 .f.f2 [list $ink_image]"
#		if { $j==0 } {
			bind . <3> "my_popup_ink_atts MainRc .f.f1 .f.f2 [list $ink_image]"
#		}
		set canvas_images "$canvas_images {$ink_image}"
		if { $i == $thesegnr } {
			jump_to_word $l $i [expr $i+1] 1
		}
		incr i
		incr nimages
		progress $i
		update
	}
	kill_progress
	if {$opening} {
		set opening 0
		view_unipen $newfile
	}
	set viewing 0
	set hierarchy_finished 1
#	activate_topbar_buttons $l
}
