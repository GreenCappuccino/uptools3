#
# from Jeffrey Hobbs (jhobbs@cs.uoregon.edu, code got from
# http://www.cs.uoregon.edu/research/tcl/faqs/tk/#cursor
#
# slightly changed by Louis, Februari 1997

proc busy {args} {
	 global errorInfo

	 set list "."
	 while {$list != ""} {
		 set next {}
		 foreach w $list {
			 set class [winfo class $w]
			 set cursor [lindex [$w config -cursor] 4]
			 if {[winfo toplevel $w] == $w || $cursor != ""} {
				 lappend busy [list $w $cursor]
			 }
			 set next [concat $next [winfo children $w]]
		 }
		 set list $next
	 }

	 foreach w $busy {
		 catch {[lindex $w 0] config -cursor watch}
	 }

	 update idletasks

	 set error [catch {uplevel eval [list $args]} result]
	 set ei $errorInfo

	 foreach w $busy {
		 catch {[lindex $w 0] config -cursor [lindex $w 1]}
	 }

	 if $error {
		 error $result $ei
	 } else {
		 return $result
	 }
}
