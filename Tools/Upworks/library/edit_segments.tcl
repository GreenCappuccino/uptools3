proc get_s_bounds {m s_im} {
	global image_arrays view_start view_end curlevel
	global jump_size jumpsizes
	global child_del

	eval set n_before [get_image_resources $s_im nsamples]
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
	set result [get_segm_markers $l $s $m $jumpsize $view_start $view_end]
	if { "$result" == "0" } {
		do_loe_beep
	} else {
		set result [get_edited_segment]
		set data [lindex $result 1]
		set del  [lindex $result 0]
		$child_del configure -text $del
		adjust_and_set_markers $m $n_before $s_im $data
	}
}

proc do_save_segment {l} {
	global view_start view_end image_arrays segment_names
	global child_sel unipen_saved

	set label [do_get_entrybox .child_container.j.rest.up.lab]
	set s $image_arrays($l.segnr)
	set del [lindex [$child_sel configure -text] 4]
	if { "$del" == "none" } {
		do_loe_beep
		DisplayMessage "Nothing selected to be saved!"
		return
	}
	save_segment $l $s $del "$label"
	set unipen_saved 0
	incr s
	set segment_names($l) [lreplace $segment_names($l) $s $s "$label"]
	next_image $l 0
	update_entries 1
	set result [get_edited_segment]
	set data [lindex $result 1]
	set del  [lindex $result 0]
	set s [expr $s - 1]
	set im "$l $s"
	$im configure -data $data
}

proc add_new_segment {l dir} {
	global image_arrays segment_names hierarchies unipen_saved

	set lev [lindex $hierarchies [expr $l + 1]]
	if { ! [DialogWait "Adding a new $lev segment involves copying the current one, and\
	subsequently re-viewing the hierarchy!"] } {
		return
	}
	add_segment $l $image_arrays($l.segnr) $dir
	catch {delete $segment_names($l)}
	set segment_names($l) [get_segment_names $l]
	view_hierarchy $l 1 -1
	set unipen_saved 0
}
