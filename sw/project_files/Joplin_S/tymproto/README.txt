
How to go?

1. Write your Proto files
Proto files - source files used by google protocol buffers compiler
You can refer to '/example/simple.proto' to write your protocol definition.
Lets have all .proto files located in `/tym_proto/` and compiled files will be put
 into directory `/tym_proto_compiled/`.  

2. Set Paths accordingly
Go into '/pb_compiler/' and edit tym_compile_gdb.bat with setting below paths:
	a. set git bin path to your git installed directory;
	b. set the protocol definition directory (not need to change in normolly);
	c. set output folder name (not need to change in normolly)
	d. set output directory (not need to change in normolly)
Then save and close tym_compile_gdb.bat.

3. Compile .proto files by double click tym_compile_gdb.bat.

4. Output
After compiled, you can get the compiled in directory `/tym_proto_compiled/`.

### Example
Lets have all .proto files located in `/tym_proto/` and we want to put
compiled files into directory `/tym_proto_compiled/`.  
Lets say we are using nanopb version 0.3.8 that is downloaded in `/nanopb-0.3.8`.



