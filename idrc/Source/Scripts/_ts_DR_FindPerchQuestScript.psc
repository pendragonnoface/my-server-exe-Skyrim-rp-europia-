;BEGIN FRAGMENT CODE - Do not edit anything between this and the end comment
;NEXT FRAGMENT INDEX 1
Scriptname _ts_DR_FindPerchQuestScript Extends Quest Hidden

;BEGIN ALIAS PROPERTY DRAGON
;ALIAS PROPERTY TYPE ReferenceAlias
ReferenceAlias Property Alias_DRAGON Auto
;END ALIAS PROPERTY

;BEGIN ALIAS PROPERTY WordWallPerch
;ALIAS PROPERTY TYPE ReferenceAlias
ReferenceAlias Property Alias_WordWallPerch Auto
;END ALIAS PROPERTY

;BEGIN ALIAS PROPERTY RockPerch
;ALIAS PROPERTY TYPE ReferenceAlias
ReferenceAlias Property Alias_RockPerch Auto
;END ALIAS PROPERTY

;BEGIN ALIAS PROPERTY TowerPerch
;ALIAS PROPERTY TYPE ReferenceAlias
ReferenceAlias Property Alias_TowerPerch Auto
;END ALIAS PROPERTY

;BEGIN FRAGMENT Fragment_0
Function Fragment_0()
;BEGIN CODE
CheckPerch()
;END CODE
EndFunction
;END FRAGMENT

;END FRAGMENT CODE - Do not edit anything between this and the begin comment
function CheckPerch()
	if !Alias_TowerPerch.GetReference() && !Alias_RockPerch.GetReference() && !Alias_WordWallPerch.GetReference()
		debug.Notification("No perch spot close-by...")
	endIf
endfunction
