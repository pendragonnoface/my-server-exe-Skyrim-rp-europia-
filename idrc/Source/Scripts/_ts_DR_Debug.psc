Scriptname _ts_DR_Debug hidden

import debug

bool property bIsDebug = true auto hidden

Function _ts_Debug_Trace_Message(string sMessage1, string sMessage2="", bool bNotification=false) global

;/	if bIsDebug
		trace(sMessage1 + sMessage2)
		
		if bNotification
			if sMessage2 == ""
				Notification(sMessage1)
			else
				Notification(sMessage2)
			endif
		endif
	endif
/;	
endfunction

Function _ts_Debug_Message(string sMessage) global

;/	if bIsDebug 
		Notification(sMessage)
	endif
/;
endfunction
