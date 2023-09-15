proc change_color {c i w} {
	global C

	$C($w) itemconfigure $C($w.l) -fill $c
	$w configure -text $c
}

proc increase_width {f w i} {
	set c $f.f$i.c
	set l l$i
	set lab $f.f$i.l
	set width [$lab cget -text]
	if {$w==-1 && $width==0} {
		return
	}
	set width [expr $w+$width]
	$c itemconfigure $l -width $width
	$lab configure -text $width
}

proc setloe {f} {
	global COLORS WIDTHS

	set i 0
	set result_colors ""
	set result_widths ""
	foreach c $COLORS {
		set col [$f.f$i.c itemcget l$i -fill]
		set width [$f.f$i.l cget -text]
		set result_colors "$result_colors $col"
		set result_widths "$result_widths $width"
		incr i
	}
	set COLORS $result_colors
	set WIDTHS $result_widths
	write_default_resources
}

proc okloe {p f} {
	setloe $f
	catch {destroy $p}
	write_default_resources
}

proc edit_lines {colors widths} {
	global fnt
	global C ORIGINAL_COLORS
	
	set popup [create_popup .linewidths "edit lines"]
	set f $popup.f
	frame $f
	set i 0
	set col_menus ""
	foreach c $ORIGINAL_COLORS {
		set MYWIDTHS($i) [lindex $widths $i]
		set col_menus "$col_menus \"$c \{change_color $c $i\}\""
		incr i
	}
	set i 0
	foreach c $colors {
		set width $MYWIDTHS($i)
		frame $f.f$i
		set m [create_pulldown_menubutton $f.f$i $c $col_menus]
		$m configure -width 10 -text $c -font fixed
		canvas $f.f$i.c -width 60 -height 10
		$f.f$i.c create line 0 5 60 5 -width $width -fill $c -tags l$i
		bind $f.f$i.c <1> "increase_width $f -1 $i"
		bind $f.f$i.c <3> "increase_width $f +1 $i"
		pack $f.f$i -side top
		label $f.f$i.l -text $width -font $fnt(Labels) 
		pack $m $f.f$i.c $f.f$i.l -side left

		set C($m) $f.f$i.c
		set C($m.l) l$i

		incr i
	}

	pack $f
	button $f.d -command "destroy $popup" -text cancel -font $fnt(Buttons) 
	button $f.s -command "setloe $f" -text set -font $fnt(Buttons) 
	button $f.b -command "okloe $popup $f" -text ok -font $fnt(Buttons) 
	pack $f.d
	pack $f.s $f.b -side left
}
