proc create_myprogress {c title} {
	if { ![winfo exists .$c.c] } {
		toplevel .$c
		wm title .$c $title
		wm geometry .$c +450+450
		canvas .$c.c -width 250 -height 40
		.$c.c create rectangle 10 10 240 30 -width 2 -outline black
		pack .myprogress.c
	} else {
	}
	return .$c.c
}

proc init_progress { n title } {
	global win nprogress prev_perc

	set prev_perc -1
   set win [create_myprogress myprogress $title]
	set nprogress $n
	catch {$win delete stat txt}
}

set prev_perc -1
proc progress { i } {
	global win nprogress prev_perc

	if { ![winfo exists $win] } {
		return
	}
	set prgr [expr $i.0 * 100 / $nprogress]
	set perc [format %.0f $prgr]
	if {$perc==$prev_perc} {
		return
	}
	set newx [expr $prgr * 2.3 + 11]
	catch {$win delete stat txt}
	catch {$win create line 11 20 $newx 20 -fill SteelBlue2 -width 18 -tags stat}
	catch {$win create text 125 20 -text "$perc %" -anchor c -tags txt}
	update
	set prev_perc $perc
}

proc kill_progress {} {
	catch {destroy .myprogress}
}
