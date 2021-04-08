Template variables:
    ${nameAndExt} - file's name with extension
    ${name} - given class name (e.g. i2c, not i2c_srv)
    ${brief} - brief description of file (not in use at the moment)
    ${date} - date of file creation, YYYY-MM-DD format
    ${user} - username, FirstName LastName format (from `git config user.name`)

Other than those variables, the templates are just regular C and H files.

Edit ../newclass.py to add new variables, template types, etc..
Edit the devTools gerrit repo to reflect these changes in the NetBeans/MPLabX wizard. 
    
