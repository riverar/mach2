New-Item -ItemType Directory "basic"
Push-Location "basic"
New-Item -ItemType Directory "dxgkrnl.pdb/991B400BD0C7BAF394B0A1BF66064F251"
Invoke-WebRequest -UseBasicParsing "https://msdl.microsoft.com/download/symbols/dxgkrnl.pdb/991B400BD0C7BAF394B0A1BF66064F251/dxgkrnl.pdb" -OutFile dxgkrnl.pdb/991B400BD0C7BAF394B0A1BF66064F251/dxgkrnl.pdb
New-Item -ItemType Directory "explorer.pdb/83685E18DD9891A07590CFE1D55ACA801"
Invoke-WebRequest -UseBasicParsing "https://msdl.microsoft.com/download/symbols/explorer.pdb/83685E18DD9891A07590CFE1D55ACA801/explorer.pdb" -OutFile explorer.pdb/83685E18DD9891A07590CFE1D55ACA801/explorer.pdb
Invoke-WebRequest -UseBasicParsing "https://msdl.microsoft.com/download/symbols/explorer.exe/8886051B4d9000/explorer.exe" -OutFile explorer.exe
Invoke-WebRequest -UseBasicParsing "https://msdl.microsoft.com/download/symbols/dxgkrnl.sys/807E58BC48e000/dxgkrnl.sys" -OutFile dxgkrnl.sys
Invoke-WebRequest -UseBasicParsing "https://msdl.microsoft.com/download/symbols/authbroker.dll/26CA9F7A39000/authbroker.dll" -OutFile AuthBroker.dll
Invoke-WebRequest -UseBasicParsing "https://msdl.microsoft.com/download/symbols/System.Runtime.CompilerServices.Unsafe.pdb/0E8DB93D1CE04371BA54A45B8AE6A714ffffffff/System.Runtime.CompilerServices.Unsafe.pdb" -OutFile System.Runtime.CompilerServices.Unsafe.pdb/0E8DB93D1CE04371BA54A45B8AE6A714ffffffff/System.Runtime.CompilerServices.Unsafe.pdb
Pop-Location

New-Item -ItemType Directory "reporting"
Push-Location "reporting"
New-Item -ItemType Directory "drvinst.pdb/19E1E8FEBD4F93E42E8B85C1FEFFD5D61"
Invoke-WebRequest -UseBasicParsing "https://msdl.microsoft.com/download/symbols/drvinst.pdb/19E1E8FEBD4F93E42E8B85C1FEFFD5D61/drvinst.pdb" -OutFile drvinst.pdb/19E1E8FEBD4F93E42E8B85C1FEFFD5D61/drvinst.pdb
Invoke-WebRequest -UseBasicParsing "https://msdl.microsoft.com/download/symbols/drvinst.exe/33DDABC45d000/drvinst.exe" -OutFile drvinst.exe
Pop-Location

New-Item -ItemType Directory "staging"
Push-Location "staging"
New-Item -ItemType Directory "ntkrnlmp.pdb/19E1E8FEBD4F93E42E8B85C1FEFFD5D61"
Invoke-WebRequest -UseBasicParsing "https://msdl.microsoft.com/download/symbols/ntkrnlmp.pdb/19E1E8FEBD4F93E42E8B85C1FEFFD5D61/ntkrnlmp.pdb" -OutFile ntkrnlmp.pdb/19E1E8FEBD4F93E42E8B85C1FEFFD5D61/ntkrnlmp.pdb
New-Item -ItemType Directory "windows.ui.xaml.pdb/4E1E303158F4ADC6CD8D258B3AE2FE421"
Invoke-WebRequest -UseBasicParsing "https://msdl.microsoft.com/download/symbols/windows.ui.xaml.pdb/4E1E303158F4ADC6CD8D258B3AE2FE421/windows.ui.xaml.pdb" -OutFile windows.ui.xaml.pdb/4E1E303158F4ADC6CD8D258B3AE2FE421/windows.ui.xaml.pdb
New-Item -ItemType Directory "AuthBroker.pdb/79DD7DD69F510933FAD793E601E7B39D1"
Invoke-WebRequest -UseBasicParsing "https://msdl.microsoft.com/download/symbols/AuthBroker.pdb/79DD7DD69F510933FAD793E601E7B39D1/AuthBroker.pdb" -OutFile AuthBroker.pdb/79DD7DD69F510933FAD793E601E7B39D1/AuthBroker.pdb
Pop-Location