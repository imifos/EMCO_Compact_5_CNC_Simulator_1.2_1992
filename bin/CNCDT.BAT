@echo off
cncsimvd
if errorlevel 1 goto cga
goto ende
:cga
cncsimcd
:ende