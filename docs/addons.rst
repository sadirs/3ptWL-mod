
AddOn's
=======


This section describes how to extend the functionality of **wlcf** by adding new features, models, or utilities to the codebase.

The modular structure of the code allows users to include additional functionality through custom header and source files.

**Include files**

Header files define the interface of new functionalities and must be placed in the appropriate include directories.

To add new features:

1. Create a header file (e.g., `my_module.h`)

2. Place it in one of the include directories:

   ::

    include/
    addons/addons_include/

3. Declare functions, structures, or parameters inside the header.

Example

c::

    #ifndef MY_MODULE_H
    #define MY_MODULE_H
    void my_new_function(double *input, double *output);
    #endif


4. Include the header in the relevant source files:

c::

    #include "my_module.h"


**Source files**

Source files implement the functionality declared in the header files.

To add a new module:

1. Create a source file (e.g., `my_module.c`)

2. Place it in one of the source directories:

   ::

    source/
    addons/

3. Implement the functions:

c::

    #include "my_module.h"
    void my_new_function(double *input, double *output) {
    // Example computation
    *output = input[0] * 2.0;}


**Compilation**

To ensure the new files are compiled:

* Add the source file to the build system (if not automatically included)
* Modify `Makefile` or `Makefile_machine` if necessary

Example (if manual inclusion is needed):

make::

    SRC += source/my_module.c

**Integration**

To integrate the new functionality into **wlcf**:

* Call your new functions from existing modules (e.g., in `procedures.c`)
* Ensure parameters are passed correctly
* Update relevant headers if new parameters are introduced
