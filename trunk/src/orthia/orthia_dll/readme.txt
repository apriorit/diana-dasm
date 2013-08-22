How to use orthia plugin in windbg session.

Step 1. Load the dll
Load the dll using the load command:
    .load <full-path>\orthia.dll
Note: 32-bit windbg on my win8-64 computer didn't want to load orthia.dll until I put it to the 
    C:\Program Files (x86)\Debugging Tools for Windows\winext

Step2. Setup the profile specifying the database file
    !orthia.profile <full-path>\<profilename>.db
or
    !orthia.profile /f <full-path>\<profilename>.db
/f option allows the command to rewrite the existing file.

The initial setup is done.

Now you can:
1) reload the module by address:
    !orthia.reload <module_start_address>
Orthia analyses all the references inside the module and puts the into the database. 
Note: in some cases module reloading can take some time, for example when working with live debug session. Orthia caches the module sections but initial data acquiring could be slow.

2) reload all modules
    !for_each_module !orthia.reload /v @#Base
3) show the references to the instruction
	!orthia.x <address_expression>
4) show the references to the instruction range
	!orthia.xr <address_expression1> <address_expression2>
5) show all loaded modules
    !orthia.lm
 
If you have any questions or proposals feel free to ask me with ligen.work@gmail.com.
Enjoy!
