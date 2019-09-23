@echo off

if %~1==1 (
	getmac > mac.txt
)
if %~1==2 (
	ipconfig > ipconfig.txt
)
if %~1==3 (
	del ipconfig.txt
)
if %~1==4 (
	echo %USERPROFILE% > homedir.txt
)
if %~1==5 (
	del homedir.txt
)
if %~1==6 (
	del %USERPROFILE%\FileSharing\Immagini_utenti\%~2%.jpg
)
if %~1==7 (
	rmdir /Q /S %~2%
)
if %~1==8 (
	del %~2%
)
if %~1==9 (
	mkdir %USERPROFILE%\FileSharing
)
if %~1==10 (
	mkdir %USERPROFILE%\FileSharing\Immagini_utenti
)