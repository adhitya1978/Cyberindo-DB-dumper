@echo off
echo ***********************************************************
echo * Demonstration using console.exe			*
echo *							*
echo ***********************************************************

echo /* Format 1*/
Summary.exe -s "2017/06/22" -e "2017/06/23"
echo **************************************

echo /* Format 2 not support*/
Summary.exe -s "22/06/2017" -e "23/06/2017"
echo **************************************

echo /* Format 3*/
Summary.exe -s "2017-06-22" -e "2017-06-23"
echo **************************************

echo /* Format 4 not support*/
Summary.exe -s "22-06-2017" -e "23-06-2017"
echo **************************************



echo /* durasi dengan 1 hari*/
Summary.exe -s "2017/06/22" -d "1"
echo **************************************

echo /* durasi dengan 5 hari*/
Summary.exe -s "2017/06/23" -d "5"
echo **************************************


echo /* print summary*/
Summary.exe -p "output.txt"

@pause