#set fnt(Menus)   fixed
#set fnt(Dialogs) fixed
#set fnt(Buttons) fixed
#set fnt(Labels)  fixed
#set fnt(Entries) fixed

set fnt(Menus)   -adobe-helvetica-bold-r-normal--18-180-75-75-p-103-iso8859-1
set fnt(Dialogs) -adobe-courier-medium-r-normal--12-120-75-75-m-70-iso8859-1
set fnt(Buttons) -adobe-courier-bold-r-normal--12-120-75-75-m-70-iso8859-1
set fnt(Labels)  -adobe-courier-bold-r-normal--12-120-75-75-m-70-iso8859-1
set fnt(Entries) -adobe-helvetica-medium-r-normal--11-80-100-100-p-56-iso8859-1


proc widget2name {w prefix} {
	set name [exec echo $w | tr -d '.()']
	return $prefix$name
}

proc create_menu_balk {} {
	frame .menu -relief raised  -borderwidth 1
	pack .menu -side top  -fill x
}

proc create_new_menu_balk {w} {
	frame $w.menu -relief raised  -borderwidth 1
	pack $w.menu -side top  -fill x
}

proc create_pulldown_menu { name labelcmds } {
	global fnt
	
	set w ".menu.m$name"

	menubutton $w -text "$name" -menu $w.m -font $fnt(Menus)
	menu $w.m -font $fnt(Menus)
	foreach lc $labelcmds {
		set label [lindex $lc 0]
		set cmd   [lindex $lc 1]
		$w.m add command -label "$label" -command $cmd -font $fnt(Menus)
	}
	pack $w -side left
	return $w
}

proc create_pulldown_menubutton { f name labelcmds } {
	global fnt
	
	set w "$f.m$name"

	menubutton $w -text "$name" -menu $w.m -font $fnt(Menus)
	menu $w.m -font $fnt(Menus)
	foreach lc $labelcmds {
		set label [lindex $lc 0]
		set cmd   [lindex $lc 1]
		$w.m add command -label "$label" -command "$cmd $w" -font $fnt(Menus)
	}
	return $w
}

proc create_popup { wname title } {
	set w $wname
	catch {destroy $w}
	toplevel $w
	wm title $w "$title"
	wm iconname $w [lindex $title 0]
	return $w
}

proc popup_scrollable_listbox {wname title items curitem ok_text ret_callback use_ret_for_click} {
	global fnt
	set w .$wname
	catch {destroy $w}
	toplevel $w
	wm title $w "$title"
	wm iconname $w [lindex $title 0]

	bind $w <Return> "$ret_callback $w 1"

	frame $w.buttons
	pack  $w.buttons -side bottom -expand y -fill x -pady 2m
	button $w.buttons.dismiss -text Dismiss -command "destroy $w" -font $fnt(Buttons) 
	if {$ok_text != ""} {
		button $w.buttons.show -text $ok_text -command "$ret_callback $w 1" -font $fnt(Buttons) 
		pack $w.buttons.show $w.buttons.dismiss -side left -expand 1
	} else {
		pack $w.buttons.dismiss -side left -expand 1
	}
	set s $w.s
	frame $s
	pack $s -side top -expand y -fill x -pady 2m
	entry $s.itemnr -relief sunken 
	$s.itemnr insert 0 $curitem
	entry $s.itemname -relief sunken 
	$s.itemname insert 0 [lindex $items [expr $curitem+1]]
	pack $s.itemnr $s.itemname -side top

	frame $w.flist -borderwidth .5c
	pack $w.flist -side top -expand yes -fill y
	scrollbar $w.flist.scroll -command "$w.flist.list yview"
	listbox $w.flist.list -yscroll "$w.flist.scroll set" -setgrid 1 -height 12
	pack $w.flist.scroll -side right -fill y
	pack $w.flist.list -side left -expand 1 -fill both

	set nitems [lindex $items 0]
	for {set i $nitems} {$i > 0} {set i [expr $i-1]} {
		set item [lindex $items $i]
		$w.flist.list insert 0 $item
	}
	$w.flist.list selection set $curitem
	$w.flist.list see $curitem
	bind $w.flist.list <Double-1> {change_item $w %W %y; $ret_callback $w 1}
	if {$use_ret_for_click} {
		bind $w.flist.list <ButtonPress-1> "change_item $w %W %y; $ret_callback $w 0"
	} else {
		bind $w.flist.list <ButtonPress-1> "change_item $w %W %y"
	}

	return $w
}

proc change_item {w W y} {
	set itemnr [$W nearest $y]
	set itemname [$W get $itemnr]
	set curnr [eval $w.s.itemnr get]
	set curname [eval $w.s.itemname get]
	$w.s.itemnr delete 0 [string length $curnr]
	$w.s.itemname delete 0 [string length $curname]
	$w.s.itemnr insert 0 $itemnr
	$w.s.itemname insert 0 $itemname
}


#
# TABLE  TABLE  TABLE  TABLE  TABLE  TABLE  TABLE  TABLE  TABLE  TABLE  TABLE  TABLE  TABLE  TABLE 
#


proc create_table { w nrows ncols font colwidth header rows } {
	frame $w.table
	pack $w.table -side top -expand y -fill x
	set m $w.table
	for {set c 0} {$c<$ncols} {incr c} {
		frame $m.col($c)
		pack $m.col($c) -side left
		label $m.col($c).header -text [lindex $header $c] -font $font -justify center -relief sunken -width $colwidth 
		pack $m.col($c).header -side top
		for {set r 0 } {$r<$nrows} {incr r} {
			set row [lindex $rows $r]
			set item [lindex $row $c]
			label $m.col($c).row($r) -text $item -font $font -justify left -width $colwidth -relief sunken 
			pack $m.col($c).row($r) -side top
		}
	}
}

proc update_table { w nrows ncols header rows } {
	set m $w.table
	for {set c 0} {$c<$ncols} {incr c} {
		$m.col($c).header configure -text [lindex $header $c]
		for {set r 0 } {$r<$nrows} {incr r} {
			set row [lindex $rows $r]
			set item [lindex $row $c]
			$m.col($c).row($r) configure -text $item
		}
	}
}

#
# TEXT  TEXT  TEXT  TEXT  TEXT  TEXT  TEXT  TEXT  TEXT  TEXT  TEXT  TEXT  TEXT  TEXT  TEXT  TEXT 
#

set text_window_created 0
proc create_a_text_window {title font txt} {
	global fnt
	global text_window_created text_window_size

	if {$text_window_created} {
		set w .atext.text
		$w delete 0.0 $text_window_size.0
		$w insert 0.0 "$txt"
		set text_window_size [expr [llength $txt] + 1]
	} else {
		set w [create_popup .atext $title]
		scrollbar $w.scroll -command "$w.text yview"
		pack $w.scroll -side right -fill y
		text $w.text -font $font -width 90 -height 60 -yscroll "$w.scroll set" -setgrid 1 -height 12
		$w.text insert 0.0 "$txt"
		set text_window_size [expr [llength $txt] + 1]
		pack $w.text
		button $w.dismiss -text Dismiss -command "delete_text_window $w" -font $fnt(Buttons) 
		pack $w.dismiss
		set text_window_created 1
	}
}

proc delete_text_window {w} {
	global text_window_created

	set text_window_created 0
	destroy $w
}
