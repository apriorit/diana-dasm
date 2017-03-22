How to use orthia plugin in windbg session.

Step 1. Load the dll
Load the dll using the load command:
    .load <full-path>\orthia.dll

Note: 32-bit windbg on my win8-64 computer didn't want to load orthia.dll until I put it to the 
    C:\Program Files (x86)\Debugging Tools for Windows\winext
so the best way is to put the dll there

Note2: debugger could be also located in windows kit folder:
    C:\Program Files (x86)\Windows Kits\8.0\Debuggers\


Step2. Setup the profile specifying the database file
    !orthia.profile <full-path>\<profilename>.db
or
    !orthia.profile /f <full-path>\<profilename>.db
/f option allows the command to rewrite the existing file.
Note: profile command supports environment variables like %temp%, for example you can setup the profile like that:
    !orthia.profile /f %temp%\test.db
The initial setup is done.

Now you can:
1) reload the module by address:
    !orthia.reload <module_start_address>
Orthia analyses all the references inside the module and puts the into the database. 
Note: module reloading could take some time, for example when working with live debug session. Orthia caches the module sections but initial data acquiring could be slow.
2) reload all modules
    !for_each_module !orthia.reload /v @#Base
3) show the references to the instruction
    !orthia.x <address_expression>
4) show the references to the instruction range
    !orthia.xr <address_expression1> <address_expression2>
5) show all loaded modules
    !orthia.lm
6) analyze custom region
    !orthia.a <address> 
 
If you have any questions or proposals feel free to ask me with ligen.work@gmail.com.
Enjoy!

P.S use \src\start.bat and \src\start_with_kit.cmd to build the solution
