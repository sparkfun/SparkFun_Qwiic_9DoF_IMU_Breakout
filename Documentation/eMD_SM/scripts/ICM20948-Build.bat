"C:\Program Files (x86)\Atmel\Studio\7.0\AtmelStudio.exe" EMD-Core\EMD-Core-ICM20948.cproj /rebuild release /out scripts\ICM20948-EMDCore.txt
"C:\Program Files (x86)\Atmel\Studio\7.0\AtmelStudio.exe" EMD-App\EMD-App-ICM20948.cproj /rebuild release /out scripts\ICM20948-EMDApp.txt
mkdir release\ICM20948
copy EMD-App\Release\EMD-App-ICM20948.bin release\ICM20948
copy EMD-App\Release\EMD-App-ICM20948.elf release\ICM20948