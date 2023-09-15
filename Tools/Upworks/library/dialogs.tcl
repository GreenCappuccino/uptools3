global acknowledge

set fnt(Menus)   -adobe-helvetica-bold-r-normal--18-180-75-75-p-103-iso8859-1
set fnt(Dialogs) -adobe-courier-medium-r-normal--12-120-75-75-m-70-iso8859-1
set fnt(Buttons) -adobe-courier-bold-r-normal--12-120-75-75-m-70-iso8859-1
set fnt(Labels)  -adobe-courier-bold-r-normal--12-120-75-75-m-70-iso8859-1
set fnt(Entries) -adobe-helvetica-medium-r-normal--11-80-100-100-p-56-iso8859-1


proc DialogExecute { msg entries callback nentries } {
	global fnt

	set w .entry1
	catch {destroy $w}
	toplevel $w
	wm title $w "dialog box"
	wm iconname $w "entry1"
	label $w.msg -font $fnt(Dialogs) -wraplength 5i -justify left -text $msg -font $fnt(Labels)
	frame $w.buttons
	pack  $w.buttons -side bottom -expand y -fill x -pady 2m
	button $w.buttons.dismiss -text Dismiss -command "destroy $w" -font $fnt(Buttons)
	button $w.buttons.ok -text Ok -command "$callback $w" -font $fnt(Buttons)
	bind $w <Return> "$callback $w"
	pack $w.buttons.dismiss $w.buttons.ok -side left -expand 1
	pack $w.msg -side top -pady 5 -padx 10 -fill x
	for {set i 0} {$i<$nentries} {set i [incr i]} {
		entry $w.e($i) -relief sunken -font $fnt(Dialogs)
		pack $w.e($i) -side top -pady 5 -padx 10 -fill x
		$w.e($i) insert 0 [lindex $entries $i]
	}
	return $w
}

proc DialogEntryBox { msg nentries labels entries callback} {
	global fnt

	set w .entrybox
	catch {destroy $w}
	toplevel $w
	wm title $w "dialog execute"
	wm iconname $w "[lindex $msg 0]"
	label $w.msg -font $fnt(Dialogs) -wraplength 5i -justify left -text $msg -font $fnt(Labels)
	frame $w.buttons
	pack  $w.buttons -side bottom -expand y -fill x -pady 2m
	entrybox $w $nentries $labels $entries $fnt(Entries)
	button $w.buttons.dismiss -text Dismiss -command "destroy $w" -font $fnt(Buttons)
	button $w.buttons.ok -text Ok -command "$callback $w" -font $fnt(Buttons)
	bind $w <Return> "$callback $w"
	pack $w.buttons.dismiss $w.buttons.ok -side left -expand 1
	pack $w.msg -side top -pady 5 -padx 10 -fill x
}


proc NewDialogEntryBox { p msg nentries labels entries kinds callback} {
	global UW_all

	set w [widget2name $p .entrybox]
	catch {destroy $w}
	toplevel $w
	wm title $w $msg
	wm iconname $w "[lindex $msg 0]"

	set butts $w.buttons
	frame $butts
	newentrybox $w dummy $nentries $labels $entries $kinds $callback
	button $butts.dismiss -text Dismiss -command "destroy $w" -font $fnt(Buttons) 
	pack  $butts -side bottom -expand y -fill x -pady 2m
	if {$callback!="nocallback"} {
		button $butts.ok -text Ok -command "$callback $w" -font $fnt(Buttons) 
		bind $w <Enter> "focus $w"
		bind $w <Return> "$callback $w"
		pack $butts.dismiss $butts.ok -side left -expand y
	} else {
		pack $butts.dismiss -side left -expand y
	}
	return $w
}

proc maxstring_length {labels} {
	set max 0
	foreach l $labels {
		set length [string length $l]
		if {$length>$max} {
			set max $length
		}
	}
	return $max
}

proc get_labelbox {w i} {
	return [eval $w.box.e($i) cget -text]	
}

proc change_labelbox {w i t} {
	$w.box.e($i) configure -text $t	
}

proc get_entrybox {w i} {
	return do_get_entrybox $w.box.e($i) 
}

proc do_get_entrybox {w} {
	return [eval $w get]	
}

proc change_entrybox {w i t} {
	do_change_entrybox $w.box.e($i) $t
}

proc do_change_entrybox {w t} {
	set newlength [string length $t]
	set oldlength [$w cget -width]
	$w delete 0 end
	$w insert 0 $t
	if { $oldlength<$newlength } {
		$w configure -width $newlength
	}
}

proc newentrybox { w im nentries labels entries kinds callback} {
	global image_arrays fnt

	frame $w.box
	pack $w.box -side top -expand y
	set firsteditable -1
	set m $w.box
	for {set e 0 } {$e<$nentries} {set e [incr e]} {
		set k [lindex $kinds $e]
		switch $k {
			0 {
				entry $m.e($e) -relief sunken -width 30 -font $fnt(Entries) 
				$m.e($e) insert 0 [lindex $entries $e]
				if {$firsteditable==-1} {
					set firsteditable $e
				}
				bind $m.e($e) <Return> "$callback $w"
			}
			1 {
				label $m.e($e) -text [lindex $entries $e] -relief sunken -font $fnt(Labels) 
			}
			default {
				set v [lindex $k 0]
				set c [lindex $k 1]
				global $v
				checkbutton $m.e($e) -selectcolor orange -variable $v -command "$c"
			}
		}
		label $m.l($e) -text [lindex $labels $e] -font $fnt(Labels) 
		grid $m.e($e) -column 1 -row $e -sticky w
		grid $m.l($e) -column 0 -row $e -sticky e
		if { "$im" != "dummy" } {
			switch [lindex $labels $e] {
				"label" {
					set image_arrays(label.$im) $m.e($e)
				}        
				"cursor" {
					set image_arrays(cursor.$im) $m.e($e)
				}        
				"nsamples" {
					set image_arrays(nsamples.$im) $m.e($e)
				}        
			}
		}
	}
	if {$firsteditable!=-1} {
		set value [lindex $entries $firsteditable]
		$m.e($firsteditable) selection range 0 [string length $value]
#		focus $m.e($firsteditable)
	}
}

proc change_selection { m } {
	global NENTRIES CUR_ENTRY

	set CUR_ENTRY [expr ($CUR_ENTRY + 1)%$NENTRIES]
	if { $CUR_ENTRY < [expr $NENTRIES - 2] } {
		set value [$m.e($CUR_ENTRY) get]
		$m.e($CUR_ENTRY) selection range 0 [string length $value]
	}
}

proc Loe_Dismiss { w } {
	global fsLoe

	set fsLoe(filename) ""
	destroy $w
}

proc fsLoe_Ok { w } {
	global fsLoe

	$w configure -cursor watch
	set fsLoe(filename) [eval $w.settings.entries.e(1) get]/[eval $w.settings.entries.e(0) get]
	destroy $w
}

proc DialogOk { w v } {
	global acknowledge

	set acknowledge $v
	destroy $w
}

proc DialogWait { msg } {
	global fnt
	global acknowledge

	set w .msg
	catch {destroy $w}
	toplevel $w
	wm title $w "please answer this question"
	wm iconname $w "msg"
	label $w.msg -font $fnt(Dialogs) -wraplength 5i -justify left -text $msg 
	frame $w.buttons
	pack  $w.buttons -side bottom -expand y -fill x -pady 2m
	button $w.buttons.dismiss -text Dismiss -command "DialogOk $w 0" 
	button $w.buttons.ok -text Ok -command "DialogOk $w 1" 
	pack $w.msg $w.buttons.dismiss $w.buttons.ok -side left -expand 1

	bind $w <Return> "DialogOk $w 1"
#	focus $w

	tkwait window $w
	return $acknowledge
}

proc DisplayMessage { msg } {
	global fnt

	set w .msg
	catch {destroy $w}
	toplevel $w
	wm title $w "please acknowledge this message"
	wm iconname $w "msg"
	label $w.msg -font $fnt(Dialogs) -wraplength 5i -justify left -text $msg -font $fnt(Labels) 
	frame $w.buttons
	pack  $w.buttons -side bottom -expand y -fill x -pady 2m
	button $w.buttons.dismiss -text Dismiss -command "destroy $w" -font $fnt(Buttons) 
	pack $w.msg $w.buttons.dismiss -side left -expand 1
	tkwait window $w
}

proc entrybox { w nentries labels entries font} {
	global NENTRIES CUR_ENTRY

	set NENTRIES [expr $nentries + 2]
	set CUR_ENTRY 0
	frame $w.settings
	pack $w.settings -side top -expand y -fill x
	set m $w.settings
	frame $m.labels
	frame $m.entries
	pack $m.labels -side left
	pack $m.entries -side left -expand y -fill x
	for {set e 0 } {$e<$nentries} {set e [incr e]} {
		entry $m.entries.e($e) -relief sunken 
		$m.entries.e($e) insert 0 [lindex $entries $e]
		label $m.labels.l($e) -text [lindex $labels $e] 
		pack $m.labels.l($e) -side top
		pack $m.entries.e($e) -side top -expand y -fill x
		bind $w <Tab> "change_selection $m"
	}
	set value [lindex $entries 0]
	$m.entries.e(0) selection range 0 [string length $value]
#	focus $m.entries.e(0)
}

proc popupmessage { msg } {
	set w [create_popup .mymess $msg]
	label $w.mylab -text $msg -relief sunken 
	pack $w.mylab -expand y -fill x -pady 5m -padx 5m
	update
}
