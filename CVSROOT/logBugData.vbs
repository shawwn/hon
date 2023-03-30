Dim BUGZ_URL: BUGZ_URL = "https://s2games.fogbugz.com/"
Dim IXREPOSITORY: IXREPOSITORY="4" ' ixRepository of this repo in FogBugz. (Not required.)

Sub LogIt( s )

	' To debug, change False to True on the line below
	If False Then
		Dim fso: Set fso = CreateObject("Scripting.FileSystemObject")
		Dim f: Set f = fso.OpenTextFile("C:\cvslog.txt", 8, True)
		f.WriteLine s
		f.Close
		Set f = Nothing
		Set fso = Nothing
	End If

End Sub

' You do NOT need to change these

Dim args: Set args = WScript.Arguments.UnNamed
Dim re: Set re = New RegExp
Dim matches, match

Dim sLogInfo
sLogInfo = WScript.StdIn.ReadAll

re.Global = True
re.Pattern = "/$"
BUGZ_URL = re.Replace( BUGZ_URL, "")  ' Strip trailing slash if present

'
' Look for bug number in the log info
'
re.IgnoreCase = True
re.Global = True
re.Pattern = "\s*BUG[ZS]*\s*IDs*\s*[#:; ]+((\d+[ ,:;#]*)+)"

Dim ixBug: ixBug = 0
Dim i
Dim bugIDString: bugIDString = ""
Set matches = re.Execute(sLogInfo)
If matches.Count > 0 Then
   For i = 0 To (matches.Count - 1)
      Set match = matches(i)
      If Len(bugIDString) > 0 Then bugIDString = bugIDString & ","
      bugIDString = bugIDString & match.SubMatches(0)
   Next
End If

'
' Remove initial and trailing '
'
Dim sArgs

For i = 0 To args.Count - 1
	sArgs = sArgs & args(i) & " "
Next
sArgs = Left( sArgs, Len(sArgs) - 1 )
sArgs = Replace(sArgs, "\ ", ":")

If Left(sArgs, 1) = "\" Then sArgs = Right(sArgs, Len(sArgs) - 1)
If Left(sArgs, 1) = "'" Then sArgs = Right(sArgs, Len(sArgs) - 1)
If Right(sArgs, 1) = "'" Then sArgs = Left(sArgs, Len(sArgs) - 1)

LogIt "sArgs: " & sArgs

Dim rgFiles
rgFiles = Split(sArgs, " ")
argCount = UBound(rgFiles) + 1

For i = LBound(rgFiles) To Ubound(rgFiles)
	rgFiles(i) = Replace(rgFiles(i), ":", " ")
	LogIt "Files: " & rgFiles(i)
Next

sDir = "/" & rgFiles(0) & "/"

If Len(bugIDString) = 0 Then
   WScript.Quit 1
End If

Wscript.echo "Submitting FogBugz info for Bugs: " & bugIDString

Dim arg, data, sFile, sPrev, sNew, http
On Error Resume Next
 Set http = CreateObject("Microsoft.XMLHTTP")
If Err.Number <> 0 Then
 Set http = CreateObject("MSXML2.ServerXMLHTTP")
End If
On Error Goto 0

If http Is Nothing Then
	LogIt "FAILURE! Couldn't create XMLHTTP object"
End If


Dim bugIDlist
bugIDString = Replace(bugIDString, " ", ",")
bugIDString = Replace(bugIDString, ":", ",")
bugIDString = Replace(bugIDString, ";", ",")
bugIDString = Replace(bugIDString, "#", ",")
bugIDlist = split(bugIDString, ",")

For Each ixBug in bugIDlist
   If IsNumeric(ixBug) Then
	ixBug = CLng(ixBug)
	Wscript.echo "Adding bug info for Bug ID #" & ixBug & "..."

	For i = 1 To argCount - 1
		data = split(rgFiles(i), ",")
		sFile = sDir & data(0)
		sPrev = data(1)
		sNew = data(2)
			If ixBug > 0 Then
				http.Open "GET", BUGZ_URL & _
					"/cvsSubmit.asp?ixBug=" & ixBug & "&sFile=" & URLEncode(sFile) & "&sPrev=" & sPrev & "&sNew=" & sNew & "ixRepository=" & ixRepository, False
				http.Send
				If http.responseText = "OK" Then
					LogIt "SUCCESS! Bug change entered!"
				Else
					LogIt "FAILURE! Could not submit to server!"
					LogIt "Status Code: " & http.status
					LogIt http.responseText
				End If
			End If
	Next
   End If
Next

' Copyright (c) 2002 Markus Diersbock, SwingNote LLC
Function URLEncode(sRawURL)
    
    Dim iLoop
    Dim sRtn
    Dim sTmp
    Dim sValidChars
    
    sValidChars = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:/.?=_-$(){}~&"

    If Len(sRawURL) > 0 Then
        ' Loop through each char


        For iLoop = 1 To Len(sRawURL)
            sTmp = Mid(sRawURL, iLoop, 1)


            If InStr(1, sValidChars, sTmp) = 0 Then
                ' If not ValidChar, convert to HEX and p
                '     refix with %
                sTmp = "%" & Hex(Asc(sTmp))
                ' If sTmp is %20 then replace with +
                sTmp = Replace(sTmp, "%20", "+")
            End If
            sRtn = sRtn & sTmp
        Next
        URLEncode = sRtn
    End If


End Function
