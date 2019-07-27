@echo off
echo ******************************************************
echo *      test API with Command Line Interface(CLI)	 *
echo *							 *
echo ******************************************************



echo ****************start from here***********************


echo /*view list table on database*/
console.exe -f "" -n "Main.gbsdat" -c "SELECT name FROM sqlite_master  WHERE type IN ('table')"
echo *******************************************************

echo /*Dump table employeeshifts*/
console.exe -f "Employeeshift.csv" -n ServerHistory.gbsdat -c "select  * FROM EmployeeShifts"

echo ******************************************************* 
echo /*Dump table employee*/
console.exe -f "Employees.csv" -n Main.gbsdat -c "select  * FROM Employees"

echo ******************************************************* 
echo /*Dump table employee details*/
console.exe -f "EmployeeDetails.csv" -n Main.gbsdat -c "select  * FROM EmployeeDetails"

echo *******************************************************

echo /*Dump table employeeshifts with format date*/
console.exe -f "employeeshift_formated.csv"  -n "ServerHistory.gbsdat" -c "SELECT id,emplid,datetime(shifttime,'unixepoch','localtime'),oldendbalance,conflictedamount,takenamount,newstartbalance,physicaldeleted,needuploading,enabled FROM employeeshifts"

echo ******************************************************* 


echo /*Dump table serverlogs */
console.exe -n ServerHistory.gbsdat -f "serverlogs.csv" -c "select * FROM ServerLogs" 

echo ******************************************************* 
echo /*Dump table sessionlogs */
console.exe -f "Sessionlogs.csv" -n "SessionHistory.gbsdat" -c "SELECT * FROM SessionLogs"

echo *******************************************************
echo /*Dump table sessionlogs*/
console.exe -n SystemHistory.gbsdat -f "Systemlogs.csv" -c "select * FROM SystemLogs" 

echo *******************************************************
echo /*Dump table TopupQueueLogs*/
console.exe -n TopupQueueHistory.gbsdat -c "select * FROM TopupQueueLogs" -f "TopupQueueLogs.csv"


IF console.exe -f "Employeeshift.csv" -n ServerHistory.gbsdat -c "select  count(id) FROM EmployeeShifts where datetime(shifttime, 'unixepoch', 'localtime') between '07/22/2017' AND '07/23/2017'"
{
	FIND 'Row.Count'

}



echo ******************** end ******************************

@pause