# manipulator.tcl  - defines routines for the manipulator window

global editing_parent

proc may_view_child {l segnr jump} {
	global curchild curlevel childimage nchildren view_start view_end
	global image_arrays editing_parent sel_start sel_end

	if { [winfo exists .child_container] && $editing_parent } {
		busy do_next_parent null
		return
	}
	if {[winfo exists .child_container]} {
		set curlevel $l
		set segnr $image_arrays($l.segnr)
		set children [get_children $l $segnr]
		set nchildren [lindex $children 0]
		if { $nchildren == 0 } {
			inkTcl_setImageAttributes $childimage -data "0 0 -1"
			return
		}
		if { $jump } {
			set curchild 0
		}
		set cdata "[get_child_signal $l $segnr $curchild]"
		set bounds "[get_child_bounds $l $segnr $curchild]"
		set view_start [lindex $bounds 0]
		set view_end [lindex $bounds 1]
		set label [lindex $bounds 2]
		set dw [expr [get_image_resources $childimage dwidth]+2]
		set sel_start 0
		set sel_end [expr [llength $cdata]/3 - 1]
		set subs "$view_start $view_end . selected_color $dw"
		inkTcl_setImageAttributes $childimage -data "$cdata" -label $label\
			-subsegments "$sel_start $sel_end (<($label)>) selected_color $dw"

		may_update_entry
		update_entries 0
		set dw [expr [get_image_resources "$l $segnr" dwidth]+2]
		set subs "$view_start $view_end (<($label)>) selected_color $dw"
		return $subs
	}
	return ""
}

proc do_add_first_segment {geom l s} {
	global curchild nchildren editing_parent
	global unipen_saved

	set editing_parent 0
	add_first_segment $l $s
	set nchildren 1
	set curchild 0
	show_child $geom $l $s 0
	update_entries 1
	may_update_entry
	set unipen_saved 0
}

proc label_child {geom l s idx dir} {
	global nchildren curchild

	set result [add_child $l $s $idx $dir]
	if {! $result} {
		do_loe_beep
		return
	}
	if { $dir == 0 } {
		incr curchild
	}
	incr nchildren
	show_child $geom $l $s $curchild
}

	set basic_jumpsizes "{1 sample}\
		{10 samples}\
		{100 samples}\
		{1000 samples}\
		Y-extrema\
		penstreams"
	set view_start 0
	set view_end   0

set jump_size(0) {1 sample}
set jump_size(1) {penstreams}
set jump_size(2) {1 sample}
set jump_size(3) {penstreams}

proc set_jump_sizes {l} {
	global jumpsizes basic_jumpsizes hierarchies nhierarchies
	global jump_size

	if { "$l" == "all" } {
			set jumpsizes "$basic_jumpsizes [lrange $hierarchies 1 end]"
	} else {
		if { $l < [expr $nhierarchies-1]} {
			set jumpsizes "$basic_jumpsizes [lrange $hierarchies [expr $l+2] end]"
		} else {
			set jumpsizes "$basic_jumpsizes"
		}
	}
}


proc do_loe_beep {} {
	puts -nonewline stdout "\a"
	flush stdout
}

proc get_c_bounds {m c_im} {
	global image_arrays view_start view_end curlevel sel_start sel_end
	global jumpsizes jump_size
	global child_del
	global editing_parent

	if { $editing_parent } {
		get_s_bounds $m $c_im
		return
	}

	eval set n_before [get_image_resources $c_im nsamples]
	switch $m {
		0 { set jumpsize $jump_size(0) }
		1 { set jumpsize $jump_size(0) }
		2 { set jumpsize $jump_size(1) }
		3 { set jumpsize $jump_size(1) }
		4 { set jumpsize $jump_size(2) }
		5 { set jumpsize $jump_size(2) }
		6 { set jumpsize $jump_size(3) }
		7 { set jumpsize $jump_size(3) }
		default { puts stdout "error: M=$m must be one of \[0...7\]!" }
	}

	set l $curlevel
	set s $image_arrays($l.segnr)
	set cur_del [lindex [$child_del configure -text] 4]
	set result [get_child_markers $l $s $m $jumpsize $cur_del]
	if {$view_start==[lindex $result 0] && $view_end==[lindex $result 1]} {
		do_loe_beep
		return
	}
	set view_start [lindex $result 0]
	set view_end   [lindex $result 1]
	set del    [lindex $result 2]
	$child_del configure -text $del
	set im $image_arrays($l.image)
	set dwidth [expr [get_image_resources $im dwidth]+2]
	inkTcl_setImageAttributes $im -subsegments "$view_start $view_end (<(...)>) selected_color $dwidth"
	set data [get_child_data $l $s $view_start $view_end]
	adjust_and_set_markers $m $n_before $c_im $data
}

proc adjust_and_set_markers {m n_before c_im data} {
	global sel_start sel_end view_start view_end

	set n_after [expr [llength $data]/3]
	set diff [expr $n_after - $n_before]
	if { $diff < 0 } {
		set diff [expr -1 * $diff]
	}
	set n_after [expr $n_after - 1]
	switch $m {

		0 {
			set sel_start [expr $sel_start + $diff]
			set sel_end [expr $sel_end + $diff]
		}

		1 {
			set sel_start [expr $sel_start - $diff]
			set sel_end [expr $sel_end - $diff]
			if { $sel_start < 0 } {
				set sel_start 0
			}
			if { $sel_end < $sel_start } {
				set sel_end $sel_start
			}
			adjust_selection_delineation 3
		}

		2 {
			set sel_start [expr $sel_start + $diff]
			set sel_end   [expr $sel_end + $diff]
		}

		3 {
			set sel_start [expr $sel_start - $diff]
			set sel_end [expr $sel_end - $diff]
			if { $sel_start < 0 } {
				set sel_start 0
			}
			if { $sel_end < $sel_start } {
				set sel_end $sel_start
			}
			adjust_selection_delineation 3
		}

		4 {
			if { $sel_end > $n_after } {
				set sel_end $n_after
				adjust_selection_delineation 4
			}
		}

		5 {
		}

		6 {
			if { $sel_end > $n_after } {
				set sel_end $n_after
				adjust_selection_delineation 4
			}
		}

		7 {
		}
	}
	set dwidth [expr [get_image_resources $c_im dwidth]+1]
	inkTcl_setImageAttributes $c_im -data $data -subsegments "$sel_start $sel_end (<(...)>) selected_color $dwidth"
}

set sel_start -1
set sel_end   -1

proc may_decrease_cursor_left {} {
	global sel_start sel_end childimage

	if {$sel_start<=0} {
		set sel_start 0
	} else {
		set sel_start [expr $sel_start - 1]
	}
	if {$sel_end>$sel_start} {
		set dw [expr [get_image_resources $childimage dwidth]+2]
		$childimage configure -cursor $sel_start -subsegments "$sel_start $sel_end (<(...)>) selected_color $dw"
		adjust_selection_delineation 2
	} else {
		$childimage configure -cursor $sel_start
	}
}

proc may_increase_cursor_left {} {
	global sel_start sel_end childimage

	if {$sel_start<0} {
		set sel_start 0
	} else {
		set sel_start [expr $sel_start + 1]
		if  { $sel_end < $sel_start } {
			set sel_start [expr $sel_start - 1]
			do_loe_beep
			return
		}
	}
	if {$sel_end>$sel_start} {
		set dw [expr [get_image_resources $childimage dwidth]+2]
		$childimage configure -cursor $sel_start -subsegments "$sel_start $sel_end (<(...)>) selected_color $dw"
		adjust_selection_delineation 2
	} else {
		$childimage configure -cursor $sel_start
	}
}

proc may_decrease_cursor_right {} {
	global sel_start sel_end childimage

	if {$sel_end<0} {
		set sel_end 0
	} else {
		set sel_end [expr $sel_end - 1]
		if  { $sel_end < $sel_start } {
			set sel_end [expr $sel_end + 1]
			do_loe_beep
			return
		}
	}
	if {$sel_end>$sel_start && $sel_start!=-1} {
		set dw [expr [get_image_resources $childimage dwidth]+2]
		$childimage configure -cursor $sel_end -subsegments "$sel_start $sel_end (<(...)>) selected_color $dw"
		adjust_selection_delineation 2
	} else {
		$childimage configure -cursor $sel_end
	}
}

proc may_increase_cursor_right {} {
	global sel_start sel_end childimage

	if {$sel_end<0} {
		set sel_end 0
	} else {
		eval set nsamples [get_image_resources [list $childimage] nsamples]
		set sel_end [expr $sel_end + 1]
		if  { $sel_end >= $nsamples } {
			set sel_end [expr $nsamples - 1]
			do_loe_beep
			return
		}
	}
	if {$sel_end>$sel_start && $sel_start!=-1} {
		set dw [expr [get_image_resources $childimage dwidth]+2]
		$childimage configure -cursor $sel_end -subsegments "$sel_start $sel_end (<(...)>) selected_color $dw"
		adjust_selection_delineation 2
	} else {
		$childimage configure -cursor $sel_end
	}
}

proc do_save_selection {} {
	global sel_start sel_end view_start view_end
	global curchild curlevel childimage nchildren
	global image_arrays editing_parent
	global child_sel hierarchies segment_names

	set l $curlevel
	set s $image_arrays($l.segnr)

	# some variables for logging
	set del [lindex [$child_sel configure -text] 4]
	set phier [lindex $hierarchies [expr $l+1]]\[$s\]

	if { $editing_parent } {
		busy do_save_segment $curlevel
		log_upworks "saved $phier\ $del ? \"[lindex $segment_names($l) [expr $s+1]]\""
		return
	}
	if { $sel_start==-1 || $sel_end==-1 } {
		do_loe_beep
		return
	}
	if { [busy do_save_child] } {
		set bounds "[busy get_child_bounds $l $s $curchild]"
		set offset [lindex $bounds 0]
		set view_start [expr $sel_start+$offset]
		set view_end   [expr $sel_end+$offset]
		set chier [lindex $hierarchies [expr $l+2]]\[$curchild\]
		log_upworks "saved $chier in $phier $del ? \"[lindex $bounds 2]\""
	}
	busy next_child 0
}

proc adjust_selection_delineation {d} {
	global curchild curlevel image_arrays
	global view_start view_end sel_start sel_end child_sel child_del
	global editing_parent

	set l $curlevel
	set s $image_arrays($l.segnr)
	switch $d {
		0 {
			set del [lindex [$child_sel configure -text] 4]
			$child_del configure -text $del
		}
		1 {
			set del [lindex [$child_del configure -text] 4]
			$child_sel configure -text $del
		}
		2 {
			set sel    [lindex [$child_sel configure -text] 4]
			set offset [lindex [$child_del configure -text] 4]
			if { $editing_parent} {
				set del [get_parent_delineation $l $s $sel_start $sel_end]
			} else {
				set del [get_child_delineation $l $s $curchild $sel_start $sel_end $offset]
			}
			if { "$del" == "$sel" } {
				do_loe_beep
			} else {
				$child_sel configure -text $del
			}
		}
		3 {
			set del    [lindex [$child_del configure -text] 4]
			set sel    [lindex [$child_sel configure -text] 4]
			set idx    [expr [string first "-" $del] -1]
			set head   [string range $del 0 $idx]
			set idx    [expr 1 + [string first "-" $sel]]
			set tail   [string range $sel $idx end]
			$child_sel configure -text "$head-$tail"
		}
		4 {
			set del    [lindex [$child_del configure -text] 4]
			set sel    [lindex [$child_sel configure -text] 4]
			set idx    [expr [string first "-" $sel] -1]
			set head   [string range $sel 0 $idx]
			set idx    [expr 1 + [string first "-" $del]]
			set tail   [string range $del $idx end]
			$child_sel configure -text "$head-$tail"
		}
	}
}

proc select_current_view {} {
	global childimage sel_start sel_end

	set sel_end [expr [get_image_resources $childimage nsamples]-1]
	set sel_start 0
	set dw [expr [get_image_resources $childimage dwidth]+2]
	$childimage configure -subsegments "$sel_start $sel_end (<(...)>) selected_color $dw"
	adjust_selection_delineation 1
}

proc may_set_select_start {im x y} {
	global sel_start sel_end

	eval set cpos [[list $im] configure -request_cursor_x $x -request_cursor_y $y]
	if { $sel_end != -1 } {
		if { $cpos < $sel_end } {
			eval set sel_start $cpos
			set dw [expr [get_image_resources $im dwidth]+2]
			$im configure -cursor $cpos -subsegments "$sel_start $sel_end (<(...)>) selected_color $dw"\
				-request_cursor_x -1 -request_cursor_y -1
			adjust_selection_delineation 2
		} else {
			do_loe_beep
			return
		}
	} else {
		eval set sel_start $cpos
		$im configure -cursor $cpos -request_cursor_x -1 -request_cursor_y -1
	}
}

proc may_set_select_end {im x y} {
	global sel_start sel_end

	eval set cpos [[list $im] configure -request_cursor_x $x -request_cursor_y $y]
	if { $sel_start != -1 } {
		if { $cpos > $sel_start } {
			eval set sel_end $cpos
			set dw [expr [get_image_resources $im dwidth]+2]
			$im configure -cursor $cpos -subsegments "$sel_start $sel_end (<(...)>) selected_color $dw"\
				-request_cursor_x -1 -request_cursor_y -1
			adjust_selection_delineation 2
		} else {
			do_loe_beep
			return
		}
	} else {
			eval set sel_end $cpos
			$im configure -cursor $cpos -request_cursor_x -1 -request_cursor_y -1
	}
}

proc delete_select {im x y} {
	global sel_start sel_end child_sel

	set sel_start -1
	set sel_end -1
	$im configure -cursor -1 -subsegments "" -request_cursor_x -1 -request_cursor_y -1
	$child_sel configure -text none
}


proc show_child {geom l s i} {
	global fnt
	global attributes bitmaps image_arrays popups images nimages childimage curchild
	global view_start view_end hierarchies
	global jumpsizes jump_size manipulator_rc
	global child_del child_sel sel_start sel_end editing_parent
	global manipulator_geometry

	set curlevel $l

	if { ! $editing_parent } {
		set curchild $i
		set data "[get_child_signal $l $s $i]"
		set bounds "[get_child_bounds $l $s $i]"
		set view_start [lindex $bounds 0]
		set view_end [lindex $bounds 1]
		set label "[lrange $bounds 2 end]"
		set sel_start 0
		set sel_end [expr [llength $data]/3 - 1]

	} else {
		set level [lindex $hierarchies [expr $l + 1]]
		set view_start $sel_start
		set lindex $sel_end
		set data [lindex $i 0]
		set label [lindex $i 1]
		set delineation [lindex $i 2]
	}

	set im $image_arrays($l.image)
	set dw [expr [get_image_resources $im dwidth]+2]
	inkTcl_setImageAttributes $im -subsegments "$view_start $view_end (<($label)>) selected_color $dw"

	if {[winfo exists .child_container]} {
		set dw [expr [get_image_resources $childimage dwidth]+2]
		inkTcl_setImageAttributes $childimage -data "$data" -label "$label"\
			-subsegments "$sel_start $sel_end (<($label)>) selected_color $dw"
		return
	}

	set popup [create_popup .child_container "UNIPEN manipulator"]
	frame $popup.i
	frame $popup.j
	pack $popup.i -side top -expand yes -fill both
	pack $popup.j -side top

	set im [eval create_image_window ManipulatorRc $popup.i 1 $manipulator_rc -data \{$data\}\
		-label {$label} -subsegments \{$sel_start $sel_end (<(...)>) selected_color $dw\}]
	set childimage $im
	set images "$images {$im}"
	bind $popup.i.f1.c.f.l <Control-1> "may_set_select_start $im %x %y"
	bind $popup.i.f1.c.f.l <Control-2> "delete_select $im %x %y"
	bind $popup.i.f1.c.f.l <Control-3> "may_set_select_end $im %x %y"
	bind $popup.i.f1.c.f.l <Left>          "may_decrease_cursor_left"
	bind $popup.i.f1.c.f.l <Right>         "may_increase_cursor_left"
	bind $popup.i.f1.c.f.l <Control-Left>  "may_decrease_cursor_right"
	bind $popup.i.f1.c.f.l <Control-Right> "may_increase_cursor_right"

	set image_arrays(label.$childimage) ""
	set image_arrays(nsamples.$childimage) ""
	set image_arrays(cursor.$childimage) ""

	if { ! $editing_parent } {
		set entry [get_entry $l $s $i]
		set level [lindex $entry 1]
		set delineation [lindex $entry 5]
		set quality [lindex $entry 3]
		set label [lindex $entry 4]
	}

#	set b $popup.d
#	button $b.prev    -command "busy next_child -1"  -bitmap @$bitmaps/bwdstep.xbm
#	button $b.next    -command "busy next_child +1"  -bitmap @$bitmaps/fwdstep.xbm
#	button $b.dismiss -text Dismiss -command "destroy .child_container" -font $fnt(Buttons)
#	pack $b.prev $b.dismiss $b.next -side left -expand n

	set f $popup.j
	frame $f.entries
	frame $f.rest
	pack $f.entries $f.rest -side left
	frame $f.rest.up
	frame $f.rest.middle
	frame $f.rest.down
	pack $f.rest.up $f.rest.middle $f.rest.down -side top

	set b $f.rest.up
	label $b.lev -text "$level" -font $fnt(Labels) 
	entry $b.lab -font $fnt(Entries) 
	pack $b.lev $b.lab -side left
	$b.lab insert 0 $label
	bind $b.lab <Return> do_save_selection

	set f $popup.j.rest.middle
	frame $f.jumpleft
	frame $f.left
	frame $f.middle
	frame $f.right
	frame $f.jumpright
	pack $f.jumpleft $f.left $f.middle $f.right $f.jumpright -side left

	set b $f.left
	frame $b.1
	frame $b.2
	frame $b.3
	pack $b.1 $b.2 $b.3 -side top
	label $b.1.l1 -text more -font $fnt(Labels) 
	label $b.1.l2 -text less -font $fnt(Labels) 
	pack $b.1.l1 $b.1.l2 -side left
	button $b.2.ll -command "get_c_bounds 0 $im" -bitmap @$bitmaps/bwd.xbm\
		-width 25 -height 25
	button $b.2.ml -command "get_c_bounds 1 $im" -bitmap @$bitmaps/fwd.xbm\
		-width 25 -height 25
	pack $b.2.ll $b.2.ml -side left
	button $b.3.ll -command "get_c_bounds 2 $im" -bitmap @$bitmaps/fastbwd.xbm\
		-width 25 -height 25
	button $b.3.ml -command "get_c_bounds 3 $im" -bitmap @$bitmaps/fastfwd.xbm\
		-width 25 -height 25
	pack $b.3.ll $b.3.ml -side left

	set b $f.right
	frame $b.1
	frame $b.2
	frame $b.3
	pack $b.1 $b.2 $b.3 -side top
	label $b.1.l1 -text less -font $fnt(Labels) 
	label $b.1.l2 -text more -font $fnt(Labels) 
	pack $b.1.l1 $b.1.l2 -side left
	button $b.2.ll -command "get_c_bounds 4 $im" -bitmap @$bitmaps/bwd.xbm\
		-width 25 -height 25
	button $b.2.ml -command "get_c_bounds 5 $im" -bitmap @$bitmaps/fwd.xbm\
		-width 25 -height 25
	pack $b.2.ll $b.2.ml -side left
	button $b.3.ll -command "get_c_bounds 6 $im" -bitmap @$bitmaps/fastbwd.xbm\
		-width 25 -height 25
	button $b.3.ml -command "get_c_bounds 7 $im" -bitmap @$bitmaps/fastfwd.xbm\
		-width 25 -height 25
	pack $b.3.ll $b.3.ml -side left

	set b $popup.j.rest.down
	button $b.hlp -text Help! -command "help_labeling" -font $fnt(Buttons) 
	button $b.ins -text "insert before" -command "add_new_child null -1" -font $fnt(Buttons) 
	button $b.app -text "insert after" -command "add_new_child null +1" -font $fnt(Buttons) 
	button $b.del -text delete -command "do_del_child" -font $fnt(Buttons) -width 16 
	button $b.dis -text Dismiss -command "destroy .child_container" -font $fnt(Buttons) -width 16 
	pack $b.hlp $b.ins $b.app $b.del $b.dis -side left

	set b $f.middle
	frame $b.view
	frame $b.selection
	pack $b.view $b.selection -side top
	button $b.view.sav -text "select current view"\
		-command "select_current_view" -font $fnt(Buttons) -width 16 
	label $b.view.del -text $delineation -font $fnt(Labels) -width 12 -relief sunken 
	pack $b.view.sav $b.view.del -side left
	button $b.selection.savsel -text "save selection"\
		-command "do_save_selection" -font $fnt(Buttons) -width 16 
	label $b.selection.del -text $delineation -font $fnt(Labels) -width 12 -relief sunken 
	pack $b.selection.savsel $b.selection.del -side left
	set child_del $b.view.del
	set child_sel $b.selection.del
	update_entries 1

	set buttons ".jumpleft.jump1 .jumpleft.jump2 .jumpright.jump1 .jumpright.jump2"
	set_jump_sizes $l
	set_jump_sizes all
	set k 0
	foreach j $buttons {
		set b $f$j
		menubutton $b -text $jump_size($k) -menu $b.m1 -font $fnt(Buttons) -indicatoron 1\
			-borderwidth 2 -width 10 -relief sunken
		menu $b.m1
		set i 0
		foreach s $jumpsizes {
			$b.m1 add command -label "$s" -command "set_jumpsize $b $k \"$jumpsizes\" $i" -font $fnt(Buttons)
			incr i
		}
		pack $b
		incr k
	}
#	place_popup $popup $geom
	my_popup_ink_atts ManipulatorRc $popup.i.f1 $popup.i.f2 [list $im]
	draw_pane_curtain $popup.i $popup.i.f1 $popup.i.f2
	eval wm geometry $popup $manipulator_geometry
	update
	bind $popup <Configure> "remember_geometry $popup manipulator_geometry"
}

proc help_labeling {} {
	DisplayMessage "\n
This explains the (initially complex) process of labeling UNIPEN segments.\n
Depicted are:\n
	the main window,        in which the current focus of attention\n
                           is marked light blue\n
	the UNIPEN viewer,      which depicts the current UNIPEN segment\n
                           from which you are labeling sub-segments\n
   the UNIPEN manipulator, which depicts the current sub-segment you are labeling\n
\n
Currently, I am working on help facilities (Loe)\n
use:\n
  - the left-most and right-most to select the 'jumpsize'\n
  - the video buttons to add or delete samples according to the jumpsize\n
  - <Control-1>    to set the start of a selection\n
  - <Control-3>    to set the end of a selection\n
  - <Left>         to extend the start left-wards (1 sample)\n
  - <Right>        to move the start right-wards (1 sample)\n
  - <Control-Left> to move the end left-wards (1 sample)\n
  - <Control-Right> to move the end right-wards (1 sample)\n
"
}

proc set_jumpsize {b k js i} {
	global jumpsizes jump_size editing_parent curlevel

	set jumpsize [lindex $jumpsizes $i]
	set jump_size($k) $jumpsize
	$b configure -text $jumpsize
}

proc add_new_child {geom dir} {
	global image_arrays hierarchies curchild curlevel nchildren
	global editing_parent unipen_saved

	set unipen_saved 0
	if { $editing_parent } {
		add_new_segment $curlevel $dir
		return
	}
	set l $curlevel
	set s $image_arrays($l.segnr)
	set c $curchild
	set hname [lindex $hierarchies [expr $l+1]]
	if { $dir == -1 } {
		label_child $geom $l $s $c 1
	} else {
		label_child $geom $l $s $c 0
	}
	may_update_entry
	update_entries 1
}

set cursegno "a"
set curlevno "a"
set curchdno "a"
set curnchd  "a"
proc update_entries {make_new} {
	global fnt
	global image_arrays curchild curlevel nchildren
	global cursegno curlevno global curchdno curnchd hierarchies
	global editing_parent

	if { ! [winfo exists .child_container] } {
		return
	}
	set w .child_container.j.entries.flist

	set l $curlevel
	set s $image_arrays($l.segnr)
	set c $curchild
	set parent_entry [get_segment_entry $l $s]
	set del [lindex $parent_entry 1]
	set parent_entry [lindex $parent_entry 0]
	set lab "[lindex $parent_entry 4]"
	set lev [lindex $hierarchies [expr $curlevel + 1]]

	if { $make_new || $s!=$cursegno || $l!=$curlevno || $curnchd!= $nchildren } {
		set cursegno $s
		set curlevno $l
		set curnchd $nchildren
		catch {$w.list delete 0 end}
		if { ! [winfo exists $w] } {
			button .child_container.j.entries.lab -command "do_edit_segment $l $s null"\
				-text "$lev $del" -font $fnt(Buttons) 
			pack .child_container.j.entries.lab -side top
			frame $w
			pack $w -side top -expand yes -fill y
			listbox $w.list -xscroll "$w.scrollx set" -yscroll "$w.scrolly set"\
				-setgrid 1 -width 30 -height 7 -font $fnt(Entries) -exportselection 0
			scrollbar $w.scrollx -command "$w.list xview" -orient horizontal
			scrollbar $w.scrolly -command "$w.list yview" -orient vertical
			pack $w.scrollx -side bottom -fill x
			pack $w.scrolly -side left -fill y
			pack $w.list -side left -expand 1 -fill both
		}

		set child_entries [get_children_entries $l $s]
		set nentries      [lindex $child_entries 0]
		set entry_level   [lindex $child_entries 1]
		set entries       [lindex $child_entries 2]
		for {set k [expr $nentries-1]} {$k>=0} {set k [expr $k-1]} {
			set e "[lindex $entries $k]"
			$w.list insert 0 $e
		}
		bind $w.list <ButtonPress-1> "goto_child $w %W %y"
	}
	catch {$w.list selection clear $curchdno}

	if { ! $editing_parent } {
		$w.list see $c
		$w.list selection set $c
		set curchdno $c
	} else {
		.child_container.j.entries.lab configure -text "$lev $del"
		do_change_entrybox .child_container.j.rest.up.lab "$lab"
		.child_container.j.rest.up.lev configure -text $lev
	}
}

proc goto_child {w W y} {
	global curchild editing_parent

	set editing_parent 0
	set c [$W nearest $y]
	set curchild $c
	busy next_child 0
}

proc may_update_entry {} {
	global image_arrays curchild curlevel
	global child_del child_sel

	if {![winfo exists .child_container.j.rest.up.lev]} {
		return
	}

	set l $curlevel
	set s $image_arrays($l.segnr)
	set c $curchild
	set entry [get_entry $l $s $c]
	set level [lindex $entry 1]
	set delineation [lindex $entry 5]
	set quality [lindex $entry 3]
	set label [lindex $entry 4]

	.child_container.j.rest.up.lev configure -text $level
	$child_del configure -text $delineation
	$child_sel configure -text $delineation
	do_change_entrybox .child_container.j.rest.up.lab $label
}

proc do_del_child {} {
	global curchild curlevel nchildren image_arrays
	global editing_parent hierarchies segment_names
	global unipen_saved child_sel

	if { $nchildren == 0 } {
		do_loe_beep
		return
	}
	set l $curlevel
	set s $image_arrays($l.segnr)
	set pname [lindex [lindex $segment_names($l) [expr $s+1]] 0]
	set del [lindex [$child_sel configure -text] 4]
	if { $editing_parent } {
		set lev [lindex $hierarchies [expr $l + 1]]
		set answer [DialogWait "\
			Deleting a $lev segment will involve re-viewing the hierarchy!"]
		if { $answer } {
			del_segment $l $s
			log_upworks "deleted $lev\[$s\] $del \"$pname\""
			set n [lindex $segment_names($l) 0]
			set head_names [expr $n - 1]
			set tail_names ""
			if { $s > 1 } {
				set head_names "$head_names [lrange $segment_names($l) 1 $s]"
			}
			set s [expr $s + 2]
			if { $s < $n } {
				set tail_names [lrange $segment_names($l) $s end]
			}
			set segment_names($l) "$head_names $tail_names"
			view_hierarchy $l 1 -1
			set unipen_saved 0
		}
		return
	}
	del_child $l $s $curchild
	set plev [lindex $hierarchies [expr $l + 1]]
	set clev [lindex $hierarchies [expr $l + 2]]
	log_upworks "deleted $clev\[$curchild\] $del in $plev\[$s\] \"$pname\""
	set nchildren [expr $nchildren - 1]
	set unipen_saved 0
	if { $nchildren == 0 } {
#		inkTcl_setImageAttributes $image_arrays($l.image) -subsegments ""
#		destroy .child_container
		do_edit_segment $l $s null
		set curchild 0
		return
	}
	busy next_child 0
}

proc do_save_child {} {
	global curchild image_arrays curlevel
	global child_del child_sel unipen_saved

	set del [lindex [$child_sel configure -text] 4]
	set l $curlevel
	set s $image_arrays($l.segnr)
	set label [do_get_entrybox .child_container.j.rest.up.lab]
	set result [busy save_child $l $s $curchild $del "$label" 0]
	if {"$result"!="1"} {
		set answer [DialogWait "\
			Child delineation $del exceeds parent!\
			You may solve this by editing and enlarging the parent first......\
			Or click <Ok> to overrule this, which involves that this segment will no longer be
			a sub-segment."]
		if { ! $answer } {
			return 0
		} else {
			set result [busy save_child $l $s $curchild $del "$label" 0]
		}
	}
	set unipen_saved 0
	update_entries 1
	$child_del configure -text $del
	if {"$result"=="1"} {
		set subs [get_bounds $l $s]
		if {"$subs"!=""} {
			"$l $s" configure -subsegments "$subs"
		}
	}
	return 1
}

proc do_edit_segment { l s geom } {
	global editing_parent

	set editing_parent 1
	busy do_next_parent $geom
}

proc do_edit_child { geom l s i } {
	global editing_parent

	set editing_parent 0
	show_child $geom $l $s $i
	update_entries 1
	may_update_entry
}


proc do_next_parent {geom} {
	global curlevel image_arrays childimage nchildren curchild
	global child_del child_sel sel_end sel_start

	set l $curlevel
	set s $image_arrays($l.segnr)
	set data [get_full_segment $l $s]
	set nchildren [get_nchildren $l $s]
	set curchild 0
	set sel_start 0
	set sel_end [expr [llength $data]/3 - 1]

	set entry [get_segment_entry $l $s]
	set del [lindex $entry 1]
	set entry [lindex $entry 0]
	if { ! [winfo exists .child_container] } {
		show_child $geom $l $s "[list $data] [list [lindex $entry 4]] $del"
	} else {
		set dw [expr [get_image_resources $childimage dwidth]+2]
		$childimage configure -data "$data" -subsegments "$sel_start $sel_end (<(...)>) selected_color $dw"
		$child_del configure -text $del
		$child_sel configure -text $del
		update_entries 0
	}

	initiate_segment_walk $l $s
}

proc next_child {dir} {
	global editing_parent segment_names curlevel image_arrays

	if { $editing_parent } {
		set l $curlevel
		busy next_image $l $dir
	} else {
		busy do_next_child $dir
	}
}

proc do_next_child {dir} {
	global nchildren childimage curchild curlevel image_arrays
	global sel_start sel_end view_start view_end

	set curchild [expr ($curchild+$dir+$nchildren)%$nchildren]
	set l $curlevel
	set s $image_arrays($l.segnr)
	set data "[get_child_signal $l $s $curchild]"
	set bounds "[get_child_bounds $l $s $curchild]"
	set view_start [lindex $bounds 0]
	set view_end [lindex $bounds 1]
	set label [lindex $bounds 2]
	set im $image_arrays($l.image)
	set dw [expr [get_image_resources $im dwidth]+2]
	inkTcl_setImageAttributes $im -subsegments "$view_start $view_end (<(...)>) selected_color $dw"
	set sel_start 0
	set sel_end [expr [llength $data]/3 - 1]
	set dw [expr [get_image_resources $childimage dwidth]+2]
	inkTcl_setImageAttributes $childimage -data "$data" -label "$label"\
		-cursor -1 -subsegments "$sel_start $sel_end (<(...)>) selected_color $dw"
	may_update_entry
	update_entries 0
}
