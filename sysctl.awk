BEGIN {
	FS="\t"
	print "#include <linux/sysctl.h>" > defs
}

/^\/\//	{
	next
}

$1 {
	if(isopen){
		print "\t{},"
		print "};"
		print ""
	}
	tab=$1
	if ($2 != "global") {
		printf "static "
	} else {
		if(!declared){
			print "struct ctl_table;" > defs
			declared = 1
		}
		print "extern struct ctl_table " $1 "[];" > defs
	}
	print "ctl_table " $1 "[] = {"
	isopen=1
	next	
}

function item(key, val, needEndif){
	if(val){
		if(key=="strategy" || key=="ctl_name"){
			print "#if LINUX_VERSION_CODE < 0x20633"
			needEndif=1
		}
#		print "\t." key "=\t" val ","
		print "         ." key " = " val ","
		if(needEndif)
			print "#endif" 
	}
}


$2 {
	type=$2
	mode=$3
	name=$4
	data=$5
	if(data=="&")
		data="&"name
	else if(!data)
		data=name

	strategy=""
	handler=""
	ctl_name="CTL_UNNUMBERED"
	sz = 0
	if(type ~ "(.*)string"){
		sz=type
		sub("\\(","",sz)
		sub("\\).*", "", sz)
		sub("#chan", "MAX_CHANNELS", sz)
		type=string
		strategy="&sysctl_string"
		handler="&proc_dostring"
		print "extern char " data "[];" > defs
	} else if(type ~ "\\[.*\\]uint") {
		sz=type
		sub("\\[", "", sz)
		sub("\\].*", "", sz)
		sub("#chan", "MAX_CHANNELS", sz)
		type=int
		handler="&proc_dointvec"
		xname = data
		if (xname ~ "^&" && sz==1) {
			sub("^&", "", xname)
			print "extern unsigned int " xname ";" > defs
		} else {
			print "extern unsigned int " data "[];" > defs
		}
		sz=sz " * sizeof(unsigned int)"
	} else if(type ~ "\\[.*\\]int") {
		sz=type
		sub("\\[", "", sz)
		sub("\\].*", "", sz)
		sub("#chan", "MAX_CHANNELS", sz)
		type=int
		handler="&proc_dointvec"
		xname = data
		if (xname ~ "^&" && sz==1) {
			sub("^&", "", xname)
			print "extern int " xname ";" > defs
		} else {
			print "extern int " data "[];" > defs
		}
		sz=sz " * sizeof(int)"
	} else if (type=="hgrev") {
		"hg -R " root " id -i" | getline id
		"hg -R " root " log -l1 --template '\"{date|shortdate} " id "\"\n'" | getline vcsrev
		data=vcsrev
		sz=length(vcsrev)-1
		name="vcs-rev"
		strategy="&sysctl_string"
		handler="&proc_dostring"
	} else if (type=="child") {
		child=data
		data=""
	}
	print "        {"
	item("ctl_name", ctl_name)
	item("procname", "\"" name "\"")
	item("data", data)
	item("maxlen", sz)
	item("mode", mode)
	item("child", child)
	item("proc_handler", handler)
	item("strategy", strategy)
	print "\t},"

	def=$6
	if (def ~ "^#") {
		sub("^#", "", def)
		if(!def){
			def = toupper(name)
		}
		print "#define SYSCTL_" def "\t" n > defs
	}
	n++
}


END {
	print "\t{},"
	print "};"
}
