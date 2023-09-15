set alphabet \
"abcdefghijklmnopqrstuvwxyz\+\-\_\!\@\#\$\%\^\&\*\(\)\=\|\}\{\[\]\."
set number "1234567890"

proc scroll_update {w args} {
	eval $w.list $args
	eval $w.list2 $args
}

proc scroll_segments {wname title items curitem ret_callback} {
	global fnt
	global segments_are_sorted segments scroll_popup

	catch {destroy $scroll_popup}
	set w .$wname
	set scroll_popup $w
	toplevel $w
	wm title $w "$title"
	wm iconname $w [lindex $title 0]

	set segments_are_sorted 0

	bind $w <Return> "$ret_callback $w"
	frame $w.buttons
	pack  $w.buttons -side bottom -expand y -fill x -pady 2m
	button $w.buttons.sort -text Sort -command "sort_segments $w \{$items\}" -font $fnt(Buttons) 
	button $w.buttons.dismiss -text Dismiss -command "destroy $w" -font $fnt(Buttons) 
	pack $w.buttons.sort $w.buttons.dismiss -side left -expand 1

	set s $w.s
	frame $s
	pack $s -side top -expand y -fill x -pady 2m
	entry $s.itemnr -relief sunken -font $fnt(Entries) 
	$s.itemnr insert 0 $curitem
	entry $s.itemname -relief sunken -font $fnt(Entries) 
	$s.itemname insert 0 [lindex [lindex $items [expr $curitem+1]] 0]
	pack $s.itemnr $s.itemname -side top
	bind Try2 <Key> "search_item $w %K %W"
	bindtags $s.itemnr "[bindtags $s.itemnr] Try2"
	bindtags $s.itemname "[bindtags $s.itemname] Try2"

	frame $w.flist -borderwidth .5c
	pack $w.flist -side top -expand yes -fill y
	scrollbar $w.flist.xscroll -command "$w.flist.list xview" -orient horizontal
	scrollbar $w.flist.yscroll -command "scroll_update $w.flist yview"
	listbox $w.flist.list -yscroll "$w.flist.yscroll set" -xscroll "$w.flist.xscroll set"\
		-setgrid 1 -height 15 -width 25 -font $fnt(Entries)
	listbox $w.flist.list2 -yscroll "$w.flist.yscroll set" -xscroll "$w.flist.xscroll set"\
		-setgrid 1 -height 15 -width 20 -font $fnt(Entries)
	pack $w.flist.yscroll -side right -fill y
	pack $w.flist.xscroll -side bottom -fill x
	pack $w.flist.list $w.flist.list2 -side left -expand 1 -fill both

	set nitems [llength $items]
	set segments [lrange $items 1 end]
	for {set i [expr $nitems]} {$i > 0} {set i [expr $i-1]} {
		set item [lindex $items $i]
		$w.flist.list insert 0 [lindex $item 0]
		$w.flist.list2 insert 0 [lindex $item 1]
	}
	$w.flist.list selection set $curitem
	$w.flist.list see $curitem

	bind Try <1> "change_item $w %W %y; $ret_callback $w"
	bind Try <Up> "check_item $w 0; $ret_callback $w"
	bind Try <Down> "check_item $w 1; $ret_callback $w"
	bindtags $w.flist.list "Try"

	focus $w.flist.list
	
	return $w
}

proc sort_segments {w items} {
	global sorted_segments segments_are_sorted

	set n [lindex $items 0]
	if { ! $segments_are_sorted } {
		set sorted ""
		set j -1
		for {set i 1} {$i<=$n} {incr i} {
			set name "[lindex $items $i]"
			lappend sorted "$name [incr j]"
		}
		set sorted_segments [lsort $sorted]
		set items $sorted_segments
		set segments_are_sorted 1
		$w.buttons.sort configure -text Unsort
	} else {
		set segments_are_sorted 0
		$w.buttons.sort configure -text Sort
		set items [lrange $items 1 end]
	}
	$w.flist.list delete 0 $n
	$w.flist.list2 delete 0 $n
	for {set i [expr $n];set j 0} {$i >= 0} {set i [expr $i-1];incr j} {
		set item [lindex $items $i]
		if { $segments_are_sorted } {
			set [lindex [lindex $sorted_segments $i] 1] $j
		}
		set name [lindex $item 0]
		set del [lindex $item 1]
		$w.flist.list insert 0 "$name"
		$w.flist.list2 insert 0 "$del"
	}
}

proc check_item {w s} {
	global segments

	set segnr 0
	set segnr     [$w.flist.list curselection]
	set n [llength $segments]
	
	if {$s == 1} {
		incr segnr
		if {$segnr > [expr $n - 1]} {
			set segnr 0
		}
	}
	if {$s == 0} {
		set segnr [expr $segnr - 1]
		if {$segnr < 0} {
			set segnr [expr $n - 1]
		}
	}
	show_select $w $segnr 1
}

proc search_item {w k p} {
	global segments_are_sorted hierarchy

	if {$p == "$w.s.itemname"} {
		set str [$w.s.itemname get]
		set i [search_segment $hierarchy $str $segments_are_sorted]
		if {$i != -1} {
			show_select $w $i 0
		}
	}
	if {$p == "$w.s.itemnr"} {
		if {[regexp $k $number]} {
			set num [$w.s.itemnr get]
			show_select $w $num 1
		}
	}
}

proc show_select {w s ok} {
	global segments segments_are_sorted sorted_segments

	set n [llength $segments]
	set str [$w.s.itemname get]
	$w.s.itemnr delete 0 10
	if {$ok} {
		$w.s.itemname delete 0 [string length $str]
	}
	$w.flist.list selection clear 0 $n

	if {$ok} {
		if {$segments_are_sorted} {
			$w.s.itemname insert 0 [lindex [lindex $sorted_segments $s] 0]
		} else {
			$w.s.itemname insert 0 [lindex [lindex $segments $s] 0]
		}
	}
	$w.s.itemnr insert 0 $s
	$w.flist.list selection set $s
	$w.flist.list activate $s
	$w.flist.list see $s
}

proc JANEK {w} {
	global segments segments_are_sorted sorted_segments

	set segnr     [eval $w.s.itemnr get]
	set segnr     [lindex $segnr 0]

	$w.flist.list selection clear 0 [llength $segments]
	$w.flist.list selection set $segnr

	if {$segments_are_sorted} {
		set item [lindex $sorted_segments $segnr]
		set segname [lindex $item 0]
		set segnr [lindex $item 1]
	} else {
		set segname   [lindex $segments $segnr]
	}
}

global allowed
set allowed(plus)			 	 "\+"
set allowed(equal) 			 "\="
set allowed(minus)			 	 "\-"
set allowed(underscore) 		 "\_"
set allowed(bracketright) 	 "\]"
set allowed(bracketleft) 	 "\["
set allowed(braceleft) 		 "\{"
set allowed(braceright) 		 "\}"
set allowed(backslash) 		 "\\"
set allowed(bar) 				 "\|"
set allowed(quotedbl) 		 "\""
set allowed(colon) 			 "\:"
set allowed(apostrophe) 		 "\'"
set allowed(semicolon) 		 "\;"
set allowed(question) 		 "\?"
set allowed(greater) 			 "\>"
set allowed(less) 				 "\<"
set allowed(asciitilde) 		 "\~"
set allowed(grave) 			 "\`"
set allowed(exclam) 			 "\!"
set allowed(at) 				 "\@"
set allowed(numbersign) 		 "\#"
set allowed(dollar) 			 "\$"
set allowed(percent) 			 "\%"
set allowed(asciicircum) 	 "\^"
set allowed(ampersand) 		 "\&"
set allowed(asterisk) 		 "\*"
set allowed(parenleft) 		 "\("
set allowed(parenright) 		 "\)"
set allowed(period)			 "\."
set allowed(comma)				 "\,"
set allowed(slash)				 "\/"
set allowed(BackSpace)			"bs"
set allowed(Delete)			"bs"
