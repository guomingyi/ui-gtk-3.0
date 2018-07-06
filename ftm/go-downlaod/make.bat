
@set THISDATE=%DATE:~0,4%%DATE:~5,2%%DATE:~8,2%

@set GOARCH=amd64
go build -o out\fw_download_%GOARCH%_%THISDATE%.exe
