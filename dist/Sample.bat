@echo off
console.exe -f "out.txt" -n ServerHistory.gbsdat -c "select  count(id) FROM EmployeeShifts where datetime(shifttime, 'unixepoch', 'localtime') between '2017-07-25 00:00:00' AND '2017-07-26 00:00:00'" 

if not exist out.txt goto end
type out.txt > 

:end
@pause